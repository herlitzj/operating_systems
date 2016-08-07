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

void read_from_socket(int socket, unsigned int x, void* buffer) {
  int bytes_read = 0;
  int result;
  result = read(socket, buffer, x);
  if (result < 1 ) {
    error("Server Error: Cannot read from socket");
  }
}

char decrypt_char(char cipher, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int cipher_index = 0, key_index = 0;
  if(cipher == '\n') return cipher;
  if(cipher == EOF) return '\0';
  for(position; position < 28; position++) {
    if (cipher == dictionary[position]) cipher_index = position;
    if (key == dictionary[position]) key_index = position;
  }

  //deal with C's crappy modulo
  cipher_index = cipher_index  - key_index;
  if(cipher_index < 0) cipher_index = 27 + cipher_index;

  return dictionary[cipher_index];
}

void encrypt(char *cipher, int length, char *key) {
  int i=0;
  while(cipher[i] != '\0') {
    cipher[i] = decrypt_char(cipher[i], key[i]);
    i++;
  }
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  int32_t buffer_size = 1000;
  socklen_t clilen;
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
  
  // read handshake
  unsigned int length = 0;
  read_from_socket(newsockfd, sizeof(length), (void *)&length);

  // send response to client
  unsigned int response = 200;
  unsigned int bad_request = 400;
  if(length == 12345) n = write(newsockfd, &response, sizeof(response));
  else n = write(newsockfd, &bad_request, sizeof(bad_request));
  if (n < 0) error("ERROR writing to socket");

  // read header from client with length of cipher
  read_from_socket(newsockfd, sizeof(length), (void *)&length);
  unsigned int cipher_length = length;

  // send response to client
  n = write(newsockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");

  // read cipher from the client
  char cipher_buffer[length];
  read_from_socket(newsockfd, length, cipher_buffer);

  // send response to client
  n = write(newsockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");

  // read header from client with length of key
  read_from_socket(newsockfd, sizeof(length), (void *)&length);

  // send response to client
  n = write(newsockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");

  // read key from the client
  char key_buffer[length];
  read_from_socket(newsockfd, length, key_buffer);

  // send response to client
  n = write(newsockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");

  // encrypt message
  encrypt(cipher_buffer, cipher_length, key_buffer);

  // send header with length of cipher
  n = write(newsockfd, &cipher_length, sizeof(cipher_length));
  if (n < 0) error("ERROR writing to socket");

  // read response from client
  read_from_socket(newsockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  // if (response == 200) printf("SERVER: %i SUCCESS\n", response);
  // else printf("RESPONSE: 500 CLIENT ERROR\n");

  // write ciphertext to client
  n = write(newsockfd, cipher_buffer, cipher_length);
  if (n < 0) error("ERROR writing to socket");

  // read response from client
  read_from_socket(newsockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  // if (response == 200) printf("SERVER: %i SUCCESS\n", response);
  // else printf("RESPONSE: 500 CLIENT ERROR\n");

  close(newsockfd);
  close(sockfd);

  return 0; 
}