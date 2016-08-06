#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void open_socket(int *sockfd) {
  printf("opening socket\n");
  *sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");
}

void bind_socket(int portno, int sockfd, struct sockaddr_in serv_addr) {
  printf("binding socket\n");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
}

void listen_to_socket(int sockfd, int *newsockfd, socklen_t *clilen, struct sockaddr_in cli_addr) {
  printf("listening to socket\n");
  listen(sockfd, 5);
  *clilen = sizeof(cli_addr);
  *newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,  clilen);
  if (newsockfd < 0) error("ERROR on accept");
  printf("done setting up listen\n");
}

void read_from_socket(int newsockfd, char buffer[], const int BUFFER_SIZE) {
  printf("reading from socket\n");
  int n;
  bzero(buffer, BUFFER_SIZE);
  n = read(newsockfd, buffer, BUFFER_SIZE - 1);
  if (n < 0) error("ERROR reading from socket");
  printf("Here is the message: %s\n", buffer);
}

void write_to_socket(int newsockfd) {
  int n;
  n = write(newsockfd,"I got your message", 18);
  if (n < 0) error("ERROR writing to socket");
}

void close_socket(int sockfd, int newsockfd) {
  printf("closing socket\n");
  close(newsockfd);
  close(sockfd);
}

char *encrypt(char *target_file, char *key_file) {

  //return encrypted_text;
}

int main(int argc, char *argv[]) {
  const int BUFFER_SIZE = 512;
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in serv_addr, cli_addr;

  // verify correct input arguments
  if (argc < 2) {
    error("ERROR, no port provided\n");
    exit(1);
  }

  // basic setup of elements
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  open_socket(&sockfd);
  bind_socket(portno, sockfd, serv_addr);
  //listen_to_socket(sockfd, &newsockfd, &clilen, cli_addr);
  printf("listening to socket\n");
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,  &clilen);
  if (newsockfd < 0) error("ERROR on accept");
  printf("done setting up listen\n");
  read_from_socket(newsockfd, buffer, BUFFER_SIZE);
  write_to_socket(newsockfd);
  close_socket(sockfd, newsockfd);

  return 0; 
}
