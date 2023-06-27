
// taken from https://github.com/jostlowe/Pico-DMX/blob/main/src/DmxOutput.cpp
// Copyright (c) 2021 Jostein Løwer, SPDX-License-Identifier: BSD-3-Clause

#include "dmx_output.h"
#include "dmx_output.pio.h"

#include "hardware/clocks.h"
#include "hardware/irq.h"

// Programm muss nur 1x geladen werden
static int prgm_offset = -1;

int dmx_begin(dmx_output_t * dmx, uint pin) {
    /* 
    Attempt to load the DMX PIO assembly program 
    into the PIO program memory if necessary
    */
    if(prgm_offset == -1) {
        if (!pio_can_add_program(DMX_PIO, &dmx_output_program)) 
            return DMX_ERR_INSUFFICIENT_PRGM_MEM;
        
        prgm_offset = pio_add_program(DMX_PIO, &dmx_output_program);
    }

    /* 
    Attempt to claim an unused State Machine 
    into the PIO program memory
    */

    int sm = pio_claim_unused_sm(DMX_PIO, false);
    if (sm == -1)
        return DMX_ERR_NO_SM_AVAILABLE;
    

    // Set this pin's GPIO function (connect PIO to the pad)
    pio_sm_set_pins_with_mask(DMX_PIO, sm, 1u << pin, 1u << pin);
    pio_sm_set_pindirs_with_mask(DMX_PIO, sm, 1u << pin, 1u << pin);
    pio_gpio_init(DMX_PIO, pin);

    // Generate the default PIO state machine config provided by pioasm
    pio_sm_config sm_conf = dmx_output_program_get_default_config(prgm_offset);

    // Setup the side-set pins for the PIO state machine
    sm_config_set_out_pins(&sm_conf, pin, 1);
    sm_config_set_sideset_pins(&sm_conf, pin);

    // Setup the clock divider to run the state machine at exactly 1MHz
    uint clk_div = clock_get_hz(clk_sys) / DMX_SM_FREQ;
    sm_config_set_clkdiv(&sm_conf, clk_div);

    // Load our configuration, jump to the start of the program and run the State Machine
    pio_sm_init(DMX_PIO, sm, prgm_offset, &sm_conf);
    pio_sm_set_enabled(DMX_PIO, sm, true);

    // Claim an unused DMA channel.
    // The channel is kept througout the lifetime of the DMX source
    int dma = dma_claim_unused_channel(false);

    if (dma == -1)
        return DMX_ERR_NO_DMA_AVAILABLE;

    // Get the default DMA config for our claimed channel
    dma_channel_config dma_conf = dma_channel_get_default_config(dma);

    // Set the DMA to move one byte per DREQ signal
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_8);

    // Setup the DREQ so that the DMA only moves data when there
    // is available room in the TXF buffer of our PIO state machine
    channel_config_set_dreq(&dma_conf, pio_get_dreq(DMX_PIO, sm, true));

    // Setup the DMA to write to the TXF buffer of the PIO state machine
    dma_channel_set_write_addr(dma, &DMX_PIO->txf[sm], false);

    // Apply the config
    dma_channel_set_config(dma, &dma_conf, false);

    // Set member values of C++ class
    dmx->_sm = sm;
    dmx->_pin = pin;
    dmx->_dma = dma;

    return DMX_SUCCESS;
}

void dmx_write(dmx_output_t * dmx, uint8_t *universe, uint length) {

    // Temporarily disable the PIO state machine
    pio_sm_set_enabled(DMX_PIO, dmx->_sm, false);

    // Reset the PIO state machine to a consistent state. Clear the buffers and registers
    pio_sm_restart(DMX_PIO, dmx->_sm);

    // Start the DMX PIO program from the beginning
    pio_sm_exec(DMX_PIO, dmx->_sm, pio_encode_jmp(prgm_offset));

    // Restart the PIO state machinge
    pio_sm_set_enabled(DMX_PIO, dmx->_sm, true);

    // Start the DMA transfer
    dma_channel_transfer_from_buffer_now(dmx->_dma, universe, length);
}

bool dmx_busy(dmx_output_t * dmx) {
    if (dma_channel_is_busy(dmx->_dma))
        return true;

    return !pio_sm_is_tx_fifo_empty(DMX_PIO, dmx->_sm);
}

/*
void Dmx::await()
{
    dma_channel_wait_for_finish_blocking(_dma);

    while (!pio_sm_is_tx_fifo_empty(_pio, _sm))
    {
    }
}
*/

void dmx_end(dmx_output_t * dmx) {
    // Stop the PIO state machine
    pio_sm_set_enabled(DMX_PIO, dmx->_sm, false);

    // Remove the PIO DMX program from the PIO program memory
    pio_remove_program(DMX_PIO, &dmx_output_program, prgm_offset);

    // Unclaim the DMA channel
    dma_channel_unclaim(dmx->_dma);

    // Unclaim the sm
    pio_sm_unclaim(DMX_PIO, dmx->_sm);
}