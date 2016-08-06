#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n, buffer_size = 1000;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[buffer_size];

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("ERROR opening socket");
  server = gethostbyname(argv[1]);

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    error("ERROR connecting");

  printf("Please enter the message: ");
  bzero(buffer, buffer_size);
  fgets(buffer, buffer_size - 1, stdin);
  n = write(sockfd, buffer, strlen(buffer));

  if (n < 0) error("ERROR writing to socket");

  printf("Please enter the key: ");
  bzero(buffer, buffer_size);
  fgets(buffer, buffer_size - 1, stdin);
  n = write(sockfd, buffer, strlen(buffer));

  if (n < 0) error("ERROR writing to socket");

  bzero(buffer, buffer_size);
  n = read(sockfd, buffer, buffer_size -1);

  if (n < 0) error("ERROR reading from socket");

  printf("GOT A REPLY %s\n", buffer);
  close(sockfd);
  
  return 0;
}
