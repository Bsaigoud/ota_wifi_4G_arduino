/**
 * @file evse_ota.cpp
 * @author S GOPIKRISHNA
 * @brief 
 * @version 0.1
 * @date 2024-10-09
 * 
 * @copyright Copyright (c) 2024
 * @patanet EVRE
 */

#include "evse_ota.h"

#include <Arduino.h>
#include <NetworkClient.h>
#include <HTTPClient.h>
#include <Update.h>
#include "Preferences.h"

#if WIFI_OTA_ENABLE
#include <WiFi.h>
const char* WIFI_SSID = "EVRE HQ";
const char* WIFI_PASSWORD = "Amplify@5";
#endif

#if GSM_OTA_ENABLE
#include <PPP.h>

#define PPP_MODEM_APN "m2misafe"
#define PPP_MODEM_PIN "0000"  // or NULL

// WaveShare SIM7600 HW Flow Control
#define PPP_MODEM_RST     -1
#define PPP_MODEM_RST_LOW false  //active HIGH
#define PPP_MODEM_TX      17
#define PPP_MODEM_RX      16
#define PPP_MODEM_RTS     -1
#define PPP_MODEM_CTS     -1
#define PPP_MODEM_FC      ESP_MODEM_FLOW_CONTROL_NONE
#define PPP_MODEM_MODEL   PPP_MODEM_SIM7600

// SIM800 basic module with just TX,RX and RST
// #define PPP_MODEM_RST     0
// #define PPP_MODEM_RST_LOW true //active LOW
// #define PPP_MODEM_TX      2
// #define PPP_MODEM_RX      19
// #define PPP_MODEM_RTS     -1
// #define PPP_MODEM_CTS     -1
// #define PPP_MODEM_FC      ESP_MODEM_FLOW_CONTROL_NONE
// #define PPP_MODEM_MODEL   PPP_MODEM_SIM800
#endif

//==========================================================================

const uint8_t OTA_KEY[32] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20,
                             0x61, 0x20, 0x73, 0x69, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x73, 0x74, 0x20, 0x6b, 0x65, 0x79 };

/*
const uint8_t  OTA_KEY[32] = {'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
                              '8',  '9',  ' ',  't',  'h',  'i',  's',  ' ',
                              'a',  ' ',  's',  'i',  'm',  'p',  'l',  'e',
                              't',  'e',  's',  't',  ' ',  'k',  'e',  'y' };
*/

//const uint8_t  OTA_KEY[33] = "0123456789 this a simpletest key";

const uint32_t OTA_ADDRESS = 0x4320;
const uint32_t OTA_CFG = 0x0f;
const uint32_t OTA_MODE = U_AES_DECRYPT_AUTO;

const char* HTTPUPDATE_USERAGRENT = "ESP32-Updater";
//const char*    HTTPUPDATE_HOST         = "www.yourdomain.com";
const char* HTTPUPDATE_HOST = "34.100.138.28";
const uint16_t HTTPUPDATE_PORT = 80;
const char* HTTPUPDATE_UPDATER_URI = "/firmware/updater.php";                          //uri to 'updater.php'
// const char *HTTPUPDATE_DIRECT_URI = "/firmware/gopi.bin";  //uri to image file
const char* HTTPUPDATE_DIRECT_URI = "/firmware/evse-adani-7-4kw-test-034.bin";  //uri to image file

const char* HTTPUPDATE_USER = NULL;  //use NULL if no authentication needed
//const char*    HTTPUPDATE_USER       = "user";
const char* HTTPUPDATE_PASSWORD = "password";

const char* HTTPUPDATE_BRAND = "EVRE";                         /* Brand ID */
// const char *HTTPUPDATE_MODEL = "HTTP_Client_AES_OTA_Update"; /* Project name */
const char* HTTPUPDATE_MODEL = "evse-adani-7-4kw"; /* Project name */
const char* HTTPUPDATE_FIRMWARE = "0.8";                     /* Firmware version */

String response = "";
uint8_t evse_ota_enable = 0;
Preferences evse_ota_pref;
//==========================================================================


String urlEncode(const String& url, const char* safeChars = "-_.~") {

  String encoded = "";
  char temp[4];

  for (int i = 0; i < url.length(); i++) {
    temp[0] = url.charAt(i);
    if (temp[0] == 32) {  //space
      encoded.concat('+');
    }
    else if ((temp[0] >= 48 && temp[0] <= 57)        /*0-9*/
      || (temp[0] >= 65 && temp[0] <= 90)     /*A-Z*/
      || (temp[0] >= 97 && temp[0] <= 122)    /*a-z*/
      || (strchr(safeChars, temp[0]) != NULL) /* "=&-_.~" */
      ) {
      encoded.concat(temp[0]);
    }
    else {  //character needs encoding
      snprintf(temp, 4, "%%%02X", temp[0]);
      encoded.concat(temp);
    }
  }
  return encoded;
}

