#include "servoLib/servo_control.h" //Libreria servo
#include "oled/menu.h"  //Libreria display
#include "gps/NMEAParser.h"  //Libreria set-up e parsing gps
#include "led/led.h"  //Libreria per controllo LED
#include <Arduino.h>
#include <QMC5883LCompass.h>  //Libreria controllo bussola
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

#define UART1_RX 5
#define UART1_TX 4
#define GPS_nEN 18  // Attenzione: GPS_EN è negato ==> attivo basso
#define UART_BAUDRATE 9600 // Baud rate per comunicazione UART con GPS
#define CHUNK_SIZE 8
#define BUFFER_SIZE 80 // Dimensione massima del buffer UART, corrisponde a massima lunghezza di una frase NMEA

// Messaggi per la configurazione del SAM-M8Q
uint8_t UBX_CFG_MSG_GGA_OFF[] = {                   // UBX-CFG-MSG NMEA-GGA Off on All interfaces
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,             // Header
    0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload
    0xFF, 0x23};                                    // Checksum

uint8_t UBX_CFG_MSG_GSV_OFF[] = {                   // UBX-CFG-MSG NMEA-GSV Off on All interfaces
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
    0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x38};  
                                  
uint8_t UBX_CFG_MSG_GSA_OFF[] = {                   // UBX-CFG-MSG NMEA-GSA Off on All interfaces
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
    0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x31};                                    

uint8_t UBX_CFG_MSG_VTG_OFF[] = {                   // UBX-CFG-MSG NMEA-VTG Off on All interfaces
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
    0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x04, 0x46};
                                    
uint8_t UBX_CFG_MSG_GLL_OFF[] = {                   // UBX-CFG-MSG NMEA-GLL Off on All interfaces
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00,
    0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x2A};                                    

// Stati della MSF che controlla il flow di esecuzione
typedef enum {
    STATE_ACTIONS,
    STATE_POSITIONING,
    STATE_FINE_POINTING,
    STATE_WAIT_AUTO,
    STATE_ZENIT,
    STATE_MANUAL,
    STATE_WAIT_BACK,
    STATE_GPS_ACQUIRE
}State_t;

/* Tipo che rappresenta uno stato:                                            */
/* state => nome stato                                                        */
/* state_function => puntatore a funzione, cio che viene eseguito nello stato */
typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

/* Funzioni che catturano la pressione di un pulsante */
void btn_up_pressed(void);
void btn_down_pressed(void);
void btn_left_pressed(void);
void btn_right_pressed(void);

/* Funzioni di ogni stato */
void fn_ACTIONS(void);
void fn_POSITIONING(void);
void fn_FINE_POINTING(void);
void fn_WAIT_AUTO(void);
void fn_ZENIT(void);
void fn_MANUAL(void);
void fn_WAIT_BACK(void);
void fn_GPS_ACQUIRE(void);

State_t current_state = STATE_ACTIONS; //stato di partenza

QMC5883LCompass compass;  // variabile per controllo della bussola

ServoController servo_controller(PIN_SERVO_AZ,PIN_SERVO_EL,PIN_VOLTAGE,PIN_VOLTAGE); //variabile per controllo servo

int btn_pressed_id; //variabile per identificare il bottone premuto, 0->none 1->up 2->down 3->left 4->right

int manual_control_mode=0; //variabile per identificare il servo da muovere in modalità manuale, 0->az 1->el

/* menu_dept: spostamenti a dx e sx nel menu               */
/* menu_hight_1: spostamenti su e giu nella prima pagina   */
/* menu_hight_2: spostamenti su e giu nella seconda pagina */
int menu_dept=1, menu_hight_1=2, menu_hight_2=1;

long int attesa_auto_pos;  //variabile che tiene traccia del momento dell'ultimo posizionamento automatico

char read_char;  //variabile che tiene i caratteri letti dal gps

// Buffer per il canale seriale UART (8 bit)
char buffer_uart1[BUFFER_SIZE] = "";

MessageRMC message;  //variabile per lo store del parsing dei dati GPS

std::string sent;

double az_val;  //valore di azimuth del sole
double el_val;  //valore di elevazione del sole

void setupM8Q();  //funzione per inizializzare il gps

int pacMan(int start ,int val, int range);  //funzione per mantenere in range menu_hight_1 e menu_hight_2

/* creazione della macchina a stati */
StateMachine_t fsm[] = {
                      {STATE_ACTIONS, fn_ACTIONS},
                      {STATE_POSITIONING, fn_POSITIONING},
                      {STATE_FINE_POINTING, fn_FINE_POINTING},
                      {STATE_WAIT_AUTO, fn_WAIT_AUTO},
                      {STATE_ZENIT, fn_ZENIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_GPS_ACQUIRE, fn_GPS_ACQUIRE}
};

