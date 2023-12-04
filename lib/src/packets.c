#include "utils/packets.h"

#include <netinet/in.h>
#include <string.h>
#include <stdint.h>

uint64_t ntohll(uint64_t netlonglong) {
    uint32_t high_part = (uint32_t)(netlonglong >> 32);
    uint32_t low_part = (uint32_t)(netlonglong & 0xFFFFFFFF);

    high_part = ntohl(high_part);
    low_part = ntohl(low_part);

    return ((uint64_t)low_part << 32) | high_part;
}

uint64_t htonll(uint64_t value) {
    uint32_t high_part = htonl((uint32_t)(value >> 32));
    uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));

    return ((uint64_t)low_part << 32) | high_part;
}

// This is terrible for security 👍
int packet_parse_header(const char* message, int message_len, PacketHeader* header) {
  header->type = message[0];

  return 0;
}

int packet_serialize_header(char* message, const PacketHeader* header) {
  message[0] = (char)header->type;

  return 0;
}

int packet_parse_init(const char* message, int message_len, PacketInit* packet) {
  packet->mode = (uint8_t)message[sizeof(PacketHeader) + 1];

  return 0;
}

int packet_serialize_init(char* message, const PacketInit* packet) {
  message[sizeof(PacketHeader) + 1] = (char)packet->mode;

  return 0;
}

int packet_parse_request(const char* message, int message_len, PacketRequest* packet) {
  strcpy(packet->request, &message[sizeof(PacketHeader) + 1]);

  return 0;
}

int packet_serialize_request(char* message, const PacketRequest* packet) {
  strcpy(&message[sizeof(PacketHeader) + 1], packet->request);

  return 0;
}

int packet_parse_data(const char* message, int message_len, PacketData* packet) {
  size_t offset = sizeof(PacketHeader) + 1;
  memcpy(&packet->segment, &message[offset], sizeof(uint64_t));
  packet->segment = ntohll(packet->segment);
  offset += sizeof(uint64_t);
  memcpy(&packet->data_len, &message[offset], sizeof(uint16_t));
  packet->data_len = ntohs(packet->data_len);
  offset += sizeof(uint16_t);
  memcpy(&packet->data, &message[offset], packet->data_len);

  return 0;
}

int packet_serialize_data(char* message, const PacketData* packet) {
    size_t offset = sizeof(PacketHeader) + 1;

    uint64_t segment_net = htonll(packet->segment);
    memcpy(&message[offset], &segment_net, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    uint16_t data_len_net = htons(packet->data_len);
    memcpy(&message[offset], &data_len_net, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&message[offset], packet->data, packet->data_len);

    return 0; // Success
}
