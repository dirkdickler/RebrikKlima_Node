#include <string.h>
#include <stdlib.h>
#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <esp_task_wdt.h>
#include <esp_log.h>
#include "time.h"
#include <Ticker.h>
#include <EEPROM.h>
#include "index.html"
#include "main.h"
#include "define.h"

// #include "FS.h"
// #include "SD.h"
// #include "SPI.h"
#include <SoftwareSerial.h>
#include <ESP32Time.h>
#include "Pin_assigment.h"
#include "constants.h"
#include "HelpFunction.h"
#include "MyBlinker.h"


// Replace with your network credentials
// const char* ssid = "Grabcovi";
// const char* password = "40177298";
const char *soft_ap_ssid = "IOkarata_Klima";
const char *soft_ap_password = "VceliDvur";
// const char *ssid = "semiart";
// const char *password = "aabbccddff";
char NazovSiete[30] = {"semiart"};
char Heslo[30] = {"aabbccddff"};

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

JSONVar myObject, myObject2, ObjDatumCas, ObjTopeni, JSON_DebugMsg;
Ticker timer_1ms(Loop_1ms, 1, 0, MILLIS);
Ticker timer_10ms(Loop_10ms, 10, 0, MILLIS);
Ticker timer_100ms(Loop_100ms, 300, 0, MILLIS);
Ticker timer_1sek(Loop_1sek, 1000, 0, MILLIS);
Ticker timer_10sek(Loop_10sek, 10000, 0, MILLIS);

JSONVar Karty1, Karty_2;
JSONVar HodnotySet, HodnotyAktualne;
// SoftwareSerial swSer(14, 12, false, 256);
// char swTxBuffer[16];
// char swRxBuffer[16];
u8 RS485_toRx_timeout;

ESP32Time rtc;

IPAddress local_IP(192, 168, 1, 14);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);	// optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

struct tm MyRTC_cas;
bool Internet_CasDostupny = false; // to je ze dostava cas z Inernetu
bool RTC_cas_OK = false;		   // ze mam RTC fakt nastaveny bud z interneru, alebo nastaveny manualne
								   // a to teda ze v RTC mam fakr realny cas
								   // Tento FLAG, nastavi len pri nacitanie casu z internutu, alebo do buducna manualne nastavenie casu cew WEB

const u8 mojePrmenaae = 25;
u16_t cnt = 0;

TERMOSTAT_t room[12];
char gloBuff[200];
bool LogEnebleWebPage = false;
u16 cas_rebrikuSet = 3600; // sekundach
unsigned long klimaJednotkaEnable = 0;
LedBlinker led(LedOrange, COMMON_NEGATIVE);

/**********************************************************
 ***************        SETUP         **************
 **********************************************************/

void setup()
{
	Serial.begin(115200);
	log_i("Spustam applikaciu...1666");
	System_init();
	ESPinfo();

	// myObject["hello"] = " 11:22 Streda";
	//  myObject["true"] = true;
	//  myObject["x"] = 42;
	//  myObject2["Citac"] = 42;

	NacitajEEPROM_setting();
	// if (!EEPROM.begin(500))
	// {
	// 	log_i("Failed to initialise EEPROM");
	// }
	// log_i("Succes to initialise EEPROM");

	HodnotySet["casOhrevu"] = cas_rebrikuSet;
	HodnotySet["klima"] = klimaJednotkaEnable;

	WiFi_init();

	timer_1ms.start();
	timer_10ms.start();
	timer_100ms.start();
	timer_1sek.start();
	timer_10sek.start();
	esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
	esp_task_wdt_add(NULL);				  // add current thread to WDT watch

	// RS485 musis spustit az tu, lebo ak ju das hore a ESP ceka na konnect wifi, a pridu nejake data na RS485, tak FreeRTOS =RESET  asi overflow;
	Serial1.begin(9600);

	led.blink(200 /* time on */,
					  200 /* time off */,
					  3 /* cycles */,
					  1000 /* pause between secuences */,
					  0 /* secuences */,
					  NULL /* function to call when finished */
			);

	// swSer.begin(115200);
	// swSer.println("");
}

void loop()
{
	esp_task_wdt_reset();
	ws.cleanupClients();
	// AsyncElegantOTA.loop();
	timer_1ms.update();
	timer_10ms.update();
	timer_100ms.update();
	timer_1sek.update();
	timer_10sek.update();
	led.update();
}

