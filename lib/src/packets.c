#include "uuid4/uuid4.h"

#include "utils/packets.h"

#include <string.h>

int packet_parse_header(char* message, int message_len, PacketHeader* header) {
  // This is terrible for security 👍
  strncpy(header->session, message, UUID4_LEN); // Should copy over null terminator
  header->type = message[UUID4_LEN + 1];

  return 0;
}
