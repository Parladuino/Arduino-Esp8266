#include "Parladuino.h"

#include <SoftwareSerial.h>

#define SSID "My WiFi ID"

#define PASS "My WiFi Password"

#define PARLADUINO_USER "My Parladuino Public key"

#define PARLADUINO_PASSWORD "My Parladuino Private key"

#define PARLADUINO_ID "Device ID" // 8 char

#define PARLADUINO_GROUP "Device Group" // 8 char

SoftwareSerial esp8266Serial = SoftwareSerial(10 , 11);	 //rx,tx

Parladuino parla = Parladuino(esp8266Serial);

void setup()

{

	pinMode(13, OUTPUT);
		
	randomSeed(analogRead(0));

	Serial.begin(115200);

	esp8266Serial.begin(9600);

	parla.initialize(SSID,PASS,PARLADUINO_USER,PARLADUINO_PASSWORD,PARLADUINO_GROUP,PARLADUINO_ID);
	
}

ParladuinoMessage message = ParladuinoMessage ();

void loop()

{
    // do not use Delay() inside the loop
	
	parla.listen(message);

	if (!message.empty){	

		parla.apply(message);

		parla.reply(message);	

	}

}


