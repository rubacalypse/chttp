#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int open_socket() {
  //SOCK_STREAM -> TCP/IP type of communication 
  int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  
  if (sock_fd < 0) {
    perror("error opening socket");
    exit(1);
  }
  return sock_fd;
}

int bind_socket(int sock_fd, int port) {
  struct sockaddr_in sock_addr;
  //AF_INET: address family of IPv4
  sock_addr.sin_family = AF_INET;
  //htons/l convert from host byte order to network byte order
  sock_addr.sin_port = htons(port);
  sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  int bind_rc = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
  if (bind_rc < 0) {
    perror("error binding socket");
    exit(1);
  }
  return bind_rc;
}

void listen_socket(int sock_fd, int num_connections) {
  if (listen(sock_fd, num_connections) < 0) {
    perror("error listening");
    exit(1);
  }
}

int read_from_client(socket_fd) {
  char buff[512];
  int num_bytes = read(socket_fd, buff, 512);
  printf("%s\r\n", buff);
  if (num_bytes < 0) {
    perror("error reading from client");
    exit(1);
  }
  if (num_bytes == 0) {
    perror("reached end-of-file");
    exit(1);
  }

  printf("Well this is nice\r\n"); 
  return 0;
}

void close_socket(int sock_fd) {
  if (close(sock_fd) < 0) {
    perror("error closing socket");
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  int port = atoi(argv[1]);
  int sock_fd = open_socket();
  struct sockaddr_in clien_addr;
  int clien_len;
  bind_socket(sock_fd, port);
  listen_socket(sock_fd, 5);
  while(1) {
    int accept_socket_fd = accept(sock_fd, (struct sockaddr*)&clien_addr, (socklen_t*)&clien_len);
    if (accept_socket_fd < 0) {
      perror("error accepting connection");
      exit(1);
    }
    int num_bytes = read_from_client(accept_socket_fd);
    if (num_bytes == 0) {
      close_socket(sock_fd);
    }
  }
  return 0;
}

