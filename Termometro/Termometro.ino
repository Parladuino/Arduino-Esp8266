#include <Parladuino.h>
#include <sha1.h>
#include <Base64.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>  
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

SoftwareSerial esp8266Serial = SoftwareSerial(10 , 11);	 //rx,tx

Parladuino parla = Parladuino(esp8266Serial);

unsigned long _lastMessage;

boolean _state;

boolean _toggle;

void setup()

{
  wdt_disable();  

  sensors.begin();

  randomSeed(analogRead(0));

  Serial.begin(115200);

  esp8266Serial.begin(9600);

  parla.initialize();

  _lastMessage = millis();

  _state = false;

  _toggle = false;

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

  else {

    if (millis() - _lastMessage >3000){

      _lastMessage = millis();

      sensors.requestTemperatures();

      float temp = sensors.getTempCByIndex(0);

      int tempI = (int)temp;

      int tempD = (temp-tempI)*100;

      message.cleanDigitals();

      message.cleanAnalogs();

      message.addDigital(ParladuinoDigitalPin(tempI,"",5));

      message.addDigital(ParladuinoDigitalPin(tempD,"",6));

      message.action=PARLADUINO_ACTION_WRITE_TO_GROUP;

      message.setToGroup("MONITOR");

      parla.send(message);

      _toggle = (_state && tempI<30) || (!_state && tempI >= 30);

      if (_toggle){

        _state = !_state;

        message.cleanDigitals();

        message.cleanAnalogs();

        message.action = PARLADUINO_ACTION_WRITE_TO_ID_AND_REPLY_TO_GROUP;

        message.setToID("Nano");

        message.addDigital(ParladuinoDigitalPin(_state,"",3));

        parla.send(message);

      }

    }

  }

}