void Loop_1ms()
{
	if (RS485_toRx_timeout) //&& (Serial1.availableForWrite() >= 100) ) //127 bytes je: PDF • 512 x 8-bit RAM shared by TX FIFOs and RX FIFOs of two UART controllers  128TX0 + 128Rx0 + 128TX1 + 128Rx1
	{
		if (--RS485_toRx_timeout == 0)
		{
			RS485_RxMode;
			log_i("[Loop 1ms]  Davam RS485 do RX mode");
		}
	}
}

void Loop_10ms()
{
	static uint8_t TimeOut_RXdata = 0;	 // musi byt static lebo sem skaces z Loop
	static uint16_t KolkkoNplnenych = 0; // musi byt static lebo sem skaces z Loop
	static char budd[250];				 // musi byt static lebo sem skaces z Loop

	uint16_t aktualny;
	char temp[200];

	aktualny = Serial1.available();
	if (aktualny)
	{
		// Serial2.readBytes (temp, aktualny);
		for (uint16_t i = 0; i < aktualny; i++)
		{
			if ((KolkkoNplnenych + aktualny) < sizeof(budd))
			{
				budd[KolkkoNplnenych + i] = Serial1.read();
			}
			else
			{
				Serial1.read();
			}
		}
		KolkkoNplnenych += aktualny;
		TimeOut_RXdata = 5;
	}

	if (TimeOut_RXdata)
	{
		if (--TimeOut_RXdata == 0)
		{
			{
				sprintf(temp, "[RS485] doslo:%u a to %s\r\n", KolkkoNplnenych, budd);
				DebugMsgToWebSocket(temp);

				RS485_PACKET_t *loc_paket;
				loc_paket = (RS485_PACKET_t *)budd;

				sprintf(temp, "[RS485]  DST adresa je:%u\r\n", loc_paket->DSTadress);
				DebugMsgToWebSocket(temp);
				// Serial.printf(temp);

				sprintf(temp, "[RS485] Mam adresu %u a idem ulozit data z RS485\r\n", loc_paket->SCRadress);
				DebugMsgToWebSocket(temp);

				sprintf(temp, "[RS485] CMD byte %u\r\n", loc_paket->CMD);
				DebugMsgToWebSocket(temp);

				if ((loc_paket->CMD & CMD_Reply) == CMD_Reply)
				{
					sprintf(temp, "[RS485] Reply od adrese %u a MSG ID je:%u\r\n", loc_paket->SCRadress, loc_paket->MsgID);
					DebugMsgToWebSocket(temp);
				}
				// 02 43 64 00 0a 00 01 20 03 00 05 00 04 f0 01 01 01 02 01 02 01 02 01 02 01  toto nastaveni na room[0] tep a RH na 258 NEKONTROLUJEM sumu

				if (loc_paket->SCRadress == 10)
				{
					room[0].T_podlaha = budd[indexData + 4];
					room[0].T_podlaha <<= 8;
					room[0].T_podlaha += budd[indexData + 3];
					room[0].T_vzduch = budd[indexData + 6];
					room[0].T_vzduch <<= 8;
					room[0].T_vzduch += budd[indexData + 5];
					room[0].RH_vlhkkost = budd[indexData + 8];
					room[0].RH_vlhkkost <<= 8;
					room[0].RH_vlhkkost += budd[indexData + 7];
					room[0].T_podlaha_SET = budd[indexData + 10];
					room[0].T_podlaha_SET <<= 8;
					room[0].T_podlaha_SET += budd[indexData + 9];
					room[0].T_vzduch_SET = budd[indexData + 12];
					room[0].T_vzduch_SET <<= 8;
					room[0].T_vzduch_SET += budd[indexData + 11];
					room[0].RH_vlhkkost_SET = budd[indexData + 14];
					room[0].RH_vlhkkost_SET <<= 8;
					room[0].RH_vlhkkost_SET += budd[indexData + 13];
					room[0].teleso = budd[indexData + 15];

					OdosliStrankeVytapeniData();
				}
				else if (loc_paket->SCRadress == 11)
				{
					room[1].T_podlaha = budd[indexData + 4];
					room[1].T_podlaha <<= 8;
					room[1].T_podlaha += budd[indexData + 3];
					room[1].T_vzduch = budd[indexData + 6];
					room[1].T_vzduch <<= 8;
					room[1].T_vzduch += budd[indexData + 5];
					room[1].RH_vlhkkost = budd[indexData + 8];
					room[1].RH_vlhkkost <<= 8;
					room[1].RH_vlhkkost += budd[indexData + 7];
					room[1].T_podlaha_SET = budd[indexData + 10];
					room[1].T_podlaha_SET <<= 8;
					room[1].T_podlaha_SET += budd[indexData + 9];
					room[1].T_vzduch_SET = budd[indexData + 12];
					room[1].T_vzduch_SET <<= 8;
					room[1].T_vzduch_SET += budd[indexData + 11];
					room[1].RH_vlhkkost_SET = budd[indexData + 14];
					room[1].RH_vlhkkost_SET <<= 8;
					room[1].RH_vlhkkost_SET += budd[indexData + 13];
					room[1].teleso = budd[indexData + 15];

					OdosliStrankeVytapeniData();
				}

				memset(budd, 0, sizeof(budd));
				KolkkoNplnenych = 0;
			}
		}
	}
}

