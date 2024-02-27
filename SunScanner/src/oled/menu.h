#ifndef __MENU_H__
#define __MENU_H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "icons.h"

//float voltage_min = 100,voltage_max=0;

float Tensione(int pin_tensione);

void setup_disp();

void clear_disp();

void drawMenuElement(int x, int y, const unsigned char *bitmap, char* name);

void drawMenu1(int i);

void drawMenu2(int i, int sel, int pin_v, float phi, float teta);

void drawMenu3(int i);

void SelezioneMenu(int i, int j, int sel, int pin_v, float phi, float teta);

int pacMan_1_n(int n, int val); 

int menu_keep_range_1_3(int val);

#endif
