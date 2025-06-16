#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#define WIFI_SSID "twinsuns27"      
#define WIFI_PASSWORD "alphacentauri"      

#define SERVER_IP "192.168.201.100"    
#define SERVER_PORT 41234   

#define ADC_PIN_X 26 
#define ADC_PIN_Y 27 

const char* get_direction(uint16_t x, uint16_t y);
void send_udp(struct udp_pcb *pcb, const char *msg);

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Starting...\n");

    adc_init();
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Wi-Fi connection failed\n");
        return -1;
    }

    printf("Wi-Fi connected\n");

    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf("Error creating UDP Protocol Control Block\n");
        return -1;
    }

    while (true) {
        // X axis
        adc_select_input(0);
        uint16_t x = adc_read();

        // Y axis
        adc_select_input(1);
        uint16_t y = adc_read();

        const char *direction = get_direction(x, y);

        char msg[100];
        snprintf(msg, sizeof(msg), "| X:%d | Y:%d | direction:%s |", x, y, direction);
        printf("Sending: %s\n", msg);

        send_udp(pcb, msg);

        sleep_ms(500);
    }

    cyw43_arch_deinit();
    return 0;
}


const char* get_direction(uint16_t x, uint16_t y) {
    const uint16_t centro = 2048;
    const uint16_t deadZone = 500;

    int dx = x - centro;
    int dy = y - centro;

    if (abs(dx) < deadZone && abs(dy) < deadZone) {
        return "center";
    }

    if (dy > deadZone) {
        if (dx > deadZone) {
            return "northeast";
        }
        else if (dx < -deadZone) {
            return "northwest";
        }
        else {
            return "north";
        }
    } else if (dy < -deadZone) {
        if (dx > deadZone) {
            return "southeast";
        }
        else if (dx < -deadZone) {
            return "southwest";
        }
        else {
            return "south";
        }
    } else {
        if (dx > deadZone) {
            return "east";
        }
        else if (dx < -deadZone) {
            return "west";
        }
    }

    return "center";
}


void send_udp(struct udp_pcb *pcb, const char *msg) {
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
    if (!p) return;
    memcpy(p->payload, msg, strlen(msg));

    ip_addr_t dest_ip;
    ipaddr_aton(SERVER_IP, &dest_ip);

    udp_sendto(pcb, p, &dest_ip, SERVER_PORT);
    pbuf_free(p);
}