void Loop_100ms(void)
{
	static bool LedNahodena = false;

	// if (LedNahodena == false)
	// {
	// 	LedNahodena = true;
	// 	// digitalWrite(LedGreen, LOW);
	// 	digitalWrite(LedOrange, LOW);
	// }
	// else
	// {
	// 	LedNahodena = false;
	// 	// digitalWrite(LedGreen, HIGH);
	// 	digitalWrite(LedOrange, HIGH);

	// 	// RS485_TxMode;
	// 	// Serial1.println("test RS485..orange LED High");
	// }

	cnt++;

	// Serial.println("[100ms] Loop");
	if (0) // cnt >= 10)
	{
		cnt++;
		myObject2["Citac"] = cnt; // " 11:22 Stredaaaa";
		String jsonString = JSON.stringify(myObject2);
		Serial.print("[100ms] Toto je string co odoslen strankam:");
		Serial.println(jsonString);
		ws.textAll(jsonString);
	}
}

void Loop_1sek(void)
{
	// log_i("-mam 1 sek....");
	//  Serial.println( random(10000,20000));
	if (Internet_CasDostupny == false)
	{
		//	Serial.print("Internet cas nedostupny !!,  ");
	}
	else
	{
		//		Serial.print("Internet cas dostupny,  ");
	}

	//	Serial.print("RTC cas cez func rtc.getTime: ");
	//	Serial.println(	rtc.getTime("%A, %B %d %Y %H:%M:%S"));
	rtc.getTime("%A, %B %d %Y %H:%M:%S");
	MyRTC_cas = rtc.getTimeStruct();
	// Serial.print("[1sek Loop]  free Heap je:");
	//  Serial.println(ESP.getFreeHeap());

	float flt = (float)ESP.getFreeHeap();
	flt /= 1000.0f;
	char locBuf[50];
	sprintf(locBuf, "%.3f", flt);
	//	log_i("HEAP free:%s", locBuf);

	String rr = "[1sek Loop] signalu: " + (String)WiFi.RSSI() + "dBm  a Heap: " + locBuf + " kB " +
				" Ine..\r\n ";

	DebugMsgToWebSocket(rr);
}

void Loop_10sek(void)
{
	log_i("-mam 10 sek....");
	static u8_t loc_cnt = 0;
	// Serial.println("\r\n[10sek Loop]  Mam Loop 10 sek..........");
	DebugMsgToWebSocket("[10sek Loop]  mam 1 sek....\r\n");
	log_i("Wifi status%u:", WiFi.status());

	WiFi_connect_sequencer();

	u8 locBuf[10];
	locBuf[0] = cas_rebrikuSet >> 8;
	locBuf[1] = (u8)cas_rebrikuSet;
	locBuf[2] = klimaJednotkaEnable >> 24;
	locBuf[3] = klimaJednotkaEnable >> 16;
	locBuf[4] = klimaJednotkaEnable >> 8;
	locBuf[5] = (u8)klimaJednotkaEnable;
	RS485_SendMSG(10, 2, 3, CMD_write, MsgID_HodnotyRebrikKarty, 6, locBuf, &RS485_toRx_timeout);

	String rr = "[Loop_10sek] To RS485 posielam paket s cas_rebrikuSet: " + (String)cas_rebrikuSet +
				"minut\r\n    a klimaJednotkaEnable (bin. obraz v DEC):" + (String)klimaJednotkaEnable + "\r\n";
	DebugMsgToWebSocket(rr);
}

