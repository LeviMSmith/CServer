#ifndef PACKETS_H_
#define PACKETS_H_

#include "uuid4/uuid4.h"

#include <stddef.h>
#include <stdint.h>

enum PacketType: uint8_t {
  PACKET_INIT = 1,
  PACKET_REQUEST,
  PACKET_DATA
};

enum ServerMode: uint8_t {
  SERVER_MODE_ECHO = 1,
  SERVER_MODE_FILE
};

extern size_t MAX_TOTAL_PACKET_SIZE; // For buffers. It's much less than this.

typedef struct {
  char session[UUID4_LEN];
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
  uint8_t data[1024];
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
