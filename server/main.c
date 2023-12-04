#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int send_buffer(int socket, const char* buffer, size_t buffer_size) {
  PacketData packet_data;
  PacketHeader packet_header;
  char packet[MAX_TOTAL_PACKET_SIZE];
  size_t remaining = buffer_size;
  uint64_t segment = 0;

  while (remaining > 0) {
    size_t chunk_size = remaining > MAX_DATA_SIZE ? MAX_DATA_SIZE : remaining;

    packet_data.segment = segment;
    packet_data.data_len = chunk_size;
    strncpy(packet_data.data, (char *)buffer + (buffer_size - remaining), chunk_size);

    packet_header.type = PACKET_DATA;

    packet_serialize_header(packet, &packet_header);
    packet_serialize_data(packet, &packet_data);

    // Send packet
    ssize_t sent = send(socket, packet, sizeof(PacketHeader) + 1 + sizeof(uint64_t) + sizeof(uint16_t) + chunk_size, 0);
    if (sent == -1) {
      perror("send");
      return -1;
    }

    remaining -= chunk_size;
    segment++;
  }

  return 0;
}

int main() {
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

    pid_t pid = fork();

    if (pid == -1) {
      perror("fork");
      close(new_socket);
    }
    else if(pid > 0) {
      // Parent doesn't need this socket
      close(new_socket);
      continue;
    }
    else {
      enum ServerMode mode;

      while (1) {
        char message[2048] = {0};
        int message_len = recv(new_socket, message, sizeof(message), 0);

        if (message_len == -1) {
          perror("recv");
          break; // Error occurred
        } else if (message_len == 0) {
          printf("Client disconnected\n");
          break; // Connection closed
        }

        PacketHeader header;
        packet_parse_header(message, message_len, &header);
        
        printf("%u\n", header.type);

        switch (header.type) {
          case PACKET_INIT: {
            PacketInit packet;
            packet_parse_init(message, message_len, &packet);

            char return_message[MAX_TOTAL_PACKET_SIZE];
            PacketHeader header;
            header.type = PACKET_INIT;

            packet_serialize_header(return_message, &header);

            PacketInit given_init;
            packet_parse_init(message, message_len, &given_init);

            PacketInit return_init;
            return_init.mode = given_init.mode;

            mode = return_init.mode;
            
            packet_serialize_init(return_message, &return_init);

            send(new_socket, return_message, sizeof(PacketHeader) + sizeof(PacketInit), 0);
            
            break;
          }
          case PACKET_REQUEST: {
            PacketRequest request;
            packet_parse_request(message, message_len, &request);

            switch (mode) {
              case SERVER_MODE_ECHO: {
                printf("ECHO: %s\n", request.request);

                if (strcmp(request.request, "close")) {
                  break;
                }

                send_buffer(new_socket, request.request, strlen(request.request));

                break;
              }
              case SERVER_MODE_FILE: {
                char full_path[1024];
                FILE* file;
                char* buffer;
                size_t file_length;

                snprintf(full_path, sizeof(full_path), "%s/%s", "public", request.request);

                file = fopen(full_path, "rb");
                if (file == NULL) {
                  perror("Error opening file");
                  return NULL;
                }

                fseek(file, 0, SEEK_END);
                file_length = ftell(file);
                rewind(file);

                buffer = malloc(file_length + 1);
                if (buffer == NULL) {
                  perror("Error allocating memory");
                  fclose(file);
                  return NULL;
                }

                fread(buffer, 1, file_length, file);
                buffer[file_length] = '\0';

                fclose(file);

                send_buffer(new_socket, buffer, file_length + 1);

                free(buffer);

                break;
              }
              default: {
                printf("Got bad server mode\n");
                break;
              }
            }
          }
          default:
            printf("Got bad packet\n");
        } // packet switch
      } // child while

      close(new_socket);
      return EXIT_SUCCESS;
    } // fork if child
  } // parent while

  close(server_fd);

  return EXIT_SUCCESS;
}
