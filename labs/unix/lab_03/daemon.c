#include <syslog.h>
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

void daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;

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
	
	if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
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

int main(int argc, char* argv[])
{
	daemonize(argv[0]);

	if (is_already_running())
	{
		syslog(LOG_ERR, "%s: daemon already running.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Daemon code
	while(1)
	{
		sleep(1);
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		syslog(LOG_INFO, "%s: Current time is: %02d:%02d:%02d\n", 
			argv[0], tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
}
