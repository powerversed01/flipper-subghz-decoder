#include "decoder.h"

uint8_t calc_checksum(const uint8_t* data, uint8_t len) {
    uint8_t sum = 0;
    for(uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum & 0xFF;
}

bool decode_custom_packet(const uint8_t* data, uint8_t len, char* out, uint8_t out_size) {
    if(len < 5) {
        return false;
    }

    if(data[0] != PROTOCOL_MAGIC_1 || data[1] != PROTOCOL_MAGIC_2) {
        return false;
    }

    uint8_t payload_len = data[2];
    if(payload_len == 0 || payload_len + 4 > len) {
        return false;
    }

    const uint8_t* payload = &data[3];
    uint8_t expected = data[3 + payload_len];
    uint8_t actual = calc_checksum(payload, payload_len);

    if(expected != actual) {
        return false;
    }

    if(payload_len >= out_size) {
        return false;
    }

    for(uint8_t i = 0; i < payload_len; i++) {
        out[i] = (char)payload[i];
    }
    out[payload_len] = '\0';

    return true;
}
