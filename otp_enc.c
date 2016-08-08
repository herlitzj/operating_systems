#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define RESPONSE_OK 200
#define RESPONSE_BAD_REQUEST 400
#define RESPONSE_INTERNAL_ERROR 500
#define MAX_MESSAGE_LEN 100000
#define ENC_HANDSHAKE 54321
#define USAGE "otp_enc [plaintext] [key] [port] [&]"

void error(const char *msg) {
  printf("Client: ");
  perror(msg);
  exit(1);
}

void read_from_socket(int socket, unsigned int message_length, void* message, int retries) {
  int bytes_read = 0;
  int result;

  if(retries > 5) {
    close(socket);
    error("Error reading from socket. Too many failed attempts");
  }

  result = read(socket, message, message_length);
  if (result < 1 ) {
    read_from_socket(socket, message_length, message, retries++);
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

void get_file_text(char *buffer, char *file_location) {
  FILE *f = fopen(file_location, "r");
  if (f != NULL) {
    size_t newLen = fread(buffer, sizeof(char), MAX_MESSAGE_LEN, f);
    if ( ferror( f ) != 0 ) {
      error("Error reading file\n");
    } else {
      buffer[newLen++] = '\0';
    }
    fclose(f);
  } else {
    error("Error opening file");
  }
}

void intiate_handshake(int socket, int retries) {
  int n;
  unsigned int handshake = ENC_HANDSHAKE;
  unsigned int response = 0;

  if(retries > 5) {
    close(socket);
    error("Error writing to socket. Too many failed attempts");
  }

  // send handshake
  n = write(socket, &handshake, sizeof(handshake));
  if (n < 0) {
    intiate_handshake(socket, retries++);
  } else {
    // read handshake response from server
    read_from_socket(socket, sizeof(response), (void *)&response, 0);
    if(response == 400) {
      close(socket);
      error("Connection declined by server");
    }
  }

}

void send_message(int socket, char *message_buffer, int retries) {
  int n;
  unsigned int response = 0;
  unsigned int message_length = strlen(message_buffer) + 1;

  if(retries > 5) {
    close(socket);
    error("Error sending buffer to client. Too many failed attempts");
  }

  // send header with length of message
  write_to_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // read response from server
  read_from_socket(socket, sizeof(response), (void *)&response, 0);

  if (response == 200) {
    // write plaintext to sever
    write_to_socket(socket, message_length, (void *)message_buffer, 0);

    // read response from server
    read_from_socket(socket, sizeof(response), (void *)&response, 0);
  }

  if (response == 200) {
  } else {
    send_message(socket, message_buffer, retries++);
  }
}

char *get_from_server(int socket) {
  int n;
  unsigned int message_length = 0;
  unsigned int response_ok = RESPONSE_OK;

  // read header from server with length of message
  read_from_socket(socket, sizeof(message_length), (void *)&message_length, 0);

  // send OK response to server
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok);

  // read message from the server
  char *temp_buffer = malloc(sizeof (char) *message_length);
  read_from_socket(socket, message_length, temp_buffer, 0);

  // send OK response to server
  write_to_socket(socket, sizeof(response_ok), (void *)&response_ok);

  return temp_buffer;
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n, buffer_size = 1000;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char plain_text[MAX_MESSAGE_LEN];
  char key[MAX_MESSAGE_LEN];

  if (argc < 4) {
    error(USAGE);
  }

  portno = atoi(argv[3]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("Error opening socket");
  server = gethostbyname("localhost");

  if (server == NULL) {
    error("Error, no such host");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    error("Error connecting");

  // read the files (ciphertext and key) into buffers for sending
  get_file_text(plain_text, argv[1]);
  get_file_text(key, argv[2]);

  if(strlen(key) < strlen(plain_text)) {
    error("Key is too short\n");
  }

  // send the handshake to the server
  intiate_handshake(sockfd, 0); 

  // send the ciphertext and key to the server
  send_message(sockfd, plain_text, 0);
  send_message(sockfd, key, 0); 

  // get the plaintext from the server
  char *cipher_text = get_from_server(sockfd);

  // print the decrypted message to stdout
  printf("%s\n", cipher_text);

  free(cipher_text);
  close(sockfd);
  
  return 0;
}