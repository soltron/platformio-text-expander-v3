//=======================================================================//
//includes
#include <avr/wdt.h>
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "comunicazione.h"  //routines per la comunicazione 485 tonali#
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
//=======================================================================//

//#define DEBUG //decommentare per attivare la scrittura su seriale per PC
#define LED_COUNT   1
#define LED_PIN     6
#define BRIGHTNESS  10

const char Address = 0x15;  //Indirizzo slave comunicazione scheda
//const char compile_date[] = __DATE__;
//const char compile_time[] = __TIME__;
//const char filename[] = __FILE__;
unsigned long tempo_loop, tempo_rx, tempo_address;
const unsigned int TIMEOUT = 50000; //timeout sulla 485
//String incomingString = "";

bool stringComplete = false;  // whether the string is complete
unsigned int num_rx = 0;
const byte leddb1 = 13;
unsigned long counterLN;
unsigned long counterTT;
int counterER;
byte led_db1 = 0;
byte inputpcfpin;

byte secondi;                // informazione dei secondi trascorsi dall'accensione
byte minuti = 0;             // informazione dei minuti trascorsi dall'accensione
byte ore = 0;               // informazione delle ore trascorse dall'accensione
byte giorni = 0;             // informazione dei giorni trascorsi dall'accensione
byte inpcf_val = 0;    
byte out_pcf = 0;
byte out_pcf_old =0;
byte r = 0;    
byte g = 0;
byte b = 0;
int virtual_timer1=0;
int virtual_timer2=0;
int virtual_timer3=0;
int virtual_timer4=0;

//=======================================================================//
// Create an ledStrip object and specify the pin it will use.
//PololuLedStrip<LED_PIN> ledStrip;
//rgb_color colors[1];
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//=======================================================================//
//globals

//=======================================================================//
//setup function runs once
void setup() {
  
//=======================================================================//
  MCUSR = 0; //!!!! ELIMINA IL PROBLEMA AL RIAVVIO CHE SI RESETTA CONTINUAMENTE
  wdt_disable();
//=======================================================================//
  inputString.reserve(35);  // reserve 35 bytes for the inputString
  tempString.reserve(35);
  
  Serial.begin(9600);
  Serial.setTimeout(500);
  while (!Serial);
  //Serial.println(compile_date);
  //Serial.println(compile_time);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(leddb1, OUTPUT);

  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  lcd.init();    // inizializza  lcd
  lcd.backlight();
  //====================================================================
  cli();//stop interrupts
  
  //set virtual_timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable virtual_timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  
  //set virtual_timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable virtual_timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  //set virtual_timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);   
  // enable virtual_timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();//allow interrupts
 //====================================================================
  //abilito il watchdog e imposto il tempo di reset 
  wdt_enable(WDTO_250MS);
 //====================================================================
}

