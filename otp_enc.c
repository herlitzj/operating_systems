#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define MAX_MESSAGE_LEN 100000
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void read_from_socket(int socket, unsigned int x, void* buffer) {
  int bytesRead = 0;
  int result;
  while (bytesRead < x) {
    result = read(socket, buffer + bytesRead, x - bytesRead);
    printf("RESULT: %i\n", result);
    if (result < 1 ) {
      error("Error reading from socket X");
    }
    bytesRead += result;
  }
}

void write_to_socket(int socket, int x, void* buffer) {
  int bytes_written = 0;
  int result;
  while (bytes_written < x) {
    result = write(socket, buffer + bytes_written, x - bytes_written);
    if (result < 1 ) {
      error("Error writting to socket");
    }
    bytes_written += result;
  }
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
  int length;

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

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(2);
  }

  get_file_text(plain_text, argv[1]);

  get_file_text(key, argv[2]);

  //printf("Plain text length: %d\nContents: %s\n", strlen(plain_text), plain_text);
  //printf("Key length: %d\nContents: %s\n", strlen(key), key);
  if(strlen(key) < strlen(plain_text)) {
    perror("Key is too short");
    exit(1);
  }

  // length = strlen(plain_text);
  // printf("WLEN: %i\n", length);
  // write_to_socket(sockfd, sizeof(length), (void*)(&length));
  // write_to_socket(sockfd, length, (void*)plain_text);
  // printf("WBOD: %s\n", plain_text);

  // length = strlen(key);
  // printf("WLEN: %i\n", length);
  // write_to_socket(sockfd, sizeof(length), (void*)(&length));
  // write_to_socket(sockfd, length, (void*)key);
  // printf("WBOD: %s\n", key);


  length = strlen(plain_text);
  n = write(sockfd, &length, sizeof(int));
  if (n < 1) error("ERROR writing header to socket");

  n = write(sockfd, plain_text, strlen(plain_text));
  if (n < 1) error("ERROR writing to socket");

  length = strlen(key);
  n = write(sockfd, &length, sizeof(int));
  if (n < 1) error("ERROR writing header to socket");

  n = write(sockfd, key, strlen(key));
  if (n < 1) error("ERROR writing to socket");
  
  read_from_socket(sockfd, sizeof(length), (void*)(&length));
  char encrypted_buffer[length];
  read_from_socket(sockfd, length, (void*)encrypted_buffer);

  // n = read(sockfd, buffer, strlen(plain_text));
  // if (n < 0) error("ERROR reading from socket");

  printf("Encrypted Text:\n%s\n", encrypted_buffer);
  close(sockfd);
  
  return 0;
}
