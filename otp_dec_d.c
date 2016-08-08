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
#define DEC_HANDSHAKE 12345
#define USAGE "otp_dec_d [port] [&]"

void error(const char *msg) {
  fprintf(stderr, "Server: ");
  fprintf(stderr, msg);
  fprintf(stderr, "\n");
  exit(1);
}

void read_from_socket(int socket, unsigned int x, void* buffer, int retries) {
  int bytes_read = 0;
  int result;

  if(retries > 5) {
    close(socket);
    error("Server Error: Cannot read from socket");
  }
  result = read(socket, buffer, x);
  if (result < 1 ) {
    read_from_socket(socket, x, buffer, retries++);
  }
}

void write_to_socket(int socket, unsigned int message_length, void* message, int retries) {
  int result;

  if(retries > 5) {
    close(socket);
    error("Error writing to socket. Too many failed attempts");
  }

  result = write(socket, message, message_length);
  if (result < 1 ) {
    write_to_socket(socket, message_length, message, retries++);
  }
}

char decrypt_char(char cipher, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int cipher_index = 0, key_index = 0;
  if(cipher == '\n' || cipher == '\0') return cipher;
  if(cipher == EOF) return cipher;
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
  for(i; i < length; i++) {
    cipher[i] = decrypt_char(cipher[i], key[i]);
  }
  // while(cipher[i] != '\0') {
  //   cipher[i] = decrypt_char(cipher[i], key[i]);
  //   i++;
  // }
}

void handshake_response(int socket) {
  int n;
  unsigned int entry_key, response;

  read_from_socket(socket, sizeof(entry_key), (void *)&entry_key, 0);

  response = entry_key == DEC_HANDSHAKE ? RESPONSE_OK : RESPONSE_BAD_REQUEST;
  
  n = write(socket, &response, sizeof(response));
  if (n < 0) error("error writing to socket");
  if (response == 400) error("Bad request");
}

char *get_from_client(int socket) {
  int n;
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from client with length of message
  read_from_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // send OK response to client
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok, 0);

  // read message from the client
  char *temp_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, temp_buffer, 0);

  // send response to client
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok, 0);

  return temp_buffer;
}

void send_to_client(int socket, char *message_buffer, int retries) {
  int n;
  unsigned int response = 0;
  unsigned int message_length = strlen(message_buffer) + 1;

  if(retries > 5) {
    close(socket);
    error("Error sending message to client. Too many failed attempts");
  }

  // send header with length of message
  write_to_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // read response from client
  read_from_socket(socket, sizeof(response), (void *)&response, 0);

  if (response == 200) {
    // write plaintext to client
    write_to_socket(socket, message_length, (void *)message_buffer, 0);

    // read response from client
    read_from_socket(socket, sizeof(response), (void *)&response, 0);
  } else {
    send_to_client(socket, message_buffer, retries++);
  }

  if (response == 200) {
    close(socket);
  } else {
    send_to_client(socket, message_buffer, retries++);
  }
}

int main(int argc, char *argv[]) {
  pid_t pid, wpid;
  int sockfd, newsockfd, portno, status;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n, m=0;

  if (argc < 2) {
    error(USAGE);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("Error opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
    sizeof(serv_addr)) < 0) 
    error("Error on binding");

  while(1) {
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("Error on accept");

    if((pid = fork()) < 0) {
      error("Error forking child process");
    } else if (pid == 0) { //handle the child fork
      // read handshake
      handshake_response(newsockfd);

      // get cipher and key from client
      char *cipher_to_plain_buffer = get_from_client(newsockfd);
      char *key_buffer = get_from_client(newsockfd);

      // decrypt the cipher
      decrypt(cipher_to_plain_buffer, strlen(cipher_to_plain_buffer) + 1, key_buffer);

      // send the plaintext back to the client
      send_to_client(newsockfd, cipher_to_plain_buffer, 0);

      // free memory
      free(cipher_to_plain_buffer);
      free(key_buffer);
      
    } else { // handle the parent fork
      close(newsockfd);
    }
  }
}