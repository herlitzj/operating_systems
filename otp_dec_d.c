#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define RESPONSE_OK 200
#define RESPONSE_BAD_REQUEST 400
#define RESPONSE_INTERNAL_ERROR 500

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

void decrypt(char *cipher, int length, char *key) {
  int i=0;
  while(cipher[i] != '\0') {
    cipher[i] = decrypt_char(cipher[i], key[i]);
    i++;
  }
}

void verify_client(int socket) {
  int n;
  unsigned int entry_key, response;

  read_from_socket(socket, sizeof(entry_key), (void *)&entry_key);

  response = entry_key == 12345 ? RESPONSE_OK : RESPONSE_BAD_REQUEST;
  
  n = write(socket, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");
  if (response == 400) error("BAD REQUEST");
}

char *get_ciphertext(int socket) {
  int n;
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from client with length of cipher
  read_from_socket(socket, sizeof(message_length), (void *)&message_length);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket");

  // read cipher from the client
  char *cipher_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, cipher_buffer);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket");

  return cipher_buffer;
}

char *get_key(int socket) {
  int n;
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from client with length of key
  read_from_socket(socket, sizeof(message_length), (void *)&message_length);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket");

  // read key from the client
  char *key_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, key_buffer);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket");

  return key_buffer;

}

void send_plaintext(int socket, char *cipher_buffer) {
  int n;
  unsigned int response = 0;
  unsigned int cipher_length = strlen(cipher_buffer) + 1;

  // send header with length of cipher
  n = write(socket, &cipher_length, sizeof(cipher_length));
  if (n < 0) error("ERROR writing to socket");

  // read response from client
  read_from_socket(socket, sizeof(response), (void *)&response);
  if (n < 0) error("ERROR reading from socket");

  if (response == 200) {
    // write ciphertext to client
    n = write(socket, cipher_buffer, cipher_length);
    if (n < 0) error("ERROR writing to socket");

    // read response from client
    read_from_socket(socket, sizeof(response), (void *)&response);
    if (n < 0) error("ERROR reading from socket");    
  }

  if (response == 200) {
    close(socket);
  } else {
    error("Error writing buffer to client");
  }
}

int main(int argc, char *argv[])
{
  pid_t pid, wpid;
  int sockfd, newsockfd, portno, status;
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

  if((pid = fork()) < 0) {
    error("Error forking child process");
  } else if (pid == 0) { //handle the child fork
    // read handshake
    verify_client(newsockfd);

    // get cipher and key from client
    char *cipher_buffer = get_ciphertext(newsockfd);
    char *key_buffer = get_key(newsockfd);

    // decrypt the cipher
    decrypt(cipher_buffer, strlen(cipher_buffer) + 1, key_buffer);

    // send the plaintext back to the client
    send_plaintext(newsockfd, cipher_buffer);

    close(sockfd);
    free(cipher_buffer);
    free(key_buffer);
    
  } else { // handle the parent fork
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  
  // exit(0); 
}