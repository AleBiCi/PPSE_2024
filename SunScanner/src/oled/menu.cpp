#include "oled/menu.h"

/* dichiarazione delle bitmap del menu */
extern const unsigned char bitmap_specs [] PROGMEM;
extern const unsigned char bitmap_GPS [] PROGMEM;
extern const unsigned char bitmap_move [] PROGMEM;
extern const unsigned char bitmap_history [] PROGMEM;
extern const unsigned char bitmap_icon_p [] PROGMEM;
extern const unsigned char bitmap_icon_v [] PROGMEM;
extern const unsigned char bitmap_icon_i [] PROGMEM;
extern const unsigned char bitmap_icon_phi [] PROGMEM;
extern const unsigned char bitmap_icon_theta [] PROGMEM;
extern const unsigned char bitmap_icon_move3 [] PROGMEM;
extern const unsigned char bitmap_icon_joistick [] PROGMEM;
extern const unsigned char bitmap_max_irradiance [] PROGMEM;
extern const unsigned char bitmap_min_irradiance [] PROGMEM;
extern const unsigned char bitmap_UP [] PROGMEM;
extern const unsigned char bitmap_DOWN [] PROGMEM;
extern const unsigned char bitmap_BACK [] PROGMEM;
extern const unsigned char bitmap_CHANGE [] PROGMEM;
extern const unsigned char bitmap_wait [] PROGMEM;

const unsigned char* home_menu[4] = { bitmap_specs, bitmap_GPS, bitmap_move, bitmap_history};
char *home_menu_names[4] = { (char*)"Specs", (char*)"GPS", (char*)"Move", (char*)"History"};
char *home_menu_move[4] = { (char*)"Automatic", (char*)"Zenit", (char*)"Manual"};

/* Lettura tensione */
float Tensione(int pin_tensione) {
  int valore_letto = analogRead(pin_tensione);
  float v_out = valore_letto * (5.0 / 1023.0);  //conversione dal livello di analogRead a tensione effettiva
  return v_out;
}

Adafruit_SSD1306 disp(128, 64, &Wire);  //Dichiarazione display

