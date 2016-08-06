/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

char encrypt_char(char plain, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int plain_index = 0, key_index = 0;
  if(plain == '\n') return plain;
  if(plain == EOF) return '\0';
  for(position; position < 28; position++) {
    if (plain == dictionary[position]) plain_index = position;
    if (key == dictionary[position]) key_index = position;
  }

  plain_index = (plain_index + key_index) % 27;
  return dictionary[plain_index];
}

void encrypt(char *plain, int plain_size, char *key) {
  int i=0;
  while(plain[i] != EOF) {
    //if(key[i] < 0) {
    //  perror("Key too short");
    //  exit(1);
    //}

    plain[i] = encrypt_char(plain[i], key[i]);
    i++;
  }

  // replace the last line break with a null char
  plain[i-2] = '\0';
  
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, buffer_size = 1000;
  socklen_t clilen;
  char plain_buffer[buffer_size], key_buffer[buffer_size];
  struct sockaddr_in serv_addr, cli_addr;
  int n, m=0;

  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
    sizeof(serv_addr)) < 0) 
    error("ERROR on binding");

  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

  if (newsockfd < 0) error("ERROR on accept");
  bzero(plain_buffer, buffer_size);
  n = read(newsockfd, plain_buffer, buffer_size - 1);

  if(strcmp(plain_buffer, "exit") == 0) m = -1;

  if (n < 0) error("ERROR reading from socket");
  
  printf("Here is the plaintext: %s\n", plain_buffer);

  n = read(newsockfd, key_buffer, buffer_size - 1);

  if (n < 0) error("ERROR reading from socket");

  encrypt(plain_buffer, strlen(plain_buffer), key_buffer);

  n = write(newsockfd, plain_buffer, strlen(plain_buffer));

  if (n < 0) error("ERROR writing to socket");

  close(newsockfd);
  close(sockfd);

  return 0; 
}
