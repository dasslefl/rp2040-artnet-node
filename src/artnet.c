
#include <string.h>

#include "artnet.h"

#include "lwip/def.h"
#include "lwip/udp.h"

static struct udp_pcb * udp_pcb;

// HACK: Daten werden in konsekutiven Puffer geladen
static uint8_t rx_buf[ARTNET_MAX_LEN];

// DMX-Zwischenpuffer
artnet_target_t universes[UNIVERSE_COUNT];

inline uint16_t reverse_16bit_endianness(uint16_t src) {
    return src << 8 | src >> 8;
}

void artnet_handle_packet(uint8_t * payload, uint payload_len) {

    if(payload_len < ARTNET_ARTPOLL_LEN) return;

    artnet_header * header = (artnet_header *) payload;

    // Magic Cookie falsch
    if(memcmp(header->artnet_string, ARTNET_STRING, sizeof(ARTNET_STRING)) != 0) return;

    uint16_t opcode = header->opcode;

    if(opcode == ARTNET_OPCODE_ARTPOLL) {
        // TODO ArtPoll
        printf("ArtPoll\n");
    } else if(opcode == ARTNET_OPCODE_ARTDMX) {
        // ArtDMX
        if(payload_len < sizeof(artnet_header)) return;

        uint16_t universe = header->universe;
        uint16_t dmx_len = reverse_16bit_endianness(header->lenght);

        uint8_t sequence = header->sequence;

        // TODO Sequence

        // DMX-Daten lesen
        payload     += sizeof(artnet_header);
        payload_len -= sizeof(artnet_header);

        if(dmx_len > payload_len || dmx_len > 512 || dmx_len == 0) return;

        // lineare Suche nach Output für Universe
        for(uint i = 0; i < UNIVERSE_COUNT; i++) {
            if(universes[i].universe == universe) {

                if(!dmx_busy(&(universes[i].output))) {
                    // bei partieller Übertragung kleine Pakete aufpolstern
                    if(dmx_len < DMX_MIN_PACKET_LENGTH) {
                        memset(payload + dmx_len, 0, DMX_MIN_PACKET_LENGTH - dmx_len);
                        dmx_len = DMX_MIN_PACKET_LENGTH;
                    }

                    // Nutzdaten kopieren
                    memcpy(universes[i].data + 1, payload, dmx_len);

                    // Startbyte
                    universes[i].data[0] = 0;

                    // raussenden
                    dmx_write(&(universes[i].output), universes[i].data, dmx_len);
                }
                break;
            }
        }
    }
}

static void udp_packet_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if(p != NULL) {

        if(p->tot_len > sizeof(rx_buf)) goto free_buf;

        uint offset = 0;

        // Linked List in geschlossenen Speicherbereich kopieren
        for (struct pbuf *q = p; q != NULL; q = q->next) {
                
            memcpy(rx_buf + offset, q->payload, q->len);
            offset += q->len;

            // Nur 1 Teil
            if (q->len == q->tot_len) {
                break;
            }
        }

        artnet_handle_packet(rx_buf, offset);
        
        free_buf:
        pbuf_free(p);
    }
}

void artnet_init() {

    // Universen laden
    for(uint i = 0; i < UNIVERSE_COUNT; i++) {
        uint universe = i + START_UNIVERSE;
        uint pin = i + DMX_START_PIN;

        universes[i].universe = universe;
        
        int err = dmx_begin(&(universes[i].output), pin);

        if(err) {
            while(true) {
                printf("Error initializing DMX Output %u: %d\n", i, err);
                sleep_ms(1000);
            }
        }
    }

    // UDP init
    udp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (udp_pcb != NULL) {
        err_t err;

        err = udp_bind(udp_pcb, IP_ANY_TYPE, 6454);

        if (err == ERR_OK) {
            udp_recv(udp_pcb, udp_packet_recv, NULL);
        } else {
            printf("Error on bind.\n");
            while(1);
        }
    } else {
        printf("Error on init.\n");
        while(1);
    }
}