#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>

//max number of bytes for words
#define SIZE 100

int parse_get_request(int sock_fd, char** body, int num_lines){
  //split first line of request
  int rc;
  char* words[SIZE];
  char* tok = strtok(body[0], " ");
  int i = 0;

  words[i] = tok;
  while(tok != NULL) {
    i++;
    tok = strtok(NULL, " ");
    words[i] = tok;
  }

  //get the path from that line
  char* path = malloc(SIZE);

  if(strncmp(words[1], "/", SIZE) == 0) {
    words[1] = "/Users/ruba/code/chttpd/index.html\0";
  } else {
    char abs_path[25] = "/Users/ruba/code/chttpd\0";
    strncpy(path, abs_path, strlen(abs_path) + 1);
  }

  strcat(path,  words[1]);

  //check if resource exists
  char* response = malloc(SIZE);
  if(access(path, F_OK) != -1) {
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 200 OK\n\0");
    //open file to extract lines 
    FILE* file;
    file = fopen(path, "r");
    char** text = malloc(SIZE);
    if(file) {
      char* line = NULL;
      size_t linecap = 0;
      ssize_t linelen = 0;
      while((linelen = getline(&line, &linecap, file)) > 0) {
        *text = calloc(SIZE, SIZE);
        strncpy(*text, line, SIZE);
        strcat(response, *text);
      }
    } else {
      perror("error opening file!");
    }

    rc = 1;
  } else {
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 404 Not Found\n\0");
    printf("%s 404 File Not Found\n", words[2]);
    rc = 0;
  }

  write(sock_fd, response, strlen(response));
  return rc;
}

int parse_head_request(int sock_fd, char** body, int num_lines) {
  //split the words of the first line of the request
  char* words[SIZE];
  char* tok = strtok(body[0], " ");
  int i = 0;
  int rc = 0;

  words[i] = tok;
  while(tok != NULL) {
    i++;
    tok = strtok(NULL, " ");
    words[i] = tok;
  }

  //store the path in relation to where chttpd lives
  char* path = malloc(SIZE);
  if(strncmp(words[1], "/", SIZE) == 0) {
    words[1] = "/Users/ruba/code/chttpd/index.html\0";
  } else {
    char abs_path[25] = "/Users/ruba/code/chttpd\0";
    strncpy(path, abs_path, strlen(abs_path) + 1);
  }
  strcat(path,  words[1]);
  //write: DATE: content-type:text/html content-length 
  char* response = malloc(SIZE);

  if(access(path, F_OK) != -1) {
    //get current time and date
    time_t current_time = time(NULL);
    if(current_time == (time_t) - 1) {
      fprintf(stderr, "error computing time\n");
      return 0;
    }
    
    struct tm* local_time = localtime(&current_time);
    if(local_time == NULL) {
      fprintf(stderr, "error converting time\n");
      return 0;
    }

    char buff[SIZE];
    strftime(buff, SIZE, "Date: %A, %d %B %Y %T %Z\n", local_time);
   
    //get file information 
    struct stat attribs;
    int stat_rc = stat(path, &attribs);
    if(stat_rc > 0) {
      fprintf(stderr, "error getting file attribs\n");
      return 0;
    }

    char date[SIZE];
    struct tm* access_time = localtime(&(attribs.st_atime));
    strftime(date, SIZE, "Last accessed:%A, %d %B %Y %T %Z\n", access_time);
    
    //print status line as 200 OK
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 200 OK\nDate: \0");
    strcat(response, buff);
    strcat(response, date);

    rc = 1;
  } else {
    strncpy(response, words[2], strlen(words[2] + 1));
    strcat(response, " 404 Not Found\n\0");
    printf("%s 404 Not Found\n", words[2]);
    rc = 0;
  }
  
  write(sock_fd, response, strlen(response));
  return rc;
}

int parse_request(char* buff, int sock_fd) {
  //TODO: fix the order of the read_from_client, parse_request.. functions
  char* buff_copy = calloc(1, SIZE);
  strncpy(buff_copy, buff, SIZE);
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
    str[x] = calloc(SIZE, sizeof(char));
    strncpy(str[x], lines[x], SIZE * sizeof(char));
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