/* stato che gestisce la pressione dei bottoni */
void fn_ACTIONS(){
  switch(btn_pressed_id){
    case 1/*UP*/:{
      if(menu_dept == 1){
        menu_hight_1=pacMan(2,--menu_hight_1,3);
        clear_disp();
      }
      if(menu_dept == 2 && menu_hight_1 == 3){
        menu_hight_2=pacMan(1,--menu_hight_2,3);
        clear_disp();
      }
      break;
    }
    case 2/*DOWN*/:{
      if(menu_dept == 1){
        menu_hight_1=pacMan(2,++menu_hight_1,3);
        clear_disp();
      }
      if(menu_dept == 2 && menu_hight_1 == 3){
        menu_hight_2=pacMan(1,++menu_hight_2,3);
        clear_disp();
      }
      break;
    }
    case 3/*LEFT*/:{
      if(menu_dept == 2){
        --menu_dept;
        clear_disp();
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
        clear_disp();
      }
      if(menu_dept == 1){
        ++menu_dept;
        clear_disp();
      }
      break;
    }
  }
  
  btn_pressed_id = 0;
  delay(100);
}

/* funzione per il posizionamento automatico */
void fn_POSITIONING(void){
  compass.read();
  servo_controller.auto_positioning(&(message.time),message.latitude,message.longitude,400,compass.getAzimuth(),&az_val,&el_val);
 
  /* messaggi seriali di debug */
  Serial.print("el: ");
  Serial.println(el_val);
  Serial.println(message.longitude);
  Serial.println(message.latitude);
    
  current_state = STATE_FINE_POINTING;
  btn_pressed_id == 0;
}

/* stato per aggiustare la posizione dei servo utilizzando la tensione letta */
void fn_FINE_POINTING(void){
  servo_controller.max_power_pos(2,5);  //vengono eseguiti 5 step da 2 gradi per ogni direzione
  attesa_auto_pos=millis();  //viene salvato il tempo dell'ultimo posizionamento
  current_state = STATE_WAIT_AUTO;
  btn_pressed_id == 0;
}

/* stato che attende o il superamento dell'intervallo di tempo per poter rieseguire il */
/* posizionamento automatico o la pressione del bottone sinistro per uscire dalla      */
/* modalità automatica                                                                 */
void fn_WAIT_AUTO(void){
  if(millis()-attesa_auto_pos > 180000){ // passati 3 minuti
    current_state = STATE_GPS_ACQUIRE;
  }
  if(btn_pressed_id == 3){ //pressione tasto sinistro
    --menu_dept;
    clear_disp();
    current_state = STATE_ACTIONS;
  }
  btn_pressed_id = 0;

}

/* Il pannello viene posizionato verticalmente */
void fn_ZENIT(void){
  servo_controller.servoEl.set_servo(90);
  current_state = STATE_ACTIONS;
  --menu_dept;
  clear_disp();
  btn_pressed_id == 0;
}

