/*
 MyBlinker.h - A LED blinker library for Arduino platform.
 Created by Evert Arias. July 13, 2017.
 MyBlinker is provided Copyright © 2017 under MIT License.
*/

#ifndef MyBlinker_h
#define MyBlinker_h

#include "Arduino.h"


#define MINIMUM_INTERVAL 20
#define COMMON_NEGATIVE 0
#define COMMON_POSITIVE 1

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

class LedBlinker {
private:
	byte mPin;
  uint8_t mPolarity = COMMON_NEGATIVE;
	byte mBlinks;
	unsigned int mOnDuration;
	unsigned int mOffDuration;
	unsigned int mPauseDuration;
	unsigned int mSequences;
	unsigned long mStartTime;
	unsigned long mLastRunTime;
	void (*mFinishedCallbackFunction)();
public:
  LedBlinker(byte const pin);
	LedBlinker(byte const pin, uint8_t ledPolarity);
	void setPin(byte const pin);
  void setPolarity(uint8_t polarity);
	void blink(unsigned int const onDuration,
		unsigned int const offDuration,
		byte const blinks,
		unsigned int const pauseDuration,
		unsigned int const sequences,
	  void (*finishedCallbackFunction)());
	void on();
	void off();
	void onUntil(unsigned int const onDuration, void (*finishedCallbackFunction)());
	void offUntil(unsigned int const offDuration, void (*finishedCallbackFunction)());
	void update();
};


#endif
