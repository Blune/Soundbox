#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <avr/sleep.h>

#define wake_pin 2
#define power_pin_player 12
#define reference 1.1

byte PRR_reg;
byte randomTrack;
byte playerBootingTime = 100;
float shutdown_voltage = 3.15;
SoftwareSerial mySoftwareSerial(10,11);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
  pinMode(wake_pin, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(power_pin_player, OUTPUT);
  digitalWrite(power_pin_player, HIGH);
  
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);

  delay(playerBootingTime);
  if (!myDFPlayer.begin(mySoftwareSerial)) 
  {  
    Serial.println(F("Could not boot DFPlayer Mini. Stopped setup"));
    while(true);
  }
  
  SetupPlayer();
  Serial.println(F("DFPlayer Mini setup done."));
}

void loop()
{
  digitalWrite(power_pin_player, HIGH);
  randomTack = getRandomSoundForPressedButton();
  if (batVoltage() <= shutdown_voltage) 
  {
    standby();
  }
  
  delay(playerBootingTime);
  
  if(randomTrack)
  {
    SetupPlayer();
    
    myDFPlayer.play(randomTrack);
    do
    {
      randomTrack = getRandomSoundForPressedButton();
    }
    while(randomTrack);
    
    for(int i = 0; i < 620; i++)
    {
      randomTrack = getRandomSoundForPressedButton();
      if(randomTrack)
      {
        i = 0;
        myDFPlayer.play(randomTrack);
        do
        {
          randomTrack = getRandomSoundForPressedButton();
        }
        while(randomTrack);
      }
      delay(100);
    }
  }
  digitalWrite(power_pin_player, LOW);
  standby();
}

byte getRandomSoundForPressedButton()
{
  if(!digitalRead(3)) return random(6,8);
  else if(!digitalRead(4)) return random(2,4);
  else if(!digitalRead(5)) return 5;
  else if(!digitalRead(6)) return random(8,13);
  else if(!digitalRead(7)) return 4;
  else if(!digitalRead(8)) return 1;
  return 0;
}

void setupPlayer()
{
  myDFPlayer.setTimeOut(500);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.volume(10);
}

float batVoltage()
{
  byte ADCSRB_before = ADCSRB; 
  byte ADCSRA_before = ADCSRA;
  byte ADMUX_before = ADMUX;
  ADCSRB = 0;
  ADMUX = 64 + 14;
  ADCSRA = 6;
  ADCSRA |= (1 << ADATE);
  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADSC);
  unsigned int adc_16 = 0;
  for (byte n=0; n<64; n++)
  {
    while (!(ADCSRA & 16));
    adc_16 += ADC;
    ADCSRA |= 16;
  }
  ADMUX = ADMUX_before;
  ADCSRA = ADCSRA_before;
  ADCSRB = ADCSRB_before;
  return reference * 65536 / adc_16;
}

void standby()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  attachInterrupt (digitalPinToInterrupt(wake_pin), waked, LOW);
  PRR_reg = PRR; 
  sleep_enable();
  PRR |= 64+32+8+4+2;
  sleep_mode();
}

void waked()
{
  detachInterrupt(digitalPinToInterrupt(wake_pin));
  PRR = PRR_reg;
}
