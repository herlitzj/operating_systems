#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define MAX_MESSAGE_LEN 100000

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
  printf("CLIENT BUFFER CONTENTS: %s\n", buffer);
  // while (bytes_read < x) {
  //   result = read(socket, buffer + bytes_read, x - bytes_read);
  //   bytes_read += result;
  // }
}

void get_file_text(char *buffer, char *file_location) {
  FILE *f = fopen(file_location, "r");
  if (f != NULL) {
    size_t newLen = fread(buffer, sizeof(char), MAX_MESSAGE_LEN, f);
    if ( ferror( f ) != 0 ) {
      perror("Error reading file");
      exit(1);
    } else {
      buffer[newLen++] = '\0';
    }
    fclose(f);
  } else {
    perror("Error opening file");
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n, buffer_size = 1000;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[buffer_size];
  char plain_text[2048];
  char key[2048];

  if (argc < 4) {
    fprintf(stderr, "usage: %s [plaintext file] [key file] [port number]\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[3]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("ERROR opening socket");
  server = gethostbyname("localhost");

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

  get_file_text(plain_text, argv[1]);

  get_file_text(key, argv[2]);

  printf("Plain text length: %d\nContents: %s\n", strlen(plain_text), plain_text);
  printf("Key length: %d\nContents: %s\n", strlen(key), key);
  if(strlen(key) < strlen(plain_text)) {
    perror("Key is too short");
    exit(1);
  }

  // send header with length of plaintext
  unsigned int length = 0;
  length = strlen(plain_text);
  n = write(sockfd, &length, sizeof(length));
  if (n < 0) error("ERROR writing to socket");

  // read response from server
  unsigned int response = 0;
  read_from_socket(sockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  if (response == 200) printf("RESPONSE: %i SUCCESS\n", response);
  else printf("RESPONSE: 500 SERVER ERROR\n");

  // write plaintext to server
  n = write(sockfd, plain_text, strlen(plain_text));
  if (n < 0) error("ERROR writing to socket");

  // read response from server
  read_from_socket(sockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  if (response == 200) printf("RESPONSE: %i SUCCESS\n", response);
  else printf("RESPONSE: 500 SERVER ERROR\n");

  // write length of key to server
  length = strlen(key);
  n = write(sockfd, &length, sizeof(length));
  if (n < 0) error("ERROR writing to socket");

  // read response from server
  read_from_socket(sockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  if (response == 200) printf("RESPONSE: %i SUCCESS\n", response);
  else printf("RESPONSE: 500 SERVER ERROR\n");

  // write key to server
  n = write(sockfd, key, strlen(key));
  if (n < 0) error("ERROR writing to socket");

  // read response from server
  read_from_socket(sockfd, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");
  if (response == 200) printf("RESPONSE: %i SUCCESS\n", response);
  else printf("RESPONSE: 500 SERVER ERROR\n");

  // read header from server with length of cipher
  read_from_socket(sockfd, sizeof(length), (void *)&length);
  printf("Client: Recieved length: %i\n", length);

  // send response to sever
  response = 200;
  n = write(sockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");

  // read ciphertext from the server
  char cipher_buffer[length];
  read_from_socket(sockfd, length, cipher_buffer);
  printf("Client: Recieved ciphertext: %s\n", cipher_buffer);
  printf("Client: Ciphertext length: %i\n", strlen(cipher_buffer));

  // send response to server
  response = 200;
  n = write(sockfd, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");
  
  close(sockfd);
  
  return 0;
}