/* Funzione di controllo manuale */
void fn_MANUAL(void){
  switch(btn_pressed_id){
    case 1/*UP*/:{ //movimento di uno dei servo
      if(manual_control_mode==0){
        servo_controller.servoAz.set_servo(servo_controller.servoAz.get_alpha()+5);
      }else{
        servo_controller.servoEl.set_servo(servo_controller.servoEl.get_alpha()+5);
      }
      break;
    }
    case 2/*DOWN*/:{ //movimento di uno dei servo
      if(manual_control_mode==0){
        servo_controller.servoAz.set_servo(servo_controller.servoAz.get_alpha()-5);
      }else{
        servo_controller.servoEl.set_servo(servo_controller.servoEl.get_alpha()-5);
      }
      break;
    }
    case 3/*LEFT*/:{ //uscita dalla modalita manuale
      menu_dept--;
      clear_disp();
      btn_pressed_id == 0;
      current_state = STATE_ACTIONS;
      break;
    }
    case 4/*RIGHT*/:{ //cambio del servo pilotato
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

/* stato per l'acquisizione e il parsing dei dati gps */
void fn_GPS_ACQUIRE(void) {
  int iterNotFixed = 0;
  int iterFixed = 0;
  int totIter = 0;
  int ledIter = 0;
  bool flag = false;

  while(iterFixed < 10 && iterNotFixed < 20) { // Il ciclo di acquisizione prosegue finché non sono stati ricevuti 10 messaggi fixed o 20 non fixed
    if (Serial2.available())
    {
      read_char = Serial2.read();
      strncat(buffer_uart1, &read_char, 1); // Legge un carattere alla volta dalla seriale e lo inserisce nel buffer_uart1
      if (read_char == '\n') // Se il carattere ricevuto è il terminatore ho acquisito una sentence intera
      {
        if (!flag)
        { // Ignora il contenuto del primo buffer -> potrebbe contenere messaggi GNTXT che romperebbero parseRMC
          flag = true;
          strcpy(buffer_uart1, ""); // Svuota buffer
        }
        else
        {
          sent.assign(buffer_uart1, strlen(buffer_uart1)); // Popolo la stringa sent
          if (parseRMC(sent, message))
          { // GPS non fixato
            singleLedIfFixed(false, totIter); // Un led alla volta lampeggia di arancione
            /* Messaggi per debug */
            Serial.print(sent.data());
            Serial.println("ITER");
            Serial.println(iterNotFixed);

            iterNotFixed++;
          }
          else
          { // GPS fixato
            singleLedIfFixed(true, totIter); // Un led alla volta lampeggia di verde
            /* Messaggi per debug */
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

  /* Messaggi per debug */
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

  if (iterFixed >= 10)
  { // Ricevuti almeno 10 messaggi validi
    for (int i = 0; i < 3; ++i)
    {
      // Tutti i led lampeggiano tre volte di verde
      setAllLedGreen();
      delay(200);
      setLedOff();
      delay(200);
    }
    current_state = STATE_POSITIONING; // Ho acquisito dati buoni, quindi passo al posizionamento dei motori
  }
  else
  {
    for (int i = 0; i < 3; ++i)
    {
      // Tutti i led lampeggiano tre volte di arancione
      setAllLedOrange();
      delay(200);
      setLedOff();
      delay(200);
    }
    attesa_auto_pos = millis();
    current_state = STATE_WAIT_AUTO; // Torno ad aspettare per un intervallo di tempo prefissato prima di riposizionare
  }
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
  // Apertura seriale virtuale (USB) a 9600 baud
  Serial.begin(9600);

  /* setup pin I2C */
  Wire.setSDA(0);
  Wire.setSCL(1);

  setup_disp();

  /* Setup gps */
  // Setta i pin 5 e 4 come ricevitore e trasmettitore per UART1 (Possibile che siano già settati di default)
  // Uso Serial2, vedi pinout Raspberry Pi Pico e specifiche seriali della libreria Arduino-pico
  Serial2.setRX(UART1_RX);
  Serial2.setTX(UART1_TX);
  Serial2.begin(UART_BAUDRATE); // UART0 su PICO (RP2040)
  
  // Setup del ricevitore GNSS
  pinMode(GPS_nEN, OUTPUT);
  digitalWrite(GPS_nEN, LOW); // GPS_EN e' attivo basso
  setupM8Q();
  delay(5000);

  /*setup pulsanti*/
  pinMode(BTN_UP,INPUT_PULLUP);
  pinMode(BTN_DOWN,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BTN_RIGHT,INPUT_PULLUP);

  /* funzioni per attivare attivare gli interrupt sui pin dei pulsanti */
  attachInterrupt(digitalPinToInterrupt(BTN_UP), btn_up_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_DOWN), btn_down_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), btn_left_pressed, FALLING );
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), btn_right_pressed, FALLING );

  pinMode(PIN_VOLTAGE,INPUT); //pin di ingresso della tensione del pannellino solare

  compass.init(); //Inizializzazione bussola

  /* Primo display del menu */
  SelezioneMenu(menu_hight_1,menu_dept,menu_hight_2,PIN_VOLTAGE,servo_controller.servoAz.get_alpha(),servo_controller.servoEl.get_alpha());

  /* Impostazione dello stato di partenza */
  current_state = STATE_ACTIONS;
}

void loop() {
  /* viene eseguita la funzione corrispondente al current state */
  fsm[current_state].state_function(); 

  /* aggiornamento del menu */
  SelezioneMenu(menu_hight_1,menu_dept,menu_hight_2,PIN_VOLTAGE,servo_controller.servoAz.get_alpha(),servo_controller.servoEl.get_alpha());
}

/*
Invio i messaggi di setup definiti in precedenze al ricevitore GNSS per sospendere la ricezione di tutti i messaggi ad eccezione dei GNRMC
ATTENZIONE: è fondamentale aspettare almeno 1000ms tra un invio e l'altro (come da HW Integration Manual) per assicurare
            la corretta ricezione dei messaggi
*/
void setupM8Q() {
  delay(2000);
  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GGA_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GGA_OFF[i]);
  }
  // Debug
  Serial.println("Fatto GGA");
  delay(1000);

  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GSV_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GSV_OFF[i]);
  }
  // Debug
  Serial.println("Fatto GSV");
  delay(1000);

  for(size_t i = 0; i < sizeof(UBX_CFG_MSG_GSA_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GSA_OFF[i]);
  }
  // Debug
  Serial.println("Fatto GSA");
  delay(1000);

  for (size_t i = 0; i < sizeof(UBX_CFG_MSG_VTG_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_VTG_OFF[i]);
  }
  // Debug
  Serial.println("Fatto VTG");
  delay(1000);

  for (size_t i = 0; i < sizeof(UBX_CFG_MSG_GLL_OFF); ++i) {
    Serial2.write(UBX_CFG_MSG_GLL_OFF[i]);
  }
  // Debug
  Serial.println("Fatto GLL");
}


int pacMan(int start ,int val, int range){
  val = val>range ? start : val;
  val = val<start ? range : val;
  return val;
}