//==========================================================================
bool addQuery(String* query, const String name, const String value) {
  if (name.length() && value.length()) {
    if (query->length() < 3) {
      *query = "?";
    }
    else {
      query->concat('&');
    }
    query->concat(urlEncode(name));
    query->concat('=');
    query->concat(urlEncode(value));
    return true;
  }
  return false;
}

//==========================================================================
//==========================================================================
void printProgress(size_t progress, const size_t& size) {
  static int last_progress = -1;
  if (size > 0) {
    progress = (progress * 100) / size;
    progress = (progress > 100 ? 100 : progress);  //0-100
    if (progress != last_progress) {
      Serial.printf("Progress: %d%%\n", progress);
      last_progress = progress;
    }
  }
}

//==========================================================================
bool http_downloadUpdate(HTTPClient& http, uint32_t size = 0) {
  size = (size == 0 ? http.getSize() : size);
  if (size == 0) {
    return false;
  }
  NetworkClient* client = http.getStreamPtr();

  if (!Update.begin(size, U_FLASH)) {
    Serial.printf("Update.begin failed! (%s)\n", Update.errorString());
    return false;
  }

  if (!Update.setupCrypt(OTA_KEY, OTA_ADDRESS, OTA_CFG, OTA_MODE)) {
    Serial.println("Update.setupCrypt failed!");
  }

  if (Update.writeStream(*client) != size) {
    Serial.printf("Update.writeStream failed! (%s)\n", Update.errorString());
    return false;
  }

  if (!Update.end()) {
    Serial.printf("Update.end failed! (%s)\n", Update.errorString());
    return false;
  }
  return true;
}

//==========================================================================
int http_sendRequest(HTTPClient& http) {

  //set request Headers to be sent to server
  http.useHTTP10(true);  // use HTTP/1.0 for update since the update handler not support any transfer Encoding
  http.setTimeout(8000);
  http.addHeader("Cache-Control", "no-cache");

  //set own name for HTTPclient user-agent
  http.setUserAgent(HTTPUPDATE_USERAGRENT);

  int code = http.GET();  //send the GET request to HTTP server
  int len = http.getSize();

  if (code == HTTP_CODE_OK) {
    return (len > 0 ? len : 0);  //return 0 or length of image to download
  }
  else if (code < 0) {
    Serial.printf("Error: %s\n", http.errorToString(code).c_str());
    return code;  //error code should be minus between -1 to -11
  }
  else {
    Serial.printf("Error: HTTP Server response code %i\n", code);
    return -code;  //return code should be minus between -100 to -511
  }
}

//==========================================================================
/* http_updater sends a GET request to 'update.php' on web server */
bool http_updater(const String& host, const uint16_t& port, String uri, const bool& download, const char* user = NULL, const char* password = NULL) {
  //add GET query params to be sent to server (are used by server 'updater.php' code to determine what action to take)
  String query = "";
  addQuery(&query, "cmd", (download ? "download" : "check"));  //action command

  //setup HTTPclient to be ready to connect & send a request to HTTP server
  HTTPClient http;
  NetworkClient client;
  uri.concat(query);  //GET query added to end of uri path
  if (!http.begin(client, host, port, uri)) {
    return false;  //httpclient setup error
  }
  Serial.printf("Sending HTTP request 'http://%s:%i%s'\n", host.c_str(), port, uri.c_str());

  //set basic authorization, if needed for webpage access
  if (user != NULL && password != NULL) {
    http.setAuthorization(user, password);  //set basic Authorization to server, if needed be gain access
  }

  //add unique Headers to be sent to server used by server 'update.php' code to determine there a suitable firmware update image available
  http.addHeader("Brand-Code", HTTPUPDATE_BRAND);
  http.addHeader("Model", HTTPUPDATE_MODEL);
  http.addHeader("Firmware", HTTPUPDATE_FIRMWARE);

  //set headers to look for to get returned values in servers http response to our http request
  const char* headerkeys[] = { "update", "version" };  //server returns update 0=no update found, 1=update found, version=version of update found
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  http.collectHeaders(headerkeys, headerkeyssize);

  //connect & send HTTP request to server
  int size = http_sendRequest(http);

  //is there an image to download
  if (size > 0 || (!download && size == 0)) {
    if (!http.header("update") || http.header("update").toInt() == 0)
    {
      Serial.println("No Firmware available");
      if (!http.header("version") || http.header("version").toFloat() <= String(HTTPUPDATE_FIRMWARE).toFloat())
      {
        Serial.println("Firmware is upto Date");
        gsm_restart();
        evse_ota_pref.begin("evse_ota", false);
        evse_ota_enable = evse_ota_pref.putUChar("evse_ota_en", 0);
        evse_ota_pref.end();
        ESP.restart();
      }
    }
    else if (!http.header("version") || http.header("version").toFloat() <= String(HTTPUPDATE_FIRMWARE).toFloat()) {
      Serial.println("Firmware is upto Date");
      gsm_restart();
      evse_ota_pref.begin("evse_ota", false);
      evse_ota_enable = evse_ota_pref.putUChar("evse_ota_en", 0);
      evse_ota_pref.end();
      ESP.restart(); 
    }
    else {
      //image avaliabe to download & update
      if (!download) {
        Serial.printf("Found V%s Firmware\n", http.header("version").c_str());
      }
      else {
        Serial.printf("Downloading & Installing V%s Firmware\n", http.header("version").c_str());
      }
      if (!download || http_downloadUpdate(http)) {
        http.end();  //end connection
        return true;
      }
    }
  }

  http.end();  //end connection
  return false;
}

