#ifndef __HelpFunction_H
#define __HelpFunction_H

#include <Arduino.h>
#include "constants.h"

char **split(char **argv, int *argc, char *string, const char delimiter, int allowempty);

String ConvetWeekDay_UStoCZ(tm *timeInfoPRT);
String ConvetWeekDay_UStoSK(tm *timeInfoPRT);
void OdosliCasDoWS(void);

bool SkontrolujCiJePovolenyDenvTyzdni(u8 Obraz, tm *timeInfoPRT);

bool Input_digital_filtering(VSTUP_t *input_struct, uint16_t filterCas);
void ScanInputs(void);
void System_init(void);

int getIpBlock(int index, String str);
IPAddress str2IP(String str);
String ipToString(IPAddress ip);
int8_t NacitajEEPROM_setting(void);

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len);

void WiFi_init(void);
void WiFi_connect_sequencer(void); //vplas kazdych 10 sek loop

int getIpBlock(int index, String str);
String ipToString(IPAddress ip);
IPAddress str2IP(String str);
String handle_LenZobraz_IP_setting(void);
String handle_Zadavanie_IP_setting(void);
void handle_Nastaveni(AsyncWebServerRequest *request);
void OdosliStrankeVytapeniData(void);
uint8_t VypocitajSumuBuffera(uint8_t *buffer, uint16_t kolko);
uint8_t KontrolaSumyBuffera(uint8_t *buffer, uint16_t kolko);
void RS485sendBuffer(u8 buffr[], u8 pocetDat, u8 *timeToRx);
void RS485_TxModee(u8 *timeout);
void RS485_SendMSG(uint16_t DstAdr, uint16_t ScrAdr, uint16_t SqcNum, uint8_t CMD, uint16_t MsgIDnum, uint16_t Pocet, uint8_t *data, u8 *timeToRx);

#endif