void FuncServer_On(void)
{
	server.on("/",
			  HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  log_i("Doslo HTTP_GET iba");
				  // if (!request->authenticate("ahoj", "xxxx"))
				  // return request->requestAuthentication();
				  // request->send_P(200, "text/html", index_html, processor);
				  request->send_P(200, "text/html", Main);
			  });

	server.on("/LenZobraznastavip", HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  LogEnebleWebPage = true;
				  request->send(200, "text/html", handle_LenZobraz_IP_setting());
			  });

	server.on("/nastavip",
			  HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  log_i("Doslo HTTP_GET nastavIP");
				  if (!request->authenticate("admin", "VceliDvur"))
					  return request->requestAuthentication();
				  request->send(200, "text/html", handle_Zadavanie_IP_setting());
			  });

	server.on("/Nastaveni",
			  HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  handle_Nastaveni(request);
				  request->send(200, "text/html", "Nastavujem a ukladam do EEPROM");
				  log_i("Idem resetovat ESP");
				  delay(2000);
				  esp_restart();
			  });

	server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
			  {
				  char ttt[500];
				  u16 citac = EEPROM.readUShort (EE_citacZapisuDoEEPORM);
				  //u16_t citac2 = EEPROM.readUShort (EE_citac2_ZapisuDoEEPORM);

				  char loc_buf[20];
				  char loc_buf1[60];
				  char loc_buf2[100];
				  if (Internet_CasDostupny == true)
				  {
					  sprintf(loc_buf, "dostupny :-)");
				  }
				  else
				  {
					  sprintf(loc_buf, "nedostupny!!");
				  }

				  if (RTC_cas_OK == false)
				  {
					  sprintf(loc_buf2, "[RTC_cas_OK == flase] RTC NE-maju realny cas!!. RTC hodnota: ");
				  }
				  else
				  {
					  sprintf(loc_buf2, "[RTC_cas_OK == true] RTC hodnota: ");
				  }
				  strftime(loc_buf1, sizeof(loc_buf1), " %H:%M:%S    %d.%m.%Y    ", &MyRTC_cas);

				  sprintf(ttt, "Firmware :%s<br>"
							   "Sila signalu WIFI(-30 je akoze OK):%i<br>"
							   "Internet cas: %s<br>"
							   "%s %s<br>"
								"Pocet zapisov do EEPROM:%u<br> ",
						  firmware, WiFi.RSSI(), loc_buf, loc_buf2, loc_buf1,citac);

				  request->send(200, "text/html", ttt); });

	server.on("/reset",
			  HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  if (!request->authenticate("admin", "radecek78"))
					  return request->requestAuthentication();

				  request->send(200, "text/html", "resetujem!!!");
				  delay(1000);
				  esp_restart();
			  });

	server.on("/debug",
			  HTTP_GET,
			  [](AsyncWebServerRequest *request)
			  {
				  LogEnebleWebPage = true;
				  request->send_P(200, "text/html", DebugLog_html);
			  });

	server.on("/GetNastaveneHodnoty", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(200, "text/html", handle_GetNastaveneHodnoty()); });

	server.on("/SetNastaveneHodnoty", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(200, "text/html", handle_SetNastaveneHodnoty(request)); });
}

String handle_GetNastaveneHodnoty()
{
	// for (u8_t i = 0; i < PocetKaret; i++)
	// {
	// 	char aaa[12];
	// 	memset(aaa, 0, 12);
	// 	EEPROM.readBytes((i * 11) + EE_zacateKaret_1, aaa, 10); //EE_zacateKaret je offset kede zacianju karty
	// 	Karty1[String(i)] = aaa;								//EEPROM.readString (i * 11);
	// }

	HodnotySet["casOhrevu"] = cas_rebrikuSet;
	HodnotySet["klima"] = klimaJednotkaEnable;
	String jsonString = JSON.stringify(HodnotySet);
	// //Serial.print ("\r\nToto je je JSON string pri GetRFID:\r\n");
	// //Serial.println (jsonString);
	return jsonString;
}