//==========================================================================
/* this downloads Firmware image file directly from web server */
bool http_direct(const String& host, const uint16_t& port, const String& uri, const char* user = NULL, const char* password = NULL) {
  //setup HTTPclient to be ready to connect & send a request to HTTP server
  HTTPClient http;
  NetworkClient client;
  if (!http.begin(client, host, port, uri)) {
    return false;  //httpclient setup error
  }
  Serial.printf("Sending HTTP request 'http://%s:%i%s'\n", host.c_str(), port, uri.c_str());

  //set basic authorization, if needed for webpage access
  if (user != NULL && password != NULL) {
    http.setAuthorization(user, password);  //set basic Authorization to server, if needed be gain access
  }

  //connect & send HTTP request to server
  int size = http_sendRequest(http);

  //is there an image to download
  if (size > 0) {
    if (http_downloadUpdate(http)) {
      http.end();
      return true;  //end connection
    }
  }
  else {
    Serial.println("Image File not found");
  }

  http.end();  //end connection
  return false;
}

#if GSM_OTA_ENABLE
void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
  case ARDUINO_EVENT_PPP_START:        Serial.println("PPP Started"); break;
  case ARDUINO_EVENT_PPP_CONNECTED:    Serial.println("PPP Connected"); break;
  case ARDUINO_EVENT_PPP_GOT_IP:       Serial.println("PPP Got IP"); break;
  case ARDUINO_EVENT_PPP_LOST_IP:      Serial.println("PPP Lost IP"); break;
  case ARDUINO_EVENT_PPP_DISCONNECTED: Serial.println("PPP Disconnected"); break;
  case ARDUINO_EVENT_PPP_STOP:         Serial.println("PPP Stopped"); break;
  default:                             break;
  }
}

void testClient(const char* host, uint16_t port) {
  NetworkClient client;
  if (!client.connect(host, port)) {
    Serial.println("Connection Failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    client.read();  //Serial.write(client.read());
  }

  Serial.println("Connection Success");
  client.stop();
}
#endif

#if WIFI_OTA_ENABLE
void wifi_ota_init(void)
{
  Serial.println();
  Serial.printf("Booting %s V%s\n", HTTPUPDATE_MODEL, HTTPUPDATE_FIRMWARE);

  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failed, retrying.");
  }
  int i = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    if ((++i % 100) == 0) {
      Serial.println();
    }
    delay(100);
  }
  Serial.printf("Connected to Wifi\nLocal IP: %s\n", WiFi.localIP().toString().c_str());

  Update.onProgress(printProgress);

  Serial.println("Checking with Server, if New Firmware available");
  if (http_updater(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_UPDATER_URI, 0, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD)) {    //check for new firmware
    if (http_updater(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_UPDATER_URI, 1, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD)) {  //update to new firmware
      Serial.println("Firmware Update Successful, rebooting");
      ESP.restart();
    }
  }

  Serial.println("Checking Server for Firmware Image File to Download & Install");
  if (http_direct(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_DIRECT_URI, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD)) {
    Serial.println("Firmware Update Successful, rebooting");
    ESP.restart();
  }
}
#endif

