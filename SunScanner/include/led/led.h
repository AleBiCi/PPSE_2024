#ifndef __LED_H__
#define __LED_H__

#include <NeoPixelConnect.h>
#define NUM_LED 8
#define PIN_RGB_LED 16

NeoPixelConnect p(PIN_RGB_LED, NUM_LED, pio0, 0);

void setAllLedRed();

void setAllLedGreen();

void setAllLedBlue();

void circleCompleteLoopLed();

void circleSerialLed();

#endif