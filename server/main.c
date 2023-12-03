#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "uuid4/uuid4.h"

#include "utils/packets.h"

// The protocol:
//   A normal client packet contains the server session
//   and then a request. The request is a string that is parsed depending on
//   a stateful mode for the client.
//
//   The server returns the session id and the request.
//
//   Create session:
//     Client sends mode
//     Server returns session id
//
//   Request:
//     Client sends request
//     Server returns data

const uint16_t PORT = 4000;

int main() {
  // char uuid[UUID4_LEN];
  //
  // uuid4_init();
  // uuid4_generate(uuid);
  // printf("%s\n", uuid);
  

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    return EXIT_FAILURE;
  }

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    return EXIT_FAILURE;
  }

  struct sockaddr_in address;
  int addrlen = sizeof(address);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    return EXIT_FAILURE;
  }

  if (listen(server_fd, 10) < 0) {
    perror("listen");
    return EXIT_FAILURE;
  }

  while (1) {
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
      perror("accept");
      close(new_socket);
      continue;
    }

    char message[2048] = {0};
    int message_len = recv(new_socket, message, sizeof(message), 0);

    PacketHeader header;
    packet_parse_header(message, message_len, &header);
    
    printf("%s\n", header.session);
    printf("%u\n", header.type);

    close(new_socket);
  }

  close(server_fd);

  return EXIT_SUCCESS;
}
