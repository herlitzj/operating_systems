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

void error(const char *msg) {
  perror(msg);
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

void encrypt(char *plain, int length, char *key) {
  int i=0;
  while(plain[i] != '\0') {
    plain[i] = encrypt_char(plain[i], key[i]);
    i++;
  }
}

void handshake_response(int socket) {
  int n;
  unsigned int entry_key, response;

  read_from_socket(socket, sizeof(entry_key), (void *)&entry_key, 0);

  response = entry_key == ENC_HANDSHAKE ? RESPONSE_OK : RESPONSE_BAD_REQUEST;
  
  n = write(socket, &response, sizeof(response));
  if (n < 0) error("ERROR writing to socket");
  if (response == 400) error("BAD REQUEST");
}

char *get_from_client(int socket) {
  int n;
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from client with length of message
  read_from_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket\n");

  // read message from the client
  char *temp_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, temp_buffer, 0);

  // send response to client
  n = write(socket, &response_ok, sizeof(response_ok));
  if (n < 0) error("ERROR writing to socket\n");

  return temp_buffer;

}

// char *get_plaintext(int socket) {
//   int n;
//   unsigned int message_length = 0;
//   unsigned int response_ok = RESPONSE_OK;

//   // read header from client with length of cipher
//   read_from_socket(socket, sizeof(message_length), (void *)&message_length);

//   // send response to client
//   n = write(socket, &response_ok, sizeof(response_ok));
//   if (n < 0) error("ERROR writing to socket");

//   // read cipher from the client
//   char *plain_buffer = malloc(sizeof (char) *message_length);
//   read_from_socket(socket, message_length, plain_buffer);

//   // send response to client
//   n = write(socket, &response_ok, sizeof(response_ok));
//   if (n < 0) error("ERROR writing to socket");

//   return plain_buffer;
// }

// char *get_key(int socket) {
//   int n;
//   unsigned int message_length = 0;
//   unsigned int response_ok = RESPONSE_OK;

//   // read header from client with length of key
//   read_from_socket(socket, sizeof(message_length), (void *)&message_length);

//   // send response to client
//   n = write(socket, &response_ok, sizeof(response_ok));
//   if (n < 0) error("ERROR writing to socket");

//   // read key from the client
//   char *key_buffer = malloc(sizeof (char) *message_length);
//   read_from_socket(socket, message_length, key_buffer);

//   // send response to client
//   n = write(socket, &response_ok, sizeof(response_ok));
//   if (n < 0) error("ERROR writing to socket");

//   return key_buffer;

// }

void send_to_client(int socket, char *cipher_buffer) {
  int n;
  unsigned int response = 0;
  unsigned int message_length = strlen(cipher_buffer) + 1;

  // send header with length of cipher
  n = write(socket, &message_length, sizeof(message_length));
  if (n < 0) error("ERROR writing to socket");

  // read response from client
  read_from_socket(socket, sizeof(response), (void *)&response, 0);

  if (response == 200) {
    // write ciphertext to client
    n = write(socket, cipher_buffer, message_length);
    if (n < 0) error("ERROR writing to socket");

    // read response from client
    read_from_socket(socket, sizeof(response), (void *)&response, 0);
  }

  if (response == 200) {
    close(socket);
  } else {
    error("Error writing buffer to client");
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

  if (sockfd < 0) error("Error opening socket\n");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    error("Error on binding\n");

  while(1) {
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept\n");

    if((pid = fork()) < 0) {
      error("Error forking child process");
    } else if (pid == 0) { //handle the child fork
      // read handshake
      handshake_response(newsockfd);

      // get message and key from client
      char *plain_to_cipher_buffer = get_from_client(newsockfd);
      char *key_buffer = get_from_client(newsockfd);

      // encrypt the message
      encrypt(plain_to_cipher_buffer, strlen(plain_to_cipher_buffer) + 1, key_buffer);

      // send the cipher back to the client
      send_to_client(newsockfd, plain_to_cipher_buffer);

      // free memory
      free(plain_to_cipher_buffer);
      free(key_buffer);
      
    } else { // handle the parent fork
      close(newsockfd);
    }
  }
}