#ifndef __MAIN_H
#define __MAIN_H

#include <Arduino.h>
#include "define.h"
#include "constants.h"
#include <ESP32Time.h>
#include <Arduino_JSON.h>


extern const char *soft_ap_ssid;
extern const char *soft_ap_password;
extern AsyncWebServer server;
extern AsyncWebSocket ws;

extern VSTUP_t DIN[pocetDIN_celkovo];
extern char NazovSiete[30];
extern char Heslo[30];
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern IPAddress primaryDNS;
extern IPAddress secondaryDNS;
extern ESP32Time rtc;
extern JSONVar myObject, myObject2, ObjDatumCas, ObjTopeni, JSON_DebugMsg;

extern bool Internet_CasDostupny;
extern bool RTC_cas_OK;
extern char TX_BUF[];
extern struct tm MyRTC_cas;
extern TERMOSTAT_t room[];
extern u8 RS485_toRx_timeout;
extern u16 cas_rebrikuSet;
extern unsigned long klimaJednotkaEnable;

void Loop_1ms(void);
void Loop_10ms(void);
void Loop_100ms(void);
void Loop_1sek(void);
void Loop_10sek(void);
void FuncServer_On(void);

void ESPinfo(void);
void DebugMsgToWebSocket(String textik);
void initWebSocket();
String handle_GetNastaveneHodnoty();
String handle_SetNastaveneHodnoty(AsyncWebServerRequest *request);

#endif