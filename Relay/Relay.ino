
#include <Parladuino.h>
#include <sha1.h>
#include <Base64.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/wdt.h>  

SoftwareSerial esp8266Serial = SoftwareSerial(10 , 11);
//rx,tx

Parladuino parla = Parladuino(esp8266Serial);

void setup()

{
  wdt_disable();  
  
  pinMode(3, OUTPUT); 

  randomSeed(analogRead(0));

  Serial.begin(115200);

  esp8266Serial.begin(9600);

  parla.initialize();
  
  wdt_enable(WDTO_4S); 
  
}

ParladuinoMessage message = ParladuinoMessage ();

void loop()

{
  // do not use Delay() inside the loop
  wdt_reset();  
  
  parla.listen(message);

  if (!message.empty){  
    
    parla.apply(message);
    
    parla.reply(message);
    
  }
  
}