String handle_SetNastaveneHodnoty(AsyncWebServerRequest *request)
{
	u16_t pocetArgumentu = request->params();

	log_i("-Client posiela pocet argumentu: %u", pocetArgumentu);

	String jsonString = "\0";

	String rr = "Prisiel JSON s pocetArgumentu: " + (String)pocetArgumentu +
				" a to tieto \r\n ";
	DebugMsgToWebSocket(rr);

	if (pocetArgumentu == 2) // pocetKlimaJednotek + 1) //+1 je glbalny cas rebriku
	{
		// for (u8_t i = 0; i < pocetArgumentu; i++)
		// {
		// 	char buff[10];
		// 	AsyncWebParameter *p = request->getParam(i);
		// 	sprintf(buff, "%s", p->name().c_str());
		// 	log_i("Cislo para:%u ma name:%s", i, buff);

		// 	if (request->hasParam(buff)) // otestuj ic mam query tento parameter, inak by sa ti resetleo ESP
		// 	{
		// 		String par1 = request->getParam(buff)->value(); // server.arg (buff);
		// 		log_i("key %s ma hodnotu: %s", buff, par1);

		// 		rr = "JOSN key: " + (String)buff + " key hodnota: " + (String)par1 + " \r\n ";
		// 		DebugMsgToWebSocket(rr);

		// 		// char aaa[12];
		// 		// memset(aaa, 0, sizeof(aaa));
		// 		// par1.toCharArray(aaa, 11);

		// 		// u8_t PozicevEEPROM = atoi(buff);
		// 		// Karty1[String(PozicevEEPROM)] = aaa;
		// 		// EEPROM.writeString((PozicevEEPROM * 11) + EE_zacateKaret_1, Karty1[String(PozicevEEPROM)]);
		// 	}
		// }

		AsyncWebParameter *p = request->getParam(0);
		char buff[10];
		if (p->name() == "casOhrevu")
		{
			log_i("Json nasiel key: casOhrevu");
			sprintf(buff, "%s", p->name().c_str());
			if (request->hasParam(buff))
			{
				String par1 = request->getParam(buff)->value(); // server.arg (buff);
				cas_rebrikuSet = par1.toInt();
				log_i("JSON  key: casOhrevu ma hodnotu %u", cas_rebrikuSet);
			}
			else
			{
				cas_rebrikuSet = 60;
				log_i("Nasiel som v JSON key: cas_rebriku, ale nema ziadnu VALUE,davam na 60 minut");
			}
		}
		else
		{
			cas_rebrikuSet = 60;
			log_i("Nenasiel som v JSON nasiel key: cas_rebriku, davam na 60 minut");
		}

		p = request->getParam(1);
		if (p->name() == "klima")
		{
			log_i("Json nasiel key: klima");
			sprintf(buff, "%s", p->name().c_str());
			if (request->hasParam(buff))
			{
				String par1 = request->getParam(buff)->value(); // server.arg (buff);
				klimaJednotkaEnable = par1.toInt();
				log_i("JSON  key: klima ma hodnotu %u", klimaJednotkaEnable);
			}
			else
			{
				klimaJednotkaEnable = 0;
				log_i("Nasiel som v JSON nasiel key: klima ale nema ziaden VALUE, klima davam klimaeneble = 0");
			}
		}
		else
		{
			klimaJednotkaEnable = 0;
			log_i("Nenasiel som v JSON nasiel key: klima davam klimaeneble = 0");
		}
		// 02 43 64 00 0a 00 01 20 03 00 05 00 04 f0   01 01 01 02 01 02 01 02 01 02 01  toto nastaveni na room[0] tep a RH na 258 NEKONTROLUJEM sumu
		log_i("-odosielam do RS458 paket");
		// u8 loc_buf[20] = {0x2, 0x43, 0x0a, 0x0, 0xb, 0x0, 0x01, 0x80, 0x0c, 0x0, 0x6, 0x0, 0xb9, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01};
		// RS485sendBuffer(loc_buf, 19, &RS485_toRx_timeout);
		u8 locBuf[10];

		locBuf[0] = cas_rebrikuSet >> 8;
		locBuf[1] = (u8)cas_rebrikuSet;
		locBuf[2] = klimaJednotkaEnable >> 24;
		locBuf[3] = klimaJednotkaEnable >> 16;
		locBuf[4] = klimaJednotkaEnable >> 8;
		locBuf[5] = (u8)klimaJednotkaEnable;

		u16 temp = EEPROM.readUShort(EE_citacZapisuDoEEPORM);
		if (1) // temp < 0xffff)
		{
			temp++;
			EEPROM.writeUShort(EE_citacZapisuDoEEPORM, temp);
		}
		EEPROM.writeUShort(EE_cas_rebrikuSet, cas_rebrikuSet);
		EEPROM.writeULong(EE_klimaJednotkaEnable, klimaJednotkaEnable);
		EEPROM.commit();

		RS485_SendMSG(10, 2, 3, CMD_write, MsgID_HodnotyRebrikKarty, 6, locBuf, &RS485_toRx_timeout);

		String rr = "[handle_SetNastaveneHodnoty] To RS485 posielam paket\r\n";
		DebugMsgToWebSocket(rr);

		// // Serial.println ("\r\nKarty som poukldal a idem na inkrement");
		// u16_t citac = EEPROM.readUShort(EE_citacZapisuDoEEPORM);
		// citac++;
		// EEPROM.writeUShort(EE_citacZapisuDoEEPORM, citac);
		// EEPROM.commit();

		HodnotySet["casOhrevu"] = cas_rebrikuSet;
		HodnotySet["klima"] = klimaJednotkaEnable;
		jsonString = JSON.stringify(HodnotySet);
	}
	else
	{
		jsonString = "Velky pocet argumentu v Query stringu!!  MAx pocet karet je 100!!!"; // PocetKaret
	}

	// //Serial.print ("\r\nToto je je JSON stringpr SetRFID:\r\n");
	// //Serial.println (jsonString);
	return jsonString;
}

