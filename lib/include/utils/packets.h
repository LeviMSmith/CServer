#ifndef PACKETS_H_
#define PACKETS_H_

#include <stddef.h>
#include <stdint.h>

enum PacketType: uint8_t {
  PACKET_NONE,
  PACKET_INIT,
  PACKET_REQUEST,
  PACKET_DATA
};

enum ServerMode: uint8_t {
  SERVER_MODE_NONE,
  SERVER_MODE_ECHO,
  SERVER_MODE_FILE
};

#define MAX_TOTAL_PACKET_SIZE 2024
#define MAX_DATA_SIZE 1024

typedef struct {
  enum PacketType type;
} PacketHeader;

// Only client
typedef struct {
  enum ServerMode mode;
} PacketInit;
        
// Only client
typedef struct {
  char request[1024]; // Assumes null terminated
} PacketRequest;

// Only server
typedef struct {
  uint64_t segment;
  uint16_t data_len;
  char data[MAX_DATA_SIZE];
} PacketData;

int packet_parse_header(const char* message, int message_len, PacketHeader* packet);
int packet_serialize_header(char* message, const PacketHeader* packet);

int packet_parse_init(const char* message, int message_len, PacketInit* packet);
int packet_serialize_init(char* message, const PacketInit* packet);

int packet_parse_request(const char* message, int message_len, PacketRequest* packet);
int packet_serialize_request(char* message, const PacketRequest* packet);

int packet_parse_data(const char* message, int message_len, PacketData* packet);
int packet_serialize_data(char* message, const PacketData* packet);

#endif // PACKETS_H_
