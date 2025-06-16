#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#define WIFI_SSID "twinsuns27"      
#define WIFI_PASSWORD "alphacentauri"      

#define SERVER_IP "192.168.201.100"    
#define SERVER_PORT 41234            

#define BUTTON_PIN 5                 
#define ADC_TEMP 4                   

void show_ip();                                      
const char* read_button();                                
float read_temp();                                 
void send_udp(struct udp_pcb *pcb, const char *msg);  

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Starting...\n");

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    if (cyw43_arch_init()) {
        printf("Error initializing Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Wi-Fi connection failed\n");
        return -1;
    }

    printf("Wi-Fi connected\n");

    show_ip();

    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf("Error creating UDP Protocol Control Block\n");
        return -1;
        
    }

    while (true) {
        const char* button_state = read_button();
        float temperature = read_temp();

        char msg[100];
        snprintf(msg, sizeof(msg), "| Button: %s | Temperature: %.2f Celsius |", button_state, temperature);
        printf("Sending: %s\n", msg);

        send_udp(pcb, msg);

        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}

void show_ip() {
    struct netif *netif = netif_default;
    if (netif) {
        printf("RP204 IP Address: %s\n", ipaddr_ntoa(&netif->ip_addr));
    } else {
        printf("Error getting IP\n");
    }
}

const char* read_button() {
    return gpio_get(BUTTON_PIN) ? "unpressed" : "pressed";
}

float read_temp() {
    adc_select_input(ADC_TEMP);
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / (1 << 12);
    return 27.0 - (voltage - 0.706) / 0.001721;
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