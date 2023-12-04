#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "utils/packets.h"

void echo(int socket, const char* input) {
  char message[MAX_TOTAL_PACKET_SIZE] = {0};

  PacketHeader packet_header;
  PacketRequest packet_request;
  strcpy(packet_request.request, input);

  packet_header.type = PACKET_REQUEST;

  packet_serialize_header(message, &packet_header);
  packet_serialize_request(message, &packet_request);
  
  int n = send(socket, message, MAX_TOTAL_PACKET_SIZE, 0);
  if (n < 0) {
    perror("ERROR writing to socket");
  }

  memset(message, 0, MAX_TOTAL_PACKET_SIZE);

  int message_len = recv(socket, message, sizeof(message), 0);
  printf("Message of len %d recieved\n", message_len);
  if (message_len == -1) {
    perror("recv");
    return;
  }
  else if (message_len == 0) {
    printf("Server disconnected\n");
    return;
  }

  PacketData* packet_data = malloc(sizeof(PacketData));

  packet_parse_data(message, message_len, packet_data);
  printf("Data len %u\n", packet_data->data_len);
  printf("ECHO recieved: %s\n", packet_data->data);

  free(packet_data);
}

void get_file(int socket, const char* input) {

}

int main() {
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("opening socket");
    return(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(4000);
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    perror("invalid address/ Address not supported");
    return(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connecting");
    return(EXIT_FAILURE);
  }

  int socket = sockfd;

  char message[MAX_TOTAL_PACKET_SIZE];

  PacketHeader header;
  header.type = PACKET_INIT;

  packet_serialize_header(message, &header);

  enum ServerMode mode;
  char input[256];

  printf("Choose a mode:\n");
  printf("1: Echo\n");
  printf("2: File get\n");
  printf("Enter your choice: ");
  scanf("%i", (int*)&mode);

  while (getchar() != '\n');
  
  if (mode != SERVER_MODE_ECHO && mode != SERVER_MODE_FILE) {
    printf("Invalid mode selected: %d.\n", mode);
    printf("Valid modes are %d and %d\n", SERVER_MODE_ECHO, SERVER_MODE_FILE);
    return 1;
  }

  printf("You selected Mode %d\n", mode);
  printf("Enter a string (type 'close' to exit):\n");

  PacketInit packet_init;
  packet_init.mode = mode;

  packet_serialize_init(message, &packet_init);

  int n = send(socket, message, MAX_TOTAL_PACKET_SIZE, 0);
  if (n < 0) {
    perror("ERROR writing to socket");
    close(sockfd);
    return EXIT_FAILURE;
  }

  int message_len = recv(socket, message, sizeof(message), 0);

  packet_parse_header(message, message_len, &header);

  int running = 1;
  // printf("%b", running);
  while (running) {
    printf("Input: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0; // Remove newline character

    if (strcmp(input, "close") == 0) {
      running = 0;
    } else {
      if (mode == SERVER_MODE_ECHO) {
        echo(socket, input);
      } else if (mode == SERVER_MODE_FILE) {
        get_file(socket, input);
      }
    }
  }

  printf("exiting");

  close(sockfd);

  return EXIT_SUCCESS;
}
