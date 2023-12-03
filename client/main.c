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
  uuid4_init();
  uuid4_generate(header.session); // Later we get this from the server
  header.type = PACKET_INIT;

  packet_serialize_header(message, &header);

  int n = send(sockfd, message, strlen(message), 0);
  if (n < 0) {
    perror("ERROR writing to socket");
    close(sockfd);
    return EXIT_FAILURE;
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
