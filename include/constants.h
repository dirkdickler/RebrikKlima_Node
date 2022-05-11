#ifndef INC_CONSTANST_H
#define INC_CONSTANST_H

#include <Arduino.h>
#include "define.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


static const char deviceName[31] = { "Zapsi Eng FW 1                " };
static const char MCU_type[9] = { "F412RE" };

static const char path_DI_Lock_txt[]  = { "log/di_lock.txt" };

typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef const int32_t sc32; /*!< Read Only */
typedef const int16_t sc16; /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */

typedef volatile int32_t vs32;
typedef volatile int16_t vs16;
typedef volatile int8_t vs8;

typedef volatile const int32_t vsc32; /*!< Read Only */
typedef volatile const int16_t vsc16; /*!< Read Only */
typedef volatile const int8_t vsc8;   /*!< Read Only */

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef const uint32_t uc32; /*!< Read Only */
typedef const uint16_t uc16; /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef volatile uint32_t vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t vu8;

typedef volatile const uint32_t vuc32; /*!< Read Only */
typedef volatile const uint16_t vuc16; /*!< Read Only */
typedef volatile const uint8_t vuc8;   /*!< Read Only */


#define HWversion 1
#define SWversion 1

//definovani Typ Nodu  a ID  sprav  26012022
//definovani modulu
#define ModulType_KU68 1
#define ModulType_TopneHlavice 2
#define ModulType_Svetla 3
#define ModulType_StackUp 4

#define MsgID_Ping 1
#define MsgID_BusMute 2
#define MsgID_TermostatData 3
#define MsgID_NameraneHodnoty 4
#define MsgID_Digital_vstupy 10
#define MsgID_Digital_vystupy 11
#define MsgID_PingAdvance 20
// -------- Konec definicie sprav a typu Nodu ----------

// --------  definicie sprav special pre Vcelidvur karta teopnych rebrikov a klimatizacii ----------
#define MsgID_HodnotyRebrikKarty 200   //tu je prvy u16 minuty kolko ma byt zebrik zopnuty a potom 32bit  obraz povolenych klima jednotek  tj celkovo 6 bytes




#define MsgID_PublicNazvyMistnosti 0xff00
#define MsgID_PublicHodnoty 0xff01
#define MsgID_GoToBoot 0xff02
#define MsgID_WriteToFlashAdress 0xff03
#define MsgID_ExitFromBoot 0xff04
#define MsgID_EraseAplSection 0xff05


#define  broadcast 0xFFFF

#define  Preamble1 0x02
#define  Frame     0x43

#define  CMD_bit_RW 7  
#define  CMD_read  0 
#define  CMD_write 0x80 


#define CMD_Ack_OK    0
#define CMD_Ack_Busy  1
#define CMD_Ack_NOK   2
#define CMD_Reply     0x20
#define CMD_Event     0x40
#define CMD_RW    0x80

//ACK_NOK 
#define NOKcode_countOverflow  1  //vela dat v pakete
#define NOKcode_sumaError      2  //nesedi suma
#define NOKcode_missingData    3  //chynaju data v pakete
#define NOKcode_naznamyMSGID   4  //


typedef struct
{  
	u8 typVstupu;     //klasika,citac,ci ma automaticky padat fo nule
	u16 pin;
	u8 input;
	u8 input_prew;
	u16  filter;
	u32 counter;
	u16 conter_filter;
	bool zmena;
}VSTUP_t;


typedef struct //
{
	bool   RTCinit_done;
	bool   eth_connect;
	bool   test_spustDNS;
	bool   SDkarta_OK;
	bool   analog10sek_uz_ulozoeny;
	bool   minutaPerioda_uz_spracovana;

} FLAGS_t;

typedef struct
{
	char name[16];
	i16 T_podlaha;
	i16 T_vzduch;
	i16 RH_vlhkkost;

	i16 T_podlaha_SET;
	i16 T_vzduch_SET;
	i16 RH_vlhkkost_SET;
	u8 teleso;
	u8 RS485_timeout;
} TERMOSTAT_t;


typedef struct
{
	u8 Prea;
	u8 Fram;
	u16 DSTadress; //cilova adresa
	u16 SCRadress; //zdrojova adresa
	u8 SQCnum;
	u8 CMD;
	u16 MsgID;
	u16 Data_count; //9,10
	u8 sumaHead;
	u8 sumaData;
	u8 *data;
} RS485_PACKET_t;


extern FLAGS_t flg;

#endif
