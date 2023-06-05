#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERV_PORT 9877
#define MAX_LEN 128

int main(void) {
  int sock;
  char buf[MAX_LEN];

  struct sockaddr_in client_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(SERV_PORT),
                                    .sin_addr = htonl(INADDR_LOOPBACK)};

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  if (connect(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) ==
      -1) {
    perror("connect");
    exit(1);
  }

  sprintf(buf, "%d", getpid());

  if (send(sock, buf, sizeof(buf), 0) == -1) {
    perror("send");
    exit(1);
  }

  printf("client sent: %s\n", buf);

  if (recv(sock, buf, sizeof(buf), 0) == -1) {
    perror("recv");
    exit(1);
  }

  printf("client received: %s\n", buf);
  close(sock);

  exit(0);
}
