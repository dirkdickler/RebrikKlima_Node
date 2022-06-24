#ifndef __DEFINE_H
#define __DEFINE_H

#include "Pin_assigment.h"

typedef struct
{
	String Nazov;
	float Tset;
} ROOM_t;

#define input1 0
#define pocetKlimaJednotek 26

#define pocetDIN_celkovo 6 // DIN + SD CD pin , !!! ale daj o +1 lebo nevim preco ho to prepisovalo davalo do neho RTC sek :-)
#define filterTime_DI 3		// 10ms loop

#define RS485_RxMode digitalWrite(RS485_DirPin, LOW);
#define RS485_TxMode digitalWrite(RS485_DirPin, HIGH);
#define RS485_TimeOut 50 // ms
#define indexData 14

#define WDT_TIMEOUT 5
#define firmware "ver20220624_2"

// EEPROM adrese
#define EE_IPadresa 00					// 16bytes
#define EE_SUBNET 16						// 16bytes
#define EE_Brana 32						// 16bytes
#define EE_NazovSiete 50				// 30bytes  48
#define EE_Heslosiete 80				// 30bytes  64
#define EE_citacZapisuDoEEPORM 110	// 2bytes       84
#define EE_cas_rebrikuSet 112        // 2bytes       86
#define EE_klimaJednotkaEnable 114   // 4bytes  	    88
#define EE_dalsi 118
#define EE_zacateKaret_1 200
#define EE_zacateKaret_2 1300 // EE_zacateKaret + 100*11tj 1300

#endif