void ESPinfo(void)
{
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	Serial.println("\r\n*******************************************************************");
	Serial.print("ESP Board MAC Address:  ");
	Serial.println(WiFi.macAddress());
	Serial.println("\r\nHardware info");
	Serial.print("APB CLOCK: ");
	Serial.print(APB_CLK_FREQ);
	Serial.println(" Hz");
	Serial.printf("%d cores Wifi %s%s\n",
				  chip_info.cores,
				  (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
				  (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
	Serial.printf("\r\nESP32 Chip Revision: %d\r\n ", chip_info.revision);
	Serial.printf("%dMB %s flash\r\n",
				  spi_flash_get_chip_size() / (1024 * 1024),
				  (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embeded" : "external");

	Serial.printf("\r\nTotal heap: %d\r\n", ESP.getHeapSize());
	Serial.printf("Free heap: %d\r\n", ESP.getFreeHeap());
	Serial.printf("Total PSRAM: %d\r\n", ESP.getPsramSize());
	Serial.printf("Free PSRAM: %d\r\n", ESP.getFreePsram()); // log_d("Free PSRAM: %d", ESP.getFreePsram());
	Serial.print("Alokujem buffer psdRamBuffer  500kB PSRAM");
	byte *psdRamBuffer = (byte *)ps_malloc(500000);
	Serial.printf(" -Free PSRAM: %d\r\n", ESP.getFreePsram());
	Serial.print("Uvolnujem buffer psdRamBuffer 500kz PSRAM ");
	free(psdRamBuffer);
	Serial.printf(" Free PSRAM po uvolneni : %d\r\n", ESP.getFreePsram()); // log_d("Free PSRAM: %d", ESP.getFreePsram());
	Serial.println("\r\n*******************************************************************");
	Serial.print("ESP32 SDK: ");
	Serial.println(ESP.getSdkVersion());
}

void DebugMsgToWebSocket(String textik)
{
	if (LogEnebleWebPage == true)
	{
		String sprava = rtc.getTime("%H:%M:%S ");
		JSON_DebugMsg["DebugMsg"] = sprava + textik;
		String jsonString = JSON.stringify(JSON_DebugMsg);
		ws.textAll(jsonString);
	}
}

void initWebSocket()
{
	ws.onEvent(onEvent);
	server.addHandler(&ws);
}
