#include "servoLib/servo_control.h"
#include "oled/menu.h"
#include "gps/NMEAParser.h"
#include "led/led.h"
#include <Arduino.h>
#include <QMC5883LCompass.h>
#include <string>
// #include <Adafruit_NeoPixel.h>
#include <NeoPixelConnect.h>

#define BTN_UP 22
#define BTN_DOWN 23
#define BTN_LEFT 25
#define BTN_RIGHT 24

#define PIN_SERVO_AZ 7
#define PIN_SERVO_EL 8

#define PIN_VOLTAGE 29
#define BUCK_EN 11 // Pin da pullappare per attivare il circuito di alimentazione 12V->5V 

#define UART1_RX 5
#define UART1_TX 4
#define GPS_nEN 18  // Attenzione: GPS_EN è negato ==> attivo basso
#define UART_BAUDRATE 9600 // Baud rate per comunicazione UART con GPS
#define CHUNK_SIZE 8
#define BUFFER_SIZE 256

typedef enum {
    STATE_IDLE,
    STATE_ACTIONS,
    STATE_POSITIONING,
    STATE_FINE_POINTING,
    STATE_WAIT_AUTO,
    STATE_ZENIT,
    STATE_MANUAL,
    STATE_WAIT_BACK,
    STATE_GPS_ACQUIRE
}State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

void btn_up_pressed(void);
void btn_down_pressed(void);
void btn_left_pressed(void);
void btn_right_pressed(void);

void fn_IDLE(void);
void fn_ACTIONS(void);
void fn_POSITIONING(void);
void fn_FINE_POINTING(void);
void fn_WAIT_AUTO(void);
void fn_ZENIT(void);
void fn_MANUAL(void);
void fn_WAIT_BACK(void);
void fn_GPS_ACQUIRE(void);

State_t current_state = STATE_IDLE;

QMC5883LCompass compass;

ServoController servo_controller(PIN_SERVO_AZ,PIN_SERVO_EL,PIN_VOLTAGE,PIN_VOLTAGE);

int btn_pressed_id; //0->none 1->up 2->down 3->left 4->right

int manual_control_mode=0; //0->az 1->el

int menu_dept=1, menu_hight_1=1, menu_hight_2=1;

int attesa_auto_pos;

char read_char;

// Buffer per il canale seriale UART (carattere, 8 bit)
char buffer_uart1[BUFFER_SIZE] = "";

MessageRMC message;

std::string sent;

double az_val;
double el_val;

void setupM8Q();

int pacMan(int val, int range);

StateMachine_t fsm[] = {
                      {STATE_IDLE, fn_IDLE},
                      {STATE_ACTIONS, fn_ACTIONS},
                      {STATE_POSITIONING, fn_POSITIONING},
                      {STATE_FINE_POINTING, fn_FINE_POINTING},
                      {STATE_WAIT_AUTO, fn_WAIT_AUTO},
                      {STATE_ZENIT, fn_ZENIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_WAIT_BACK, fn_WAIT_BACK},
                      {STATE_GPS_ACQUIRE, fn_GPS_ACQUIRE}
};

void fn_IDLE(){
  if(btn_pressed_id!=0){
    current_state = STATE_ACTIONS;
  }
  delay(100);
  btn_pressed_id = 0;
}

void fn_ACTIONS(){
  switch(btn_pressed_id){
    case 1/*UP*/:{
      if(menu_dept == 1){
        menu_hight_1=pacMan(--menu_hight_1,4);
      }
      if(menu_dept == 2 && menu_hight_1 == 3){
        menu_hight_2=pacMan(--menu_hight_2,3);
      }
      break;
    }
    case 2/*DOWN*/:{
      if(menu_dept == 1){
        menu_hight_1=pacMan(++menu_hight_1,4);
      }
      if(menu_dept == 2 && menu_hight_1 == 3){
        menu_hight_2=pacMan(++menu_hight_2,3);
      }
      break;
    }
    case 3/*LEFT*/:{
      if(menu_dept == 2){
        --menu_dept;
      }
      break;
    }
    case 4/*RIGHT*/:{
      if(menu_dept == 2 && menu_hight_1 == 3){
        switch(menu_hight_2){
          case 1:{
            current_state = STATE_GPS_ACQUIRE;
            break;
          }
          case 2:{
            current_state = STATE_ZENIT;
            break;
          }
          case 3:{
            current_state = STATE_MANUAL;
            break;
          }
        }
        ++menu_dept;
      }
      if(menu_dept == 1){
        ++menu_dept;
      }
      break;
    }
  }
  
  btn_pressed_id = 0;
  delay(100);
}

