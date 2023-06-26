cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(rp2040_artnet_node C CXX ASM)

pico_sdk_init()

# RMII Library
add_subdirectory("lib/rp2040-rmii")

# rest of your project
add_executable(rp2040_artnet_node
    src/main.c
    src/hd44780_spi.c
)

# pico_generate_pio_header(rp2040_artnet_node ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_link_libraries(rp2040_artnet_node pico_stdlib rp2040_rmii hardware_pio hardware_dma hardware_irq hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(rp2040_artnet_node 1)
pico_enable_stdio_uart(rp2040_artnet_node 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(rp2040_artnet_node)
