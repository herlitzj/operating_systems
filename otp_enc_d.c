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
#define ENC_HANDSHAKE 54321
#define USAGE "otp_enc_d [port] [&]"

// general error handler
void error(const char *msg) {
  fprintf(stderr, "Server: ");
  fprintf(stderr, msg);
  fprintf(stderr, "\n");
  exit(1);
}

// general function for reading messages off the socket
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

// general function for writing messages to the socket
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

// function for encypting a character based on input plaintext char and key char
char encrypt_char(char plain, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int plain_index = 0, key_index = 0;
  if(plain == '\n') return plain;
  for(position; position < 28; position++) {
    if (plain == dictionary[position]) plain_index = position;
    if (key == dictionary[position]) key_index = position;
  }

  plain_index = (plain_index + key_index) % 27;
  return dictionary[plain_index];
}

// main function for encrypting a text buffer
// loops over the buffer and encrypts each char
void encrypt(char *plain, int length, char *key) {
  int i=0;
  for(i; i <= length; i++) {
    printf("CH: %c, NU: %i\n", plain[i], plain[i]);
  }
  i=0;
  while(plain[i] != '\0') {
    plain[i] = encrypt_char(plain[i], key[i]);
    i++;
  }
}

// function for authenticating an incoming call from a client
// check to make sure client has sent the correct code
// if not, send error message and exit
void handshake_response(int socket) {
  int n;
  unsigned int entry_key, response;

  read_from_socket(socket, sizeof(entry_key), (void *)&entry_key, 0);

  response = entry_key == ENC_HANDSHAKE ? RESPONSE_OK : RESPONSE_BAD_REQUEST;
  
  n = write(socket, &response, sizeof(response));
  if (n < 0) error("Error writing to socket");
  if (response == 400) error("Bad request");
}

// function for reading messages from the client
// first it reads the header to get a message's length
// then it builds a temp buffer and reads the message into the buffer
// then it returns a pointer to the buffer
char *get_from_client(int socket) {
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from client with length of message
  read_from_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // send response to client
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok, 0);

  // read message from the client
  char *temp_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, temp_buffer, 0);

  // send response to client
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok, 0);

  return temp_buffer;
}

// function for reading messages from the client
// includes both a header with the message length
// and the body of the message
void send_to_client(int socket, char *message_buffer, int retries) {
  unsigned int response = 0;
  unsigned int message_length = strlen(message_buffer) + 1;

  // error out on too many failed attempts
  if(retries > 5) {
    close(socket);
    error("Error sending message to client. Too many failed attempts\n");
  }

  // send header with length of cipher
  write_to_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // read response from client
  read_from_socket(socket, sizeof(response), (void *)&response, 0);

  // if you get a 200 response for the header then send the body
  if (response == 200) {
    // write ciphertext to client
    write_to_socket(socket, message_length, (void *)message_buffer, 0);

    // read response from client
    read_from_socket(socket, sizeof(response), (void *)&response, 0);
  } else { // otherwise resend the message
    send_to_client(socket, message_buffer, retries++);
  }

  // if you get a 200 response for the body then close the socket
  if (response == 200) {
    close(socket);
  } else { // otherwise resend the message
    send_to_client(socket, message_buffer, retries++);
  }
}

// main function is generally the same as the sample server code
// the while loop is used to fork off processes from the pool
// and read/write to the client
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

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    error("Error on binding");

  while(1) {
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");

    if((pid = fork()) < 0) { // handle and error forking
      error("Error forking child process");
    } else if (pid == 0) { // handle the child fork
      // read handshake
      handshake_response(newsockfd);

      // get message and key from client
      char *plain_to_cipher_buffer = get_from_client(newsockfd);
      char *key_buffer = get_from_client(newsockfd);

      // encrypt the message
      encrypt(plain_to_cipher_buffer, strlen(plain_to_cipher_buffer) + 1, key_buffer);

      // send the cipher back to the client
      send_to_client(newsockfd, plain_to_cipher_buffer, 0);

      // free memory
      free(plain_to_cipher_buffer);
      free(key_buffer);
      
    } else { // handle the parent fork
      close(newsockfd);
    }
  }
}