void fn_POSITIONING(void){
  message.time.tm_isdst = false;
  //compass.read();
  servo_controller.auto_positioning(&(message.time),message.latitude,message.longitude,400,compass.getAzimuth(),&az_val,&el_val);
  
  current_state = STATE_FINE_POINTING;
  btn_pressed_id == 0;
}

void fn_FINE_POINTING(void){
  servo_controller.max_power_pos(2,5);
  attesa_auto_pos=millis();
  current_state = STATE_WAIT_AUTO;
  btn_pressed_id == 0;
}

void fn_WAIT_AUTO(void){
  if(millis()-attesa_auto_pos > 180000){
    current_state = STATE_GPS_ACQUIRE;
  }
  // Led di stato
  // messaggio a schermo per esito acquisizione
  if(btn_pressed_id == 3){
    --menu_dept;
    current_state = STATE_ACTIONS;
  }
  btn_pressed_id = 0;

}

void fn_ZENIT(void){
  servo_controller.servoEl.set_servo(90);
  current_state = STATE_ACTIONS;
  --menu_dept;
  btn_pressed_id == 0;
}

void fn_MANUAL(void){
  switch(btn_pressed_id){
    case 1/*UP*/:{
      if(manual_control_mode==0){
        servo_controller.servoAz.set_servo(servo_controller.servoAz.get_alpha()+5);
      }else{
        servo_controller.servoEl.set_servo(servo_controller.servoEl.get_alpha()+5);
      }
      break;
    }
    case 2/*DOWN*/:{
      if(manual_control_mode==0){
        servo_controller.servoAz.set_servo(servo_controller.servoAz.get_alpha()-5);
      }else{
        servo_controller.servoEl.set_servo(servo_controller.servoEl.get_alpha()-5);
      }
      break;
    }
    case 3/*LEFT*/:{
      /*NEED TO CHANGE MENU POS*/
      menu_dept--;
      btn_pressed_id == 0;
      current_state = STATE_ACTIONS;
      break;
    }
    case 4/*RIGHT*/:{
      if(manual_control_mode==0){
        manual_control_mode = 1;
      }else{
        manual_control_mode = 0;
      }
      break;
    }
  }
  btn_pressed_id = 0;
}


void fn_WAIT_BACK(void){

  if(btn_pressed_id == 3){
    menu_dept--;
    btn_pressed_id == 0;
    current_state = STATE_ACTIONS;
  }
  btn_pressed_id == 0;
  delay(100);
}

void fn_GPS_ACQUIRE(void) {
  int iterNotFixed = 0;
  int iterFixed = 0;
  int totIter = 0;
  int ledIter = 0;
  bool flag = false;

  while(iterFixed < 10 && iterNotFixed < 20) {
    if (Serial2.available())
    {
      read_char = Serial2.read();
      strncat(buffer_uart1, &read_char, 1);
      if (read_char == '\n')
      {
        if (!flag)
        { // Ignora il contenuto del primo buffer, in quanto può contenere messaggi GNTXT che romperebbero parseRMC
          flag = true;
          strcpy(buffer_uart1, "");
        }
        else
        {
          sent.assign(buffer_uart1, strlen(buffer_uart1));          
          if (parseRMC(sent, message))
          { // GPS non fixato
            singleLedIfFixed(false, totIter);
            Serial.print(sent.data());
            Serial.println("ITER");
            Serial.println(iterNotFixed);
            iterNotFixed++;
          }
          else
          { // GPS fixato
            singleLedIfFixed(true, totIter);
            Serial.print(sent.data());
            Serial.println("ITER FIXED");
            Serial.println(iterFixed);
            iterFixed++;
          }
          totIter++;
          strcpy(buffer_uart1, "");
        }
      }
    }
  }

  Serial.print(message.latitude);
  Serial.println();
  Serial.print(message.longitude);
  Serial.println();
  Serial.print(message.time.tm_hour);
  Serial.println();
  Serial.print(message.time.tm_min);
  Serial.println();
  Serial.print(message.time.tm_sec);
  Serial.println();
  Serial.print(message.time.tm_mday);
  Serial.println();
  Serial.print(message.time.tm_mon);
  Serial.println();
  Serial.print(message.time.tm_year);
  Serial.println();

  if(iterFixed >= 10) {
    current_state=STATE_POSITIONING;
  } else current_state=STATE_WAIT_AUTO;
}