/* Setup del display */
void setup_disp(){
  if(!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  disp.clearDisplay();
  disp.display();
}

void clear_disp(){
  disp.clearDisplay();
}

/* Funzione per disegnare una singola opzione del menu */
void drawMenuElement(int x, int y, const unsigned char *bitmap, char* name) {
  disp.drawBitmap(x, y, bitmap, 16, 16, WHITE);  //icona da printare
  disp.setTextSize(1);
  disp.setTextColor(WHITE);
  disp.setCursor(x + 28, y + 3);
  disp.print(name);  //testo printare
  disp.display();
}

/* Schermata di caricamento: 3 quadratini che appaiono in sequenza */
void WaitMove() {
  disp.clearDisplay();

  disp.drawBitmap(44, 28, bitmap_wait, 8, 8, WHITE);
  disp.display();
  delay(500);

  disp.drawBitmap(60, 28, bitmap_wait, 8, 8, WHITE);
  disp.display();
  delay(500);

  disp.drawBitmap(76, 28, bitmap_wait, 8, 8, WHITE);
  disp.display();
  delay(500);
}

 //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* menu iniziale a profondità 1 */
void drawMenu1(int i) {
  //display.clearDisplay();
  switch (i) {
    case 1 /*SPECS*/:
      drawMenuElement(4, 2, bitmap_history, home_menu_names[3]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_specs, home_menu_names[0]);
      drawMenuElement(4, 48, bitmap_GPS, home_menu_names[1]);
      break;

    case 2 /*GPS*/:
      drawMenuElement(4, 2, bitmap_specs, home_menu_names[0]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_GPS, home_menu_names[1]);
      drawMenuElement(4, 48, bitmap_move, home_menu_names[2]);
      break;

    case 3 /*move*/:
      drawMenuElement(4, 2, bitmap_GPS, home_menu_names[1]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_move, home_menu_names[2]);
      drawMenuElement(4, 48, bitmap_history, home_menu_names[3]);
      break;

    case 4 /*history*/:
      drawMenuElement(4, 2, bitmap_move, home_menu_names[2]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_history, home_menu_names[3]);
      drawMenuElement(4, 48, bitmap_specs, home_menu_names[0]);
      break;
  }
}

/*Menu di secondo livello: dopo aver premuto il pulsante dx*/
void drawMenu2(int i, int sel, int pin_v, float phi, float teta) {
  //Dichiarazione di variabili che vengono poi printate a schermo
  char tensione_string[5];
  char phi_string[5];
  char theta_string[5];
  char max_tension_string[5];
  char min_tension_string[5];

  switch (i) {
    case 1 /*specs*/:
      
      dtostrf(Tensione(pin_v), 5, 2, tensione_string);
      drawMenuElement(4, 24, bitmap_icon_v, tensione_string);
      break;

    case 2/*GPS*/:
      dtostrf(phi, 5, 2, phi_string);
      drawMenuElement(4, 2, bitmap_icon_phi, phi_string);

      dtostrf(teta, 5, 2, theta_string);
      drawMenuElement(4, 24, bitmap_icon_theta, theta_string);
      break;

    case 3 /*move*/:
      //altro switch perchè nel caso si selezioni move ci sono 3 opzioni possibili
      switch (sel) {
        case 1 /*automatic*/:
          drawMenuElement(4, 2, bitmap_icon_joistick, home_menu_move[2]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_move, home_menu_move[0]);
          drawMenuElement(4, 48, bitmap_icon_move3, home_menu_move[1]);
          disp.display();
          break;

        case 2 /*zenit*/:
          drawMenuElement(4, 2, bitmap_move, home_menu_move[0]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_icon_move3, home_menu_move[1]);
          drawMenuElement(4, 48, bitmap_icon_joistick, home_menu_move[2]);
          disp.display();
          break;

        case 3 /*manuale*/:
          drawMenuElement(4, 2, bitmap_icon_move3, home_menu_move[1]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_icon_joistick, home_menu_move[2]);
          drawMenuElement(4, 48, bitmap_move, home_menu_move[0]);
          disp.display();
          break;
      }

      break;

    case 4 /*history*/:
      
      dtostrf(Tensione(pin_v), 5, 2, tensione_string);
      drawMenuElement(4, 2, bitmap_max_irradiance, max_tension_string);

      dtostrf(Tensione(pin_v), 5, 2, min_tension_string);
      drawMenuElement(4, 24, bitmap_min_irradiance, min_tension_string);
      break;
  }
}

/* Informazioni a schermo dopo aver selezionato una delle tre move */
void drawMenu3(int i) {
  switch (i) {
    case 1:
        //WaitMove();
      break;

    case 2:
        //WaitMove();
      break;

    case 3:
      disp.drawBitmap(56, 3, bitmap_UP, 16, 16, WHITE);
      disp.setTextSize(1);
      disp.setTextColor(WHITE);
      disp.setCursor(44, 6);
      disp.print("UP");

      disp.drawBitmap(35, 24, bitmap_BACK, 16, 16, WHITE);
      disp.setTextSize(1);
      disp.setTextColor(WHITE);
      disp.setCursor(9, 29);
      disp.print("BACK");

      disp.drawBitmap(56, 45, bitmap_DOWN, 16, 16, WHITE);
      disp.setTextSize(1);
      disp.setTextColor(WHITE);
      disp.setCursor(77, 50);
      disp.print("DOWN");

      disp.drawBitmap(77, 24, bitmap_CHANGE, 16, 16, WHITE);
      disp.setTextSize(1);
      disp.setTextColor(WHITE);
      disp.setCursor(80, 15);
      disp.print("CHANGE");

      disp.drawBitmap(60, 28, bitmap_wait, 8, 8, WHITE);
      disp.display();

      break;
  }
  disp.display();
}

/* Funzione che seleziona il menu da printare */
void SelezioneMenu(int i, int j, int sel, int pin_v, float phi, float teta){
  switch(j){
  case 1:
    drawMenu1(i);
    break;
  case 2:
    drawMenu2(i,sel,pin_v,phi,teta);
    break;
  case 3:
    drawMenu3(sel);
    break;
  }
}



