
// taken from https://github.com/jostlowe/Pico-DMX/blob/main/src/DmxOutput.h
// Copyright (c) 2021 Jostein Løwer, SPDX-License-Identifier: BSD-3-Clause

#ifndef DMX_OUTPUT_H
#define DMX_OUTPUT_H

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#define DMX_UNIVERSE_SIZE 512
#define DMX_SM_FREQ 1000000
#define DMX_MIN_PACKET_LENGTH 32

#define DMX_PIO pio1

typedef struct {
    uint _pin;
    uint _sm;
    uint _dma;
} dmx_output_t;


enum {
    DMX_SUCCESS = 0,

    // There were no available state machines left in the
    // pio instance.
    DMX_ERR_NO_SM_AVAILABLE = -1,

    // There is not enough program memory left in the PIO to fit
    // The DMX PIO program
    DMX_ERR_INSUFFICIENT_PRGM_MEM = -2,

    // There are no available DMA channels to handle
    // the transfer of DMX data to the PIO
    DMX_ERR_NO_DMA_AVAILABLE = -3
};

// Returns enum
int dmx_begin(dmx_output_t * dmx, uint pin);

void dmx_write(dmx_output_t * dmx, uint8_t *universe, uint length);

bool dmx_busy(dmx_output_t * dmx);

// void await(dmx_output_t * dmx);

void dmx_end(dmx_output_t * dmx);

#endif