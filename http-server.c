#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>


int parse_get_request(char** body){
  return 0;
}

int parse_head_request(char** body, int num_lines) {
  for(int i = 0; i < num_lines; i++) {
    printf("lines[%d]: %s\n", i, body[i]);
  }
  return 0;
}

int parse_request(char* buff) {
  //split into lines
  //split lines into words
  char** lines = calloc(50, sizeof(char *));
  int i = 0;
  char* substr = strtok(buff, "\n");
  lines[i] = substr;
  while(substr != NULL){
    i++;
    substr = strtok(NULL, "\n");
    lines[i] = substr;
  }

  //since strtok modifies its argument, I'm creating a copy of lines to figure
  //out the type(GET, POST, HEAD) of request
  char** str = calloc(50, sizeof(char *));
  for (int x = 0; x < i; x++) {
    str[x] = calloc(50, sizeof(char));
    //strncpy(str[x], lines[x], sizeof(str));
    strncpy(str[x], lines[x], 50 * sizeof(char));
  }

  //determine type of request from the first word of the message
  //should probably do it in a more organized fashion
  char* tok = strtok(str[0], " ");
  if(strcmp(tok, "GET") == 0) {
    parse_get_request(lines);
  } else if (strcmp(tok, "HEAD") == 0) {
    parse_head_request(lines, i);
  } else {
    perror("invalid request lol");
    return 0;
  }
  printf("finished parsing\n");
  return 1;
}

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


int accept_connection(int sock_fd) {
  struct sockaddr_in clien_addr;
  //haven't made up my mind over using memset here
  //memset(&clien_addr, 0, sizeof(struct sockaddr_in));
  int clien_len;
  int accepted_socket_fd = accept(sock_fd, (struct sockaddr*)&clien_addr, (socklen_t*)&clien_len);
  if (accepted_socket_fd < 0) {
    perror("error accepting connection");
    exit(1);
  }
  return accepted_socket_fd;
}

int read_from_client(int socket_fd) {
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
  return num_bytes;
}

void close_socket(int sock_fd) {
  if (close(sock_fd) < 0) {
    perror("error closing socket");
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  char blorp[1000] = "HEAD /path/to/file HTTP/1.0\nfrom: bla\nto: blorp\n";
  parse_request(blorp);
  int port = atoi(argv[1]);
  int sock_fd = open_socket();
  bind_socket(sock_fd, port);
  listen_socket(sock_fd, 5);
  while(1) {
    int accepted_fd = accept_connection(sock_fd);
    int num_bytes = read_from_client(accepted_fd);
    if (num_bytes == 0) {
      close_socket(sock_fd);
    }
  }
  return 0;
}