void loop() {
  byte inpcf_val;
  tempo_loop = tick;
  //====================================================================
  //executed Virtual virtual_timer1
  if (virtual_timer1 >=200) {
    virtual_timer1=0;
   short channel = 1;
    // read PCF 
   Wire.requestFrom(0x22, 1 << channel);
   if (Wire.available())
   {
      inpcf_val = Wire.read();
   }
   Wire.endTransmission();
   //   Serial.println(inpcf_val);
    out_pcf=inpcf_val;
  }
 //====================================================================
  //executed Virtual virtual_timer2
 if (virtual_timer2>=300) {
  virtual_timer2=0;
  if (out_pcf_old != out_pcf) {
  out_pcf_old = out_pcf;
  // Escribir dato en cada uno de los 8 canales
    Wire.beginTransmission(0x23);
    Wire.write(~(out_pcf));
    Wire.endTransmission();
    }
 } 
  //====================================================================
  //executed Virtual virtual_timer3
  //skips first time and executed every 
  if (virtual_timer3>=2000) {
    virtual_timer3=0;     
//    lcd.noBacklight(); // turn off backlight
    lcd.clear();
    delay(5);
    lcd.print(giorni);
    lcd.print(F("-"));
    lcd.print(ore);
    lcd.print(F(":"));
    lcd.print(minuti);
    lcd.print(F(":"));
    lcd.print(secondi);
    lcd.print(F("  "));
    lcd.setCursor(0, 3);
    lcd.print(F("Rin"));
    lcd.print(counterLN, DEC);
    lcd.print(F(" ER"));
    lcd.print(counterER, DEC);
    lcd.print(F(" In"));
    lcd.print(inpcf_val, HEX);
    lcd.setCursor(0, 2);      
    lcd.print(F("Rx:"));
    lcd.print(counterTT, DEC);
     // }*/
   }
  //====================================================================
  //executed Virtual virtual_timer4
  if (virtual_timer4>=400) {
    virtual_timer4=0;
    led_db1 ^= 1;
    digitalWrite(A0, led_db1);
    out_pcf ^= (1<<4);
    cli();
    strip.setPixelColor(0, g, r, b); //Imposta il colore al nostro led
    strip.show();
    sei();
  }
  //====================================================================

  if ((tempo_loop - tempo_rx) > TIMEOUT) {
    //Senza comunicazione va in debug
    tempo_rx = tempo_loop;
    tempo_address = tempo_loop;
    Serial.flush();// clear the string:
    Serial.print(F("\nDebug:"));
    Serial.print(giorni);
    Serial.print(F("-"));
    Serial.print(ore);
    Serial.print(F(":"));
    Serial.print(minuti);
    Serial.print(F(":"));
    Serial.print(secondi);
    Serial.print(F("  inpcf:"));
    Serial.print(inpcf_val);
    Serial.print(F("  rxtot:"));
    Serial.print(counterLN, DEC);
    Serial.print(F("  ertot:"));
    Serial.println(counterER, DEC);
  }
//====================================================================
  if ((tempo_loop - tempo_address) > TIMEOUT) {
    //C'è comunicazione ma non viene interrogato il suo indirizzo
    tempo_address = tempo_loop;
  }
//====================================================================
  digitalWrite(leddb1, LOW); //led debug (lampeggia sulla comunicazione)
  unsigned long temp_calc = tick - time_now_rx ;
//====================================================================
  if ((temp_calc > 10) && on_com) { //Quando smetto di ricevere dati per 1.25 ms considero la stringa rx485 terminata > 10msec
    //time_now_rx = tick; // !!!!risolve il problema del disallineamento in rx :-(
    on_com = 0;
    stringComplete = 1;
  }
//====================================================================

  if (stringComplete) {
    cli();
    tempString="";
    byte lunghezza = inputString.length();
      for (byte x =0; x<=lunghezza+1;x++) {
        char inChar = inputString[x];
        tempString += inChar;
        //Serial.write(inChar);
      }
    inputString="";
    sei();
    counterTT++;
    //Serial.print("*");
    //Serial.println(tempString);
    digitalWrite(leddb1, HIGH);//led debug (lampeggia sulla comunicazione)
    //Serial.flush();// clear the string:
    tempo_rx = tick;
    stringComplete = false;
    byte pos_command = checkcomm();
    byte rx_add = tempString[pos_command - 1];
   
    if (Address == rx_add)   {
      tempo_address = tick;
      char cmd_com  = tempString[pos_command];
      byte rval_com = tempString[pos_command + 4];
      byte gval_com = tempString[pos_command + 5];
      byte bval_com = tempString[pos_command + 6];
      byte sommatoria = cecksum(pos_command);
      //        Serial.write(sommatoria);
      //        Serial.write(buttonState1);
      //        Serial.write(buttonState2);
      //        Serial.write(buttonState3);
      //        Serial.write(swState1);
      //        Serial.write(swState2);
      //        Serial.write(swState3);
      //        Serial.write(swState4);
      //        Serial.println(rx_add,HEX);
      //        Serial.print(rx_add,HEX);
      //        Serial.print("C");
      //        Serial.print(cmd_com,HEX);
      //        Serial.print(" L");
      //        Serial.print(lng_com,HEX);
      //        Serial.print("LV ");
      //        Serial.print(ledval_com,HEX);
      //        Serial.print("LUM ");
      //        Serial.print(lumval_com,HEX);
      //        Serial.print(" ");
      //        Serial.print(rval_com,HEX);
      //        Serial.print(" ");
      //        Serial.print(gval_com,HEX);
      //        Serial.print(" ");
      //        Serial.print(bval_com,HEX);
      //        Serial.print(" ck:");
      //        Serial.print(ckval_com,HEX);
      //        Serial.print(" ckcal:");
      //        Serial.print(sommatoria,HEX);
      //        Serial.println(" ");

      if ((cmd_com == 0x50) && (sommatoria)) {
        r=rval_com;
        g=gval_com;
        b=bval_com;
        counterLN++;
      } else {
        counterER++;
        //Serial.print(F("CK*"));
      }
    }
    //}
  }

//====================================================================
  //resetto il watchdog
  wdt_reset();
}
//====================================================================

ISR(TIMER0_COMPA_vect){//virtual_timer0 interrupt 2kHz 
//generates pulse wave of frequency 2kHz/2 = 1kHz (takes two cycles for full wave- toggle high then toggle low)
  virtual_timer1++;
  virtual_timer2++;
  virtual_timer3++;
  virtual_timer4++;
}

ISR(TIMER1_COMPA_vect){//virtual_timer1 interrupt 1Hz 
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  
  secondi ++ ; // trasformo la tmpsecvar tmp in secondi da 0 a 60
    // quando raggiungo 60 secondi incremento i minuti e aggiorno la tmpsecvar di trasformazione secondi minuti
  if (secondi >= 60) {
    secondi=0;
    minuti = minuti + 1;
  }
  //ogni 60 minuti incremento  ora e azzero la  minuti
  if (minuti >= 60) {
    minuti = 0;
    ore = ore + 1;
  }
  //ogni 24 ore incremento la  giorni e azzero la  ore
  if (ore >= 24) {
    ore = 0;
    giorni = giorni + 1;
  }
}
  
ISR(TIMER2_COMPA_vect){//virtual_timer1 interrupt 8kHz 
//generates pulse wave of frequency 8kHz/2 = 4kHz (takes two cycles for full wave- toggle high then toggle low)
  tick++;
}