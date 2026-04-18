#include "Arduino.h"
#include <SparkFun_ST25DV64KC_Arduino_Library.h> // Click here to get the library:  http://librarymanager/All#SparkFun_ST25DV64KC
#include <mailboxHandling.h>
#include <PinChangeInterrupt.h>
#include "epd4in2_V2.h"
#include "imagedata.h"

//pin definitions
#define LED_PIN 7
#define VOLTAGEMONITOR_PIN 6

#define BUTTON1_PIN 16
#define BUTTON2_PIN 17

#define ST25GPO_PIN 14
#define ST25LPD_PIN 15

#define GPIO1_PIN 0
#define GPIO2_Pin 1

#define MIN_REFRESH_VOLTAGE 3.2f

SFE_ST25DV64KC_NDEF tag;
Epd epd;

uint8_t tagMemory[256];
uint16_t memLoc;

bool receivedGPO = false;
bool button1Pressed = false;
bool button2Pressed = false;

int statusWritePeriod = 2000;
int statusWriteDelta = 0;

long readVcc();
void ReadBWImageChunked();
void writeCardStatus(float voltage, bool btn1, bool btn2);

void ST25GPOChanged() { 
  receivedGPO = true; 
}
void Button1Pressed() 
{ 
  button1Pressed = digitalRead(BUTTON1_PIN);  
}
void Button2Pressed() 
{   
  button2Pressed = digitalRead(BUTTON2_PIN);  
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();    

    pinMode(BUTTON1_PIN, INPUT);
    pinMode(BUTTON2_PIN, INPUT);
    attachPCINT(digitalPinToPCINT(BUTTON1_PIN), Button1Pressed, RISING);
    attachPCINT(digitalPinToPCINT(BUTTON2_PIN), Button2Pressed, RISING);

    pinMode(ST25GPO_PIN, INPUT_PULLUP);   
    attachPCINT(digitalPinToPCINT(ST25GPO_PIN), ST25GPOChanged, FALLING); 

    pinMode(LED_PIN, OUTPUT);


    if (!tag.begin(Wire)) 
    {
        //ST25DV error
        while (1)
        {
            digitalWrite(LED_PIN, 1);
            delay(250);
            digitalWrite(LED_PIN, 0);
            delay(250);
        }
    }
    EnableFTM(tag);  
    
    if (epd.Init() != 0) 
    {
      //ST25DV error
      while (1)
      {
          digitalWrite(LED_PIN, 1);
          delay(1000);
          digitalWrite(LED_PIN, 0);
          delay(1000);
      }
    }    
    
    Serial.println("started");
}

void loop()
{  
  float voltage = int(readVcc())/1000.0; 
  bool button1Down = digitalRead(BUTTON1_PIN);
  bool button2Down = digitalRead(BUTTON2_PIN);
      
  if (receivedGPO && receivedMail(tag))
  {               
    Serial.println("asdf");
    receivedGPO = false;
    uint8_t msg[256];
    readMailbox(tag, msg);
    switch (msg[0])
    {
    case 0x01:
      Serial.println("LETS GOO!");
      ReadBWImageChunked();
      break;
    
    default:
      break;
    }
  }
  else if (button1Pressed || button2Pressed)
  {      
    writeCardStatus(voltage, button1Down, button2Down);
    statusWriteDelta = 0;
    button1Pressed = false;
    button2Pressed = false;
  }
  else if (statusWriteDelta >= statusWritePeriod)
  {
    writeCardStatus(voltage, button1Down, button2Down);
    statusWriteDelta = 0;
  }

  statusWriteDelta += 100;
  delay(100);
}

//
void ReadBWImageChunked()
{
  Serial.println("reading");
  epd.SendCommand(0x24); // WRITE_RAM
  for (int i = 0;i < 75;i++)
  {        
    unsigned char chunk[200];
    while (!receivedMail(tag))
    { delay(10); }            
    readMailbox(tag, chunk);
    for (int j = 0;j < 200;j++)
    {
      epd.SendData(chunk[j]);
    }
  }
  epd.TurnOnDisplay(); // refresh once

  receivedGPO = false;
}


void writeCardStatus(float voltage, bool btn1, bool btn2)
{                    
  uint8_t status[2];
  status[0] = (voltage-int(voltage))*100;
  //first half msb of voltage, 2nd half button info
  uint8_t data = (btn2 << 1) + btn1;
  status[1] = (data << 4) + int(voltage);              
  writeToMailbox(tag, status, 2);    
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
     ADMUX = _BV(MUX5) | _BV(MUX0) ;
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}