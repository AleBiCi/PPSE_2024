#include "led/led.h"

NeoPixelConnect p(PIN_RGB_LED, NUM_LED, pio0, 0);

void setAllLedRed(){
    p.neoPixelFill(255,0,0,true);
    p.neoPixelClear(true);
}

void setAllLedGreen(){
    p.neoPixelFill(0,255,0,true);
    p.neoPixelClear(true);
}

void setAllLedBlue(){
    p.neoPixelFill(0,0,255,true);
    p.neoPixelClear(true);
}

void setAllLedOrange(){
    p.neoPixelFill(255,153,0,true);
    p.neoPixelClear(true);
}

void setLedOff() {
    p.neoPixelClear(true);
}

void circleCompleteLoopLed(){
    for(int i=0; i<NUM_LED; i++){
        p.neoPixelSetValue(i,255,0,0,true);        
        delay(1000);
    }
}

void circleSerialLed(){
    for(int i=0; i<NUM_LED; i++){
        p.neoPixelSetValue(i,255,0,0,true);        
        delay(1000);
    }
    p.neoPixelClear(true);
}

void singleLedIfFixed(bool fixed, int num_iter) {
    num_iter%=NUM_LED;
    if(fixed) {
        p.neoPixelSetValue(num_iter, 0, 255, 0, true);
        delay(100);
        p.neoPixelClear(true);
    } else {
        p.neoPixelSetValue(num_iter, 255, 0, 0, true);
        delay(100);
        p.neoPixelClear(true);
    }
}
