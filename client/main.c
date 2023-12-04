#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils/packets.h"

#include "uuid4/uuid4.h"

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

  char message[MAX_TOTAL_PACKET_SIZE];

  PacketHeader header;
  header.type = PACKET_INIT;
  // Doesn't matter what's in header.session

  packet_serialize_header(message, &header);

  PacketInit init;
  init.mode = SERVER_MODE_ECHO;

  packet_serialize_init(message, &init);

  int n = send(sockfd, message, MAX_TOTAL_PACKET_SIZE, 0);
  if (n < 0) {
    perror("ERROR writing to socket");
    close(sockfd);
    return EXIT_FAILURE;
  }

  int message_len = recv(sockfd, message, sizeof(message), 0);

  packet_parse_header(message, message_len, &header);

  char session[UUID4_LEN];
  strcpy(session, header.session);

  printf("Got session %s\n", session);

  PacketRequest request;
  strcpy(request.request, "hello");

  // session is still set
  header.type = PACKET_REQUEST;

  packet_serialize_header(message, &header);
  packet_serialize_request(message, &request);

  n = send(sockfd, message, MAX_TOTAL_PACKET_SIZE, 0);
  if (n < 0) {
    perror("ERROR writing to socket");
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
