#include <stdio.h>

#include "CmdParser.h"
#include "Exploits.h"
#include "WebServer.h"
#include "WifiCtl.h"
#include "esp_err.h"
#include "esp_event.h"

extern "C" void app_main(void) {
  printf("{ESP_STARTED,}\n");
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  cmdParserCreateTask();
  wifiCtlInit();
  wifiCtlCreateAp();
  wifiCtlApStart();
  webServerStart();
  exploitsCreateDnsHijackTask();
}