#ifndef __LED_H__
#define __LED_H__

#include <NeoPixelConnect.h>
#define NUM_LED 8
#define PIN_RGB_LED 13

void setAllLedRed();

void setAllLedGreen();

void setAllLedBlue();

void setAllLedOrange();

void setLedOff();

void circleCompleteLoopLed();

void circleSerialLed();

void singleLedIfFixed(bool fixed, int num_iter);

#endif
