#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <syslog.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

sigset_t mask;

int thread_active = 1;

void sig_handler(int signo);

void daemonize(const char *cmd);

int lockfile(int fd);

int is_already_running();

void *thr_fn(void *arg);

int main(int argc, char* argv[])
{
	pthread_t tid;
	int signo;
	struct sigaction action;

	printf("Login before daemonize: %s\n", getlogin());

	daemonize(argv[0]);

	syslog(LOG_INFO, "Login after daemonize: %s\n", getlogin());

	// Daemon code

	if (is_already_running())
	{
		syslog(LOG_ERR, "%s: daemon already running.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGHUP, &action, NULL) == -1)
	{
		syslog(LOG_ERR, "%s: can't restore SIGHUP default.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sigfillset(&mask);
	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1)
	{
		syslog(LOG_ERR, "%s: can't block signals.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&tid, NULL, thr_fn, NULL) == -1)
	{
		syslog(LOG_ERR, "%s: pthread_create error: %s\n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		if (sigwait(&mask, &signo) == -1)
		{
			syslog(LOG_ERR, "%s: sigwait error: %s\n", argv[0], strerror(errno));
			exit(EXIT_FAILURE);
		}

		switch(signo)
		{
			case SIGTERM:
				syslog(LOG_INFO, "%s: SIGTERM received.\n", argv[0]);
				thread_active = 0;
				pthread_join(tid, NULL);
				exit(EXIT_SUCCESS);

			case SIGHUP:
				syslog(LOG_INFO, "%s: SIGHUP received.\n", argv[0]);
				break;
			default:
				syslog(LOG_INFO, "%s: unexpected signal %d received.\n", argv[0], signo);
				break;
		}
	}
}

void daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction action;

	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
	{
		fprintf(stderr, "%s: Error reading max descriptor number.\n", cmd);
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "%s: can't fork.\n", cmd);
		exit(EXIT_FAILURE);
	}

	if (pid != 0)
		exit(EXIT_SUCCESS);

	setsid();
	
	action.sa_handler = SIG_IGN;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGHUP, &action, NULL) == -1)
	{
		fprintf(stderr, "%s: can't ignore SIGHUP.\n", cmd);
		exit(EXIT_FAILURE);
	}

	if (chdir("/") == -1)
	{
		fprintf(stderr, "%s: unable to make / current directory.\n", cmd);
		exit(EXIT_FAILURE);
	}

	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	fd0 = open("dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	// chroot("/home");
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) 
	{
		syslog(LOG_ERR, "file descriptors incorrect: %d %d %d\n", fd0, fd1, fd2);
		exit(EXIT_FAILURE);
	}
}

int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	return fcntl(fd, F_SETLK, &fl);
}

int is_already_running()
{
	int fd;

	fd = open(LOCKFILE, O_WRONLY|O_CREAT, LOCKMODE);
	if (fd == -1)
	{
		syslog(LOG_ERR, "can't open %s: %s\n",
			LOCKFILE, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (lockfile(fd) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return EXIT_FAILURE;
		}

		syslog(LOG_ERR, "can't lock file %s: %s\n",
			LOCKFILE, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 0;
}

void *thr_fn(void *arg)
{
	while(thread_active)
	{
		sleep(1);
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		syslog(LOG_INFO, "Daemon%d: Current time is: %02d:%02d:%02d\n", 
			getpid(), tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	return NULL;
}
