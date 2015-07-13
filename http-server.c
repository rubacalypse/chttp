#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>

int open_socket(){
  int sock_fd;
  //SOCK_STREAM -> TCP/IP type of communication 
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("error opening socket");
    exit(1);
  }
  return sock_fd;
}

int bind_socket(int sock_fd, int port) {
  
  struct sockaddr_in* sock_addr;
  //AF_INET: address family of IPv4
  sock_addr->sin_family = AF_INET;
  //htons/l convert from host byte order to network byte order
  sock_addr->sin_port = htons(port);
  sock_addr->sin_addr.s_addr = htonl(INADDR_ANY);
  
  //still a bit confused when it comes to the difference between sockaddr and sockaddr_in
  int bind_rc = bind(sock_fd, (struct sockaddr *)sock_addr, sizeof(sock_addr));
  if (bind_rc < 0) {
    perror("error binding socket");
    exit(1);
  }

  return bind_rc;

}

int main(int argc, char* argv[]){
  int port = atoi(argv[1]);
  int sock_fd = open_socket();
  bind_socket(sock_fd, port);
}

