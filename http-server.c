#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

//max number of bytes for words
#define MAX_BYTES 100


int parse_get_request(int sock_fd, char** body, int num_lines){
  //TODO: deal with resource path
  int rc;
  char* words[MAX_BYTES];
  
  char* tok = strtok(body[0], " ");
  int i = 0;

  words[i] = tok;
  while(tok != NULL) {
    i++;
    tok = strtok(NULL, " ");
    words[i] = tok;
  }

  char* path = malloc(MAX_BYTES);

  if(strncmp(words[1], "/", MAX_BYTES) == 0) {
    words[1] = "/Users/ruba/code/chttpd/index.html\0";
  } else {
    char abs_path[25] = "/Users/ruba/code/chttpd\0";
    strncpy(path, abs_path, strlen(abs_path) + 1);
  }

  strcat(path,  words[1]);

  if(access(path, F_OK) != -1) {
    char* response = malloc(MAX_BYTES);
    FILE* file;
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 200 OK\n\0");
    file = fopen(path, "r");
    char** text = malloc(MAX_BYTES);
    if(file) {
      char* line = NULL;
      size_t linecap = 0;
      ssize_t linelen = 0;
      while((linelen = getline(&line, &linecap, file)) > 0) {
        *text = calloc(MAX_BYTES, MAX_BYTES);
        strncpy(*text, line, MAX_BYTES);
        strcat(response, *text);
      }
    } else {
      perror("error opening file!");
    }

    write(sock_fd, response, strlen(response));
    rc = 1;
  } else {
    printf("%s 404 File Not Found\n", words[2]);
    rc = 0;
  }
  return rc;
}

int parse_head_request(int sock_fd, char** body, int num_lines) {
  //TODO: figure out what to write back
  char* words[MAX_BYTES];
  char* tok = strtok(body[0], " ");
  int i = 0;

  words[i] = tok;
  while(tok != NULL) {
    i++;
    tok = strtok(NULL, " ");
    words[i] = tok;
  }

  char* path = malloc(MAX_BYTES);

  if(strncmp(words[1], "/", MAX_BYTES) == 0) {
    words[1] = "/Users/ruba/code/chttpd/index.html\0";
  } else {
    char abs_path[25] = "/Users/ruba/code/chttpd\0";
    strncpy(path, abs_path, strlen(abs_path) + 1);
  }

  strcat(path,  words[1]);

  if(access(path, F_OK) != -1) {
    char* response = malloc(MAX_BYTES);
    FILE* file;
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 200 OK\n\0");
    write(sock_fd, response, strlen(response));
   
  } else {
    printf("%s 404 File Not Found\n", words[2]);
  }
  
  return 0;
}

int parse_request(char* buff, int sock_fd) {
  //TODO: fix the order of the read_from_client, parse_request.. functions
  char* buff_copy = calloc(1, MAX_BYTES);
  strncpy(buff_copy, buff, MAX_BYTES);
  int num_lines = 0;
  char* newline = strtok(buff_copy, "\n");
  while(newline != NULL) {
   num_lines +=1;
   newline = strtok(NULL, "\n");
  }
  
  char** lines = calloc(num_lines, sizeof(char *));
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
  char** str = calloc(num_lines, sizeof(char *));
  for (int x = 0; x < i; x++) {
    str[x] = calloc(MAX_BYTES, sizeof(char));
    strncpy(str[x], lines[x], MAX_BYTES * sizeof(char));
  }

  //determine type of request from the first word of the message
  //should probably do it in a more organized fashion
  char* tok = strtok(str[0], " ");
  if(strcmp(tok, "GET") == 0) {
    if(parse_get_request(sock_fd, lines, num_lines)) {
      return 1;
    } else {
      return 0;
    }
  } else if (strcmp(tok, "HEAD") == 0) {
    parse_head_request(sock_fd, lines, num_lines);
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

  parse_request(buff, socket_fd);
  return num_bytes;
}

void close_socket(int sock_fd) {
  if (close(sock_fd) < 0) {
    perror("error closing socket");
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  //char blorp[1000] = "HEAD /path/to/file HTTP/1.0\nfrom: bla\nto: blorp\n";
  //parse_request(blorp);
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

