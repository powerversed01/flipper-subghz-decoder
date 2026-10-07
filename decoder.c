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

void encode_custom_packet(const char* msg, uint8_t* out, uint8_t* out_len) {
    uint8_t msg_len = strlen(msg);
    if(msg_len > 255) msg_len = 255;

    out[0] = PROTOCOL_MAGIC_1;
    out[1] = PROTOCOL_MAGIC_2;
    out[2] = msg_len;

    memcpy(&out[3], msg, msg_len);

    uint8_t checksum = calc_checksum((const uint8_t*)msg, msg_len);
    out[3 + msg_len] = checksum;

    *out_len = 4 + msg_len;
}
