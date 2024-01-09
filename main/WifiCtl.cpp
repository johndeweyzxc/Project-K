/**
 * @file wifi_ctl.cpp
 * @author johndeweyzxc (johndewey02003@gmail.com)
 * @brief Implements the functionality for controlling the Wi-Fi in ESP32
 */

#include "WifiCtl.h"

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "lwip/ip_addr.h"

static esp_netif_t* netif;

static void wifiCtlEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t* event =
        (wifi_event_ap_staconnected_t*)event_data;
    uint8_t* b = event->mac;
    printf("{WIFI,CONNECT,%02X:%02X:%02X:%02X:%02X:%02X,}\n", b[0], b[1], b[2],
           b[3], b[4], b[5]);
  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t* event =
        (wifi_event_ap_stadisconnected_t*)event_data;
    uint8_t* b = event->mac;
    printf("{WIFI,DISCONNECT,%02X:%02X:%02X:%02X:%02X:%02X,}\n", b[0], b[1],
           b[2], b[3], b[4], b[5]);
  }
}

void wifiCtlApStop() {
  ESP_ERROR_CHECK(esp_wifi_stop());
  printf("{WIFI,STOPPED,}\n");
}

extern "C" void wifiCtlApStart() {
  ESP_ERROR_CHECK(esp_netif_dhcps_stop(netif));
  esp_netif_ip_info_t ip_info;
  IP4_ADDR(&ip_info.ip, 172, 217, 28, 1);
  IP4_ADDR(&ip_info.gw, 172, 217, 28, 1);
  IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
  ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));
  esp_netif_dhcps_start(netif);

  ESP_ERROR_CHECK(esp_wifi_start());
  printf("{WIFI,STARTED,}\n");
}

extern "C" void wifiCtlCreateAp() {
  wifi_config_t wifi_config = {
      .ap = {.ssid = "PLDTHOMEFIBRb1ab23",
             .password = "12345678",
             .ssid_len = strlen("PLDTHOMEFIBRb1ab23"),
             .channel = 1,
             .authmode = WIFI_AUTH_WPA_WPA2_PSK,
             .max_connection = 4},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
}

extern "C" void wifiCtlInit() {
  ESP_ERROR_CHECK(esp_netif_init());
  netif = esp_netif_create_default_wifi_ap();
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiCtlEventHandler, NULL, NULL));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}