void btn_up_pressed(void){
  btn_pressed_id = 1;
}
void btn_down_pressed(void){
  btn_pressed_id = 2;
}
void btn_left_pressed(void){
  btn_pressed_id = 3;
}
void btn_right_pressed(void){
  btn_pressed_id = 4;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(BUCK_EN, OUTPUT);
  digitalWrite(BUCK_EN, HIGH);

  Serial.begin(9600);

  Wire.setSDA(0);
  Wire.setSCL(1);

  setup_disp();

  // Setta i pin 5 e 4 come ricevitore e trasmettitore per UART1 (Possibile che siano già settati di default)
  // Uso Serial2, vedi pinout Raspberry Pi Pico e specifiche seriali della libreria Arduino-pico
  Serial2.setRX(UART1_RX);
  Serial2.setTX(UART1_TX);

  Serial2.begin(UART_BAUDRATE); // UART0 su PICO (RP2040)
  
  pinMode(GPS_nEN, OUTPUT);
  digitalWrite(GPS_nEN, LOW); // GPS EN e' attivo basso

  pinMode(BTN_UP,INPUT_PULLUP);
  pinMode(BTN_DOWN,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BTN_RIGHT,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_UP), btn_up_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_DOWN), btn_down_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), btn_left_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), btn_right_pressed, FALLING );

  pinMode(PIN_VOLTAGE,INPUT);

  setupM8Q();
  delay(5000);

  current_state = STATE_ACTIONS;
}

void loop() {
  fsm[current_state].state_function();
  SelezioneMenu(menu_hight_1,menu_dept,menu_hight_2,PIN_VOLTAGE,servo_controller.servoAz.get_alpha(),servo_controller.servoEl.get_alpha());
}

void setupM8Q() {
  byte UBX_CFG_MSG_GGA_OFF[] = {// UBX-CFG-MSG NMEA-GGA Off on All interfaces
            0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
            0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
            0xFF, 0x23}; // Checksum
  byte UBX_CFG_MSG_GSV_OFF[] = {// UBX-CFG-MSG NMEA-GSV Off on All interfaces
            0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
            0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
            0x02, 0x38}; // Checksum
  byte UBX_CFG_MSG_GSA_OFF[] = {// UBX-CFG-MSG NMEA-GSA Off on All interfaces
            0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
            0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
            0x01, 0x31}; // Checksum
  byte UBX_CFG_MSG_VTG_OFF[] = {// UBX-CFG-MSG NMEA-VTG Off on All interfaces
            0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
            0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
            0x04, 0x46}; // Checksum
  byte UBX_CFG_MSG_GLL_OFF[] = {// UBX-CFG-MSG NMEA-GLL Off on All interfaces
            0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
            0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
            0x00, 0x2A}; // Checksum

  delay(2000);
  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GGA_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GGA_OFF[i]);
  }
  Serial.println("Fatto GGA");
  delay(1000);

  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GSV_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GSV_OFF[i]);
  }
  Serial.println("Fatto GSV");
  delay(1000);

  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GSA_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GSA_OFF[i]);
  }
  Serial.println("Fatto GSA");
  delay(1000);

  for (size_t i = 0; i < sizeof(UBX_CFG_MSG_VTG_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_VTG_OFF[i]);
  }
  Serial.println("Fatto VTG");
  delay(1000);

  for (size_t i = 0; i < sizeof(UBX_CFG_MSG_GLL_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GLL_OFF[i]);
  }
  Serial.println("Fatto GLL");
}


int pacMan(int val, int range){
  val = val>range ? 1 : val;
  val = val<1 ? range : val;
  return val;
}
