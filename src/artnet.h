
// artnet.h
// defines / function prototypes for HTTP

#ifndef __ARTNET_H__
#define __ARTNET_H__

#include <stdint.h>
#include "pico/stdlib.h"

#include "dmx_output.h"

#define ARTNET_UDP_PORT 6454

// Magic Cookie, hat automatisch \0 am Ende
#define ARTNET_STRING "Art-Net"

#define ARTNET_OPCODE_ARTPOLL 0x2000
#define ARTNET_OPCODE_ARTDMX  0x5000

#define ARTNET_ARTPOLL_LEN (sizeof(artnet_header) - 2 * sizeof(uint16_t)) // volles Paket minus Universe und Length
#define ARTNET_MAX_LEN (sizeof(artnet_header) + DMX_UNIVERSE_SIZE + 8) // bisschen Toleranz

#define DMX_START_PIN 14 // Pin erstes Universum
#define UNIVERSE_COUNT 2
#define START_UNIVERSE 0

typedef struct {
    uint universe;

    dmx_output_t output;
    uint8_t data[DMX_UNIVERSE_SIZE + 1]; // +1 wegen Startbyte
} artnet_target_t;

// ArtNet Header
typedef struct __attribute__((__packed__)) {
    uint8_t artnet_string[8];
    uint16_t opcode;           
    uint16_t protocol_version; // Big Endian
    uint8_t sequence;
    uint8_t physical;
    uint16_t universe;
    uint16_t lenght;           // Big Endian
} artnet_header;

void artnet_init();

#endif