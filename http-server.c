#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>

int open_socket(){
  int sock_fd;

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("error opening socket");
    exit(1);
  }
  return sock_fd;
}

int bind_socket(int sock_fd, int port) {
  struct sockaddr_in* sock_addr;
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_port = htons(port);
  sock_addr->sin_addr.s_addr = htonl(INADDR_ANY);

  int bind_rc = bind(sock_fd, (struct sockaddr *)sock_addr, sizeof(sock_addr));
  if (bind_rc < 0) {
    perror("error binding socket");
    exit(1);
  }

  return bind_rc;

}

int main(int argc, char* argv[]){
  printf("hello");
  int port = atoi(argv[1]);
  int sock_fd = open_socket();
  bind_socket(sock_fd, port);
}

