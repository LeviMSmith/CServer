#ifndef PACKETS_H_
#define PACKETS_H_

#include "uuid4/uuid4.h"

#include <stddef.h>
#include <stdint.h>

enum PacketType: uint8_t {
  PACKET_INIT,
  PACKET_REQUEST,
  PACKET_DATA
};

enum ServerMode: uint8_t {
  SERVER_MODE_ECHO,
  SERVER_MODE_FILE
};

extern size_t MAX_TOTAL_PACKET_SIZE; // For buffers. It's much less than this.

// Both
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
  uint8_t data[1024];
  uint16_t data_len;
} PacketData;

int packet_parse_header(char* message, int message_len, PacketHeader* header);
int packet_serialize_header(char* message, PacketHeader* header);

#endif // PACKETS_H_
