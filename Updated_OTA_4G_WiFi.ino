#include "evse_ota.h"

void setup() {
  Serial.begin(115200);
  while(!Serial); // wait for Serial to be ready

#if WIFI_OTA_ENABLE
  Serial.println("[setup] WiFi OTA Checking, Current Firmware Version: " + String(HTTPUPDATE_FIRMWARE));
#endif

#if GSM_OTA_ENABLE
  Serial.println("[setup] GSM OTA Checking, Current Firmware Version: " + String(HTTPUPDATE_FIRMWARE));
#endif

#if WIFI_OTA_ENABLE
  wifi_ota_init();
#endif

#if GSM_OTA_ENABLE
  init_gsm_event();
  gsm_ota_init();
#endif
}

void loop() {


// Serial.println("Free Mem: " + String(ESP.getFreeHeap()));

#if GSM_OTA_ENABLE
  gsm_ota_loop();
#endif

  delay(10000);
}

