#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PROTOCOL_MAGIC_1 0xAA
#define PROTOCOL_MAGIC_2 0x55

bool decode_custom_packet(const uint8_t* data, uint8_t len, char* out, uint8_t out_size);
uint8_t calc_checksum(const uint8_t* data, uint8_t len);
