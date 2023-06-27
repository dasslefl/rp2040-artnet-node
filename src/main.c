
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "eth_lwip.h"
#include "eth.h"

#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/udp.h"

#include "hd44780_spi.h"
#include "dmx_output.h"
#include "artnet.h"

static uint lcd_update_timer = 0;

void netif_link_callback(struct netif *netif) {
    printf("netif link status changed %s\n", netif_is_link_up(netif) ? "up" : "down");
}

void netif_status_callback(struct netif *netif) {
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

int main() {
    struct netif netif;
    // Also runs stdio init
    eth_lwip_init(&netif);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    lcd_init(16, 2, LCD_5x8DOTS);

    lcd_puts("PicoNode");
    lcd_set_cursor(0, 1);
    lcd_puts(__DATE__);

    printf("PicoNode\n");

    sleep_ms(2000);

    lcd_clear();

    // assign callbacks for link and status
    netif_set_link_callback(&netif, netif_link_callback);
    netif_set_status_callback(&netif, netif_status_callback);

    // set the default interface and bring it up
    netif_set_default(&netif);
    netif_set_up(&netif);

    // Start DHCP client
    dhcp_start(&netif);

    artnet_init();
    
    while (true) {
        eth_lwip_poll();

        if(eth_every_ms(&lcd_update_timer, 500)) {
            lcd_set_cursor(0, 0);
            lcd_puts(netif_is_link_up(&netif) ? "Link: UP  " : "Link: DOWN");
            lcd_set_cursor(0, 1);
            lcd_puts(netif_is_link_up(&netif) ? ip4addr_ntoa(netif_ip4_addr(&netif)) : "                ");
        }
    }
}

