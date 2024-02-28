#include "oled/menu.h"

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

float Tensione(int pin_tensione) {
  int valore_letto = analogRead(pin_tensione);
  float v_out = valore_letto * (5.0 / 1023.0);
  //if(v_out>voltage_max){
  //  voltage_max=v_out;
  //}
  //if(v_out<voltage_min){
  //  voltage_min=v_out;
  //}
  return v_out;
}

Adafruit_SSD1306 disp(128, 64, &Wire);

void setup_disp(){
  if(!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void clear_disp(){
  disp.clearDisplay();
}


void drawMenuElement(int x, int y, const unsigned char *bitmap, char* name) {
  disp.drawBitmap(x, y, bitmap, 16, 16, WHITE);
  disp.setTextSize(1);
  disp.setTextColor(WHITE);
  disp.setCursor(x + 28, y + 3);
  disp.print(name);
  disp.display();
}

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

void drawMenu1(int i) {
  //display.clearDisplay();
  switch (i) {
    case 1:
      drawMenuElement(4, 2, bitmap_history, home_menu_names[3]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_specs, home_menu_names[0]);
      drawMenuElement(4, 48, bitmap_GPS, home_menu_names[1]);
      break;

    case 2:
      drawMenuElement(4, 2, bitmap_specs, home_menu_names[0]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_GPS, home_menu_names[1]);
      drawMenuElement(4, 48, bitmap_move, home_menu_names[2]);
      break;

    case 3:
      drawMenuElement(4, 2, bitmap_GPS, home_menu_names[1]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_move, home_menu_names[2]);
      drawMenuElement(4, 48, bitmap_history, home_menu_names[3]);
      break;

    case 4:
      drawMenuElement(4, 2, bitmap_move, home_menu_names[2]);
      disp.drawRect(1, 22, 126, 20, WHITE);
      drawMenuElement(4, 24, bitmap_history, home_menu_names[3]);
      drawMenuElement(4, 48, bitmap_specs, home_menu_names[0]);
      break;
  }
}

void drawMenu2(int i, int sel, int pin_v, float phi, float teta) {
  char tensione_string[5];
  char phi_string[5];
  char theta_string[5];
  char max_tension_string[5];
  char min_tension_string[5];
  //display.clearDisplay();

  switch (i) {
    //specs
    case 1:
      
      dtostrf(Tensione(pin_v), 5, 2, tensione_string);
      drawMenuElement(4, 24, bitmap_icon_v, tensione_string);
      break;

    //GPS
    case 2:
      dtostrf(phi, 5, 2, phi_string);
      drawMenuElement(4, 2, bitmap_icon_phi, phi_string);

      dtostrf(teta, 5, 2, theta_string);
      drawMenuElement(4, 24, bitmap_icon_theta, theta_string);
      break;

    //move
    case 3:

      switch (sel) {
        case 1:
          drawMenuElement(4, 2, bitmap_icon_joistick, home_menu_move[2]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_move, home_menu_move[0]);
          drawMenuElement(4, 48, bitmap_icon_move3, home_menu_move[1]);
          disp.display();
          break;

        case 2:
          drawMenuElement(4, 2, bitmap_move, home_menu_move[0]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_icon_move3, home_menu_move[1]);
          drawMenuElement(4, 48, bitmap_icon_joistick, home_menu_move[2]);
          disp.display();
          break;

        case 3:
          drawMenuElement(4, 2, bitmap_icon_move3, home_menu_move[1]);
          disp.drawRect(1, 22, 126, 20, WHITE);
          drawMenuElement(4, 24, bitmap_icon_joistick, home_menu_move[2]);
          drawMenuElement(4, 48, bitmap_move, home_menu_move[0]);
          disp.display();
          break;
      }

      break;

    //history
    case 4:
      
      //dtostrf(Tensione(pin_v), 5, 2, tensione_string);
      //drawMenuElement(4, 2, bitmap_max_irradiance, max_tension_string);

      //dtostrf(Tensione(pin_v), 5, 2, min_tension_string);
      //drawMenuElement(4, 24, bitmap_min_irradiance, min_tension_string);
      break;
  }
}

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

int pacMan_1_n(int n, int val){
  if(val<1){
    val = n;
  }
  if(val>n){
    val = 1;
  }
  return val;
}

int menu_keep_range_1_3(int val){
  if(val<1){
    val = 1;
  }
  if(val>3){
    val = 3;
  }
  return val;
}

