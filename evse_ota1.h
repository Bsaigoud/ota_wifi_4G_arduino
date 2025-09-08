/**
 * @file evse_ota.h
 * @author S GOPIKRISHNA
 * @brief
 * @version 0.1
 * @date 2024-10-09
 *
 * @copyright Copyright (c) 2024
 * @patanet EVRE
 **/

#pragma once

#define WIFI_OTA_ENABLE_ESP_V_3_0_SUPP (0)
#define GSM_OTA_ENABLE_ESP_V_3_0_SUPP (0)
#define TINY_GSM_OTA_ENABLE (1)
#define WIFI_OTA_ENABLE (1)
#define ESP_V_3_0_SUPP (0)
#define WDT_V5B_ENABLE (1)
#if WDT_V5B_ENABLE
#include <esp_task_wdt.h>
// 2min WDT
#define WDT_TIMEOUT 180
#define WDT_PIN (2)

void init_wdt_v5b(void);
#endif

#if WIFI_OTA_ENABLE_ESP_V_3_0_SUPP
void wifi_ota_init(void);
#endif

#if GSM_OTA_ENABLE_ESP_V_3_0_SUPP
void gsm_restart(void);
void init_gsm_event(void);
void gsm_ota_init(void);
void gsm_ota_loop(void);
#endif

#if TINY_GSM_OTA_ENABLE
// bool tgsm_http_downloadUpdate(HTTPClient_gsm& http, uint32_t size = 0);
// int tgsm_http_sendRequest(HTTPClient_gsm& http);
// bool tgsm_http_downloadUpdate(HTTPClient& http, uint32_t size = 0);
// bool tgsm_http_updater(const String& host, const uint16_t& port, String uri, const bool& download, const char* user = NULL, const char* password = NULL);
// bool tgsm_http_direct(const String& host, const uint16_t& port, const String& uri, const char* user = NULL, const char* password = NULL);
void tiny_gsm_setup(void);
void esp_4g_connection_check(void);
void tgsm_ota_direct_dw(void);

extern bool modemConnected;

#endif

#if WIFI_OTA_ENABLE
void init_wifi_ota(void);
void wifi_ota_direct_dwn(void);
#endif

extern const char *HTTPUPDATE_FIRMWARE; /* Firmware version */