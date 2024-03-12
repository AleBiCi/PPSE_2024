#ifndef __MENU_H__
#define __MENU_H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "icons.h"

//float voltage_min = 100,voltage_max=0;

/* Lettura tensione */
float Tensione(int pin_tensione);

/* Setup del display */
void setup_disp();

void clear_disp();

/* Funzione per disegnare una singola opzione del menu */
void drawMenuElement(int x, int y, const unsigned char *bitmap, char* name);

/* menu iniziale a profondità 1 */
void drawMenu1(int i);

/*Menu di secondo livello: dopo aver premuto il pulsante dx*/
void drawMenu2(int i, int sel, int pin_v, float phi, float teta);

/* Informazioni a schermo dopo aver selezionato una delle tre move */
void drawMenu3(int i);

/* Funzione che seleziona il menu da printare */
void SelezioneMenu(int i, int j, int sel, int pin_v, float phi, float teta);


#endif