#if GSM_OTA_ENABLE

void init_gsm_event(void)
{
  // Listen for modem events
  Network.onEvent(onEvent);
  Update.onProgress(printProgress);
}

void gsm_restart(void)
{
  response = PPP.cmd("AT", 10000);
  Serial.print("Response: ");
  Serial.println(response);
  if (response == ESP_OK)
  {
    Serial.println("Modem communication OK");
  }
  else
  {
    Serial.println("Failed AT Command");
    delay(1000);
  }

  response = PPP.cmd("AT+CFUN=1,1", 10000);
  Serial.print("Response: ");
  Serial.println(response);
  if (response == ESP_OK)
  {
    Serial.println("Modem communication OK");
    delay(1000);  // Wait 10 seconds for the modem to reset
  }
  else
  {
    Serial.println("Failed Modem Restart Command");
  }

  delay(5000);
}

void gsm_ota_init(void)
{
  Serial.println("Initializing modem...");
  // Configure the modem
  PPP.setApn(PPP_MODEM_APN);
  // PPP.setPin(PPP_MODEM_PIN);
  // PPP.setResetPin(PPP_MODEM_RST, PPP_MODEM_RST_LOW);
  PPP.setPins(PPP_MODEM_TX, PPP_MODEM_RX, PPP_MODEM_RTS, PPP_MODEM_CTS, PPP_MODEM_FC);
  delay(1000);

  int k = 0;
  while (!PPP.begin(PPP_MODEM_MODEL) && ((++k) < 10));

  gsm_restart();

  Serial.println("Starting the modem. It might take a while!");

  while (!PPP.begin(PPP_MODEM_MODEL, UART_NUM_1, 115200) && ((++k) < 250));

  Serial.print("Manufacturer: ");

  Serial.println(PPP.cmd("AT+CGMI", 10000));
  Serial.print("Model: ");
  Serial.println(PPP.moduleName());
  Serial.print("IMEI: ");
  Serial.println(PPP.IMEI());

  bool attached = PPP.attached();
  if (!attached) {
    int i = 0;
    unsigned int s = millis();
    Serial.print("Waiting to connect to network");
    while (!attached && ((++i) < 600))
    {
      Serial.print(".");
      delay(100);
      // PPP.mode(ESP_MODEM_MODE_COMMAND);
      attached = PPP.attached();
    }
    Serial.print((millis() - s) / 1000.0, 1);
    Serial.println("s");
    attached = PPP.attached();
  }

  Serial.print("Attached: ");
  Serial.println(attached);
  Serial.print("State: ");
  Serial.println(PPP.radioState());
  if (attached) {
    Serial.print("Operator: ");
    Serial.println(PPP.operatorName());
    Serial.print("IMSI: ");
    Serial.println(PPP.IMSI());
    Serial.print("RSSI: ");
    Serial.println(PPP.RSSI());
    int ber = PPP.BER();
    if (ber > 0) {
      Serial.print("BER: ");
      Serial.println(ber);
      Serial.print("NetMode: ");
      Serial.println(PPP.networkMode());
    }

    Serial.println("Switching to data mode...");
    PPP.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode
    if (!PPP.waitStatusBits(ESP_NETIF_CONNECTED_BIT, 1000)) {
      Serial.println("Failed to connect to internet!");
    }
    else {
      Serial.println("Connected to internet!");
    }
  }
  else {
    Serial.println("Failed to connect to network!");
  }

return;
}

void gsm_ota_loop(void)
{
  if (PPP.connected())
  {
    // testClient("google.com", 80);
#if 0
    Serial.println("Checking Server for Firmware Image File to Download & Install");
    if (http_direct(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_DIRECT_URI, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD))
    {
      Serial.println("Firmware Update Successful, rebooting");
      ESP.restart();
    }
#else 
    Serial.println("Checking with Server, if New Firmware available");
    if (http_updater(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_UPDATER_URI, 0, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD))
    { // check for new firmware
      if (http_updater(HTTPUPDATE_HOST, HTTPUPDATE_PORT, HTTPUPDATE_UPDATER_URI, 1, HTTPUPDATE_USER, HTTPUPDATE_PASSWORD))
      { // update to new firmware
        gsm_restart();
        evse_ota_pref.begin("evse_ota", false);
        evse_ota_enable = evse_ota_pref.putUChar("evse_ota_en", 0);
        evse_ota_pref.end();
        Serial.println("Firmware Update Successful, rebooting");
        ESP.restart();
      }
    }
#endif
  }
}
#endif
