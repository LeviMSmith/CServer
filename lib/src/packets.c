#include "uuid4/uuid4.h"

#include "utils/packets.h"

#include <string.h>

size_t MAX_TOTAL_PACKET_SIZE = 2048;

int packet_parse_header(char* message, int message_len, PacketHeader* header) {
  // This is terrible for security 👍
  strncpy(header->session, message, UUID4_LEN); // Should copy over null terminator
  header->type = message[UUID4_LEN + 1];

  return 0;
}

int packet_serialize_header(char* message, PacketHeader* header) {
  strncpy(message, header->session, UUID4_LEN);
  message[UUID4_LEN + 1] = (char)header->type;

  return 0;
}
