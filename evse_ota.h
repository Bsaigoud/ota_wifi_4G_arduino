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

#define WIFI_OTA_ENABLE     (0)
#define GSM_OTA_ENABLE      (1)


#if WIFI_OTA_ENABLE
void wifi_ota_init(void);
#endif

#if GSM_OTA_ENABLE
void gsm_restart(void);
void init_gsm_event(void);
void gsm_ota_init(void);
void gsm_ota_loop(void);
#endif

extern const char* HTTPUPDATE_FIRMWARE;                     /* Firmware version */