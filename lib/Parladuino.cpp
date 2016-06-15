#include "Parladuino.h"
#include "sha1.h"
#include "Base64.h"
#include <EEPROM.h>

//protocolo en formato JSON
//{"fi":"xxxxxxxx","fg":"xxxxxxxx","ti":"xxxxxxxx","tg":"xxxxxxxx","ac":###,"an":[{"p":###,"n":"xxxx","v":#####},{...}],"dg":[{"p":###,"n":"xxxx","v":#####},{...}]}


// Constantes necesarias para la serializacion y deserializacion de los mensajes

// Constantes para Pin
const char PROGMEM PARLADUINO_PIN[]  = "\"p\":";
const char PROGMEM PARLADUINO_NAME[]  = "\"n\":\"";
const char PROGMEM PARLADUINO_VALUE[]  = "\"v\":";

// Constantes para Mesnajes
const char PROGMEM PARLADUINO_FROM_ID[]  = "\"fi\":\"";
const char PROGMEM PARLADUINO_FROM_GROUP[]  = "\"fg\":\"";
const char PROGMEM PARLADUINO_TO_ID[]  = "\"ti\":\"";
const char PROGMEM PARLADUINO_TO_GROUP[]  = "\"tg\":\"";
const char PROGMEM PARLADUINO_ACTION[]  = "\"ac\":";
const char PROGMEM PARLADUINO_ANALOGS[]  = "an\":[";
const char PROGMEM PARLADUINO_DIGITALS[]  = "dg\":[";


const byte parladuinoEEPROMSettings[]={
	PARLADUINO_SSID,
	PARLADUINO_WIFIPASS,
	PARLADUINO_USER,
	PARLADUINO_PASS,
	PARLADUINO_ID,
	PARLADUINO_GROUP
};


// estructura que representa un pin 
const char* const pinStruct[] PROGMEM =
{
	PARLADUINO_PIN,
	PARLADUINO_NAME,
	PARLADUINO_VALUE
};

// estructura que representa un mensaje
const  char* const  messageStruct[] PROGMEM =
{

	PARLADUINO_FROM_ID,
	PARLADUINO_FROM_GROUP,
	PARLADUINO_TO_ID,
	PARLADUINO_TO_GROUP,
	PARLADUINO_ACTION,
	PARLADUINO_ANALOGS,
	PARLADUINO_DIGITALS
};




//========================================================================
//                                   PIN 

//constructor
ParladuinoGenericPin::ParladuinoGenericPin() :value(0), name(), pin(-1) {

	init(0, char(0), -1);
}
//sobrecarga constructor
ParladuinoGenericPin::ParladuinoGenericPin(unsigned int pValue, char* pName, int8_t pPin) {
	init(pValue, pName, pPin);
};
//inicializado 
void ParladuinoGenericPin::init(unsigned int pValue, char* pName, int8_t pPin) {
	value = pValue;
	setName(pName);
	pin = pPin;


};

// funciones de asignacion de propiedades
void ParladuinoGenericPin::setName(char pValue[15]) { strncpy(name, pValue, 14); name[14] = (char)0; }
void ParladuinoGenericPin::setPin(char pValue[15]) { pin = atoi(pValue); }
void ParladuinoGenericPin::setValue(char pValue[15]) { value = atoi(pValue); }

// serializa y va completando el resultado en "result"
void  ParladuinoGenericPin::serialize(char(&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]) {
	result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_OPEN_CURLY));
	serializeProperty(0, pin, result);
	result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_COMMA));
	serializeProperty(1, name, result);
	result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_COMMA));
	serializeProperty(2, value, result);
	result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_CLOSE_CURLY));
}

//deserializa utilizando metodo deserialize de la clase base.
void  ParladuinoGenericPin::deserialize(char &c) {
	deserializeObject(c);
}
// metodo obligatorio para la asignacion de valor a las propiedades en la deserializacion
void ParladuinoGenericPin::setProperty() {

	switch (_property) {
	case 0:
		setPin(_buffer);
		break;
	case 1:
		setName(_buffer);
		break;
	case 2:
		setValue(_buffer);
		break;
	}
}

//funcion obligatoria que devuelve nombre de propiedad que se esta deserializando
char* ParladuinoGenericPin::_properties(byte index) {
	char b[PARLADUINO_MAX_PROPERTY_NAME_LENGTH] = {};
	return strncpy_P(b, (char*)pgm_read_word(&(pinStruct[index])), PARLADUINO_MAX_PROPERTY_NAME_LENGTH);
};



//========================================================================
//                                    MENSAJE


//constructor
ParladuinoMessage::ParladuinoMessage() :fromID(), fromGroup(), toID(), toGroup(), action(0) {
	init(char(0), char(0));
}

//sobrecarga del constructor
ParladuinoMessage::ParladuinoMessage(char* pToID, char* pToGroup) : fromID(), fromGroup(), action(0) {
	init(pToID, pToGroup);
};

//inicializacion
void ParladuinoMessage::init(char* pToID, char* pToGroup) {
	_digitalIndex = 0;
	_analogIndex = 0;

	empty = true;
	setToID(pToID);
	setToGroup(pToGroup);
}

// funciones de asignacion de propiedades
void  ParladuinoMessage::setToID(char value[9]) { strncpy(toID, value, 8); toID[8] = (char)0; }
void  ParladuinoMessage::setToGroup(char value[9]) { strncpy(toGroup, value, 8); toGroup[8] = (char)0; }
void  ParladuinoMessage::setFromID(char value[9]) { strncpy(fromID, value, 8); fromID[8] = (char)0; }
void  ParladuinoMessage::setFromGroup(char value[9]) { strncpy(fromGroup, value, 8); fromGroup[8] = (char)0; }
void  ParladuinoMessage::setAction(char pValue[9]) { action = atoi(pValue); }

// serializa y va completando el resultado en "result"
void ParladuinoMessage::serialize(char(&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]) {

	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_OPEN_CURLY));
	serializeProperty(0, fromID, result);


	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeProperty(1, fromGroup, result);


	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeProperty(2, toID, result);


	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeProperty(3, toGroup, result);

	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeProperty(4, action, result);

	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeArrayProperty(5, analogs, _analogIndex, result);


	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_COMMA));
	serializeArrayProperty(6, digitals, _digitalIndex, result);


	result[strlen(result)] = (char)pgm_read_word(&(PARLADUINO_CLOSE_CURLY));


}

//deserializa utilizando metodo deserialize de la clase base.
void ParladuinoMessage::deserialize(char &c) {
#ifdef DEBUGING
	Serial.println();
#endif

	switch (_property) {
	case 5:
		//analogs
		if (c == PARLADUINO_CLOSE_BRACKET) {
#ifdef DEBUGING
			Serial.print("<--------");
			Serial.println(_property);
#endif
			_itsIn = false;
			_property++;
			break;
		} //end analogs
		if (c == PARLADUINO_OPEN_CURLY) {
			//new analog
			addAnalog(ParladuinoAnalogPin());
		}

		analogs[_analogIndex - 1].deserialize(c);
		break;
	case 6:
		//digitals
		if (c == PARLADUINO_CLOSE_BRACKET) {
#ifdef DEBUGING
			Serial.print("<--------");
			Serial.println(_property);
#endif
			_itsIn = false;
			_property++;
			break;
		} //end digitals
		if (c == PARLADUINO_OPEN_CURLY) { //new digital
			addDigital(ParladuinoDigitalPin());
		}
		digitals[_digitalIndex - 1].deserialize(c);
		break;
	case 7:
#ifdef DEBUGING
		Serial.print("<--------");
		Serial.println(_property);
#endif
		empty = false;
		break;
	default: // this
		deserializeObject(c);
	}
}

// metodo obligatorio para la asignacion de valor a las propiedades en la deserializacion
void ParladuinoMessage::setProperty() {
	switch (_property) {
	case 0:
		setFromID(_buffer);
		break;
	case 1:
		setFromGroup(_buffer);
		break;
	case 2:
		setToID(_buffer);
		break;
	case 3:
		setToGroup(_buffer);
		break;
	case 4:
		setAction(_buffer);
		break;
	case 5:
		break;
	case 6:
		break;
	}
}

// funcion obligatoria que devuelve nombre de propiedad que se esta deserializando
char* ParladuinoMessage::_properties(byte index) {
	char b[PARLADUINO_MAX_PROPERTY_NAME_LENGTH] = {};
	return strncpy_P(b, (char*)pgm_read_word(&(messageStruct[index])), PARLADUINO_MAX_PROPERTY_NAME_LENGTH);
};


//=======================================================================================
//                                    PARLADUINO







//Intenta conectar con wifi y hacer el handshake con parladuino
void Parladuino::initialize() {

	//Codigo para inicializar la EEPROM
	/*EEPROM.write(PARLADUINO_SSID,' ');
	EEPROM.write(PARLADUINO_SSID+1,0);

	EEPROM.write(PARLADUINO_WIFIPASS,' ');
	EEPROM.write(PARLADUINO_WIFIPASS+1,0);

	EEPROM.write(PARLADUINO_USER ,' ');
	EEPROM.write(PARLADUINO_USER+1 ,0);

	EEPROM.write(PARLADUINO_PASS,' ') ;
	EEPROM.write(PARLADUINO_PASS+1,0) ;

	EEPROM.write(PARLADUINO_ID,' ');
	EEPROM.write(PARLADUINO_ID+1,0);

	EEPROM.write(PARLADUINO_GROUP,' ');
	EEPROM.write(PARLADUINO_GROUP,0);*/
	/////////////////////////////////////
	


	_esp8266->println(F("AT+GMR"));
	if (!findOver(F("OK"), 1000)) {};
	//mode 3 tanto cliente como servidor


	/*_esp8266->println(F("AT+UART=9600,8,1,0,0"));
	_esp8266->println(F("AT+CWMODE=3"));
	if (!findOver(F("OK"), 1000)) {};
	_esp8266->println(F("AT+CWSAP=\"ESPTEST\",\"1234567890\",1,3"));
	if (!findOver(F("OK"), 20000)) {}*/

	_esp8266->println(F("AT+CWSAP?"));
	if (!findOver(F("OK"), 2000)) {};
	_esp8266->println(F("AT+CIFSR"));
	if (!findOver(F("OK"), 2000)) {};

	setAsServer();
	//_esp8266->println(F("AT+CWJAP=\"APTI\",\"Autonuevo2014\""));
	//if (!findOver(F("OK"), 15000)) {};
	/*_esp8266->print(F("AT+CWJAP=\""));
	_esp8266->print(_SSID);
	_esp8266->print(F("\",\""));
	_esp8266->print(_PASS);
	_esp8266->println(F("\""));

	if (!findOver(F("OK"), 15000)) {}*/

	_esp8266->println(F("AT+CWJAP?"));
	if (findOver(F("No AP"), 2000)) {
		
		if (joinAP()) {
			
			doHandShake();
		}
	}
	else {
		
		doHandShake();
	}
}

// set confugura como servidor
void Parladuino::setAsServer() {
	_esp8266->println(F("AT+CIPMUX=1"));
	findOver(F("OK"), 2000);
	_esp8266->println(F("AT+CIPSERVER=1,80"));
	findOver(F("OK"), 2000);
	_esp8266->println(F("AT+CIPSTO=10"));
	findOver(F("OK"), 2000);
}

//conecta con acces point 
bool Parladuino::joinAP() {

	//mode 3 tanto cliente como servidor
	_esp8266->println(F("AT+CWMODE=3"));
	if (!findOver(F("OK"), 1000))
		return false;

	//Reset de la placa hasta obtener ready
	_esp8266->println(F("AT+RST"));
	if (!findOver(F("ready"), 5000))
		return false;

	// vuelve a setearse como servidor despues del reset
	setAsServer();

	//se une al acces point
	_esp8266->print(F("AT+CWJAP=\""));
	printFromEEPROM(PARLADUINO_SSID);
	_esp8266->print(F("\",\""));

	printFromEEPROM(PARLADUINO_WIFIPASS);
	_esp8266->println(F("\""));

	if (!findOver(F("OK"), 15000)) {
		return false;
	}

	return true;
}



// Handshake con el websocket de Parladuino
bool Parladuino::doHandShake() {

	// conecta con Parladuino
	_esp8266->println(F(PARLADUINO_IP));
	if (!findOver(F("OK"), 3000)) {}
	//return false;


	// busca una credencial en Parladuino
	char credential[80] = {};
	_esp8266->print(F("AT+CIPSEND=1,"));
#ifdef TESTING
	_esp8266->println(30);
#else
	_esp8266->println(26);
#endif
	if (findOver(F(">"), 1000)) {
#ifdef TESTING
		_esp8266->print(F("GET /ParlaSocketTest/api/Key"));
#else
		_esp8266->print(F("GET /ParlaSocket/api/Key"));
#endif
		_esp8266->println("");
		if (findOver(F(":\""), 1000)) {
			// carga la credencial
			int cc;
			while ((cc = readOver(100)) >= 0) {
				if (cc == '\"')
					break;
				
				credential[strlen(credential)] = cc;
			}
			// concatena credencial con paswword
			//strcat(credential, _PASSWORD);

			concatFromEEPROM(credential,PARLADUINO_PASS);


		}
		else {
			return false;
		}


	}
	else {
		return false;
	}

	// si se cerro la conexion la vuelve a abrir
	if (findOver(F("CLOSED"), 1000)) {
		_esp8266->println(F(PARLADUINO_IP));
		findOver(F("OK"), 5000);
	}

	//hace el sha1 de la credencial+password
	uint8_t *hash;
	char credResult[41] = {};
	Sha1.init();
	Sha1.print(credential);
	hash = Sha1.result();

	int h = 0;
	for (int i = 0; i < 40; i = i + 2) {
		int b = hash[h] % 16;
		int a = (hash[h] / 16);

		a = a + 48;
		if (a > 57)
			a = a + 39;
		b = b + 48;
		if (b > 57)
			b = b + 39;
		credResult[i] = (char)a;
		credResult[i + 1] = (char)b;
		h++;
	}
	credResult[40] = '\0';

	// calculo la WebSocket-key 
	//-------------------------

	// creo una key con la funcion random
	char keyStart[17];
	char b64Key[25];
	char key[61] = "------------------------";

	for (int i = 0; i < 16; ++i) {
		keyStart[i] = (char)random(1, 256);
	}

	//encripto en base64
	base64_encode(b64Key, keyStart, 16);

	//me quedo con los primeros 24 caracteres
	for (int i = 0; i < 24; ++i) {
		key[i] = b64Key[i];
	}

	_esp8266->print(F("AT+CIPSEND=1,"));
#ifdef TESTING
	_esp8266->println(280 +  lengthFromEEPROM(PARLADUINO_GROUP) + lengthFromEEPROM(PARLADUINO_ID)  + lengthFromEEPROM(PARLADUINO_USER)+ strlen(credResult));
#else
	_esp8266->println(276 +  lengthFromEEPROM(PARLADUINO_GROUP) + lengthFromEEPROM(PARLADUINO_ID)  + lengthFromEEPROM(PARLADUINO_USER)+ strlen(credResult));

#endif
	if (findOver(F(">"), 20)) {
#ifdef TESTING
		_esp8266->print(F("GET /ParlaSocketTest/api/Ws?user="));
#else
		_esp8266->print(F("GET /ParlaSocket/api/Ws?user="));
#endif
		//_esp8266->print(_USER);
		printFromEEPROM(PARLADUINO_USER);
		_esp8266->print(F("&pass="));
		_esp8266->print(credResult);
		_esp8266->print(F("&group="));
		//_esp8266->print(_GROUP);
		printFromEEPROM(PARLADUINO_GROUP);
		_esp8266->print(F("&ID="));
		//_esp8266->print(_ID);
		printFromEEPROM(PARLADUINO_ID);
		_esp8266->print(F(" HTTP/1.1"));
		_esp8266->print(F("\r\nHost:190.190.178.205"));
		_esp8266->print(F("\r\nOrigin: Arduino"));
		_esp8266->print(F("\r\nUpgrade:websocket"));
		_esp8266->print(F("\r\nConnection:Upgrade"));
		_esp8266->print(F("\r\nSec-WebSocket-Extensions:permessage-deflate; client_max_window_bits"));
		_esp8266->print(F("\r\nSec-WebSocket-Key:"));
		_esp8266->print(key);
		_esp8266->print(F("\r\nSec-WebSocket-Version:13"));
		_esp8266->println(F("\r\n\r\n"));

		if (findOver(F("OK"), 1000)) {
			char serverKey[30] = {};
			int bite;
			// tomo  "Sec-WebSocket-Accept: " de la respuesta
			if (findOver(F("Sec-WebSocket-Accept: "), 2000)) {
				while ((bite = readOver(500)) >= 0) {
					if (bite == '\n')
						break;
					serverKey[strlen(serverKey)] = bite;
				}

				// concateno la key que envie con 258EAFA5-E914-47DA-95CA-C5AB0DC85B11
				// que es lo mismo que hace el servidor
				strcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

				uint8_t *hash;
				char result[21];
				char b64Result[30];

				//obtengo el sha1 de la key
				Sha1.init();
				Sha1.print(key);
				hash = Sha1.result();

				for (int i = 0; i < 20; ++i) {
					result[i] = (char)hash[i];
				}
				result[20] = '\0';

				// obtengo el base 64 
				base64_encode(b64Result, result, 20);

				_lastPing = millis();

				// si coinciden la key del servidor con la key calculada !! todo bien !!
				return (memcmp(&serverKey, &b64Result, 28) == 0);

			}
		}
	}

	return false;

}

void Parladuino::printFromEEPROM(int addr) {
	byte c ;
	while ((c= EEPROM.read(addr)) > 0){
		
		_esp8266->print(char(c));
		addr++;
	}
	
}

byte Parladuino::lengthFromEEPROM(int from) {
	int addr = from;
	while (EEPROM.read(addr) > 0){
		addr++;
	}
	return (addr-from);
}

char* Parladuino::concatFromEEPROM(char* text, int addr) {
	byte c ;
	while ((c= EEPROM.read(addr)) > 0){
		
		text[strlen(text)] = c;
		addr++;
	}
	text[strlen(text)] = char(0);
	
	return text;
}

// metodo para limpiar el buffer del wifi
void Parladuino::cleanBuffer() {
	while (_esp8266->read() != -1) {}
}

//busca una palabra en la cola del buffer del wifi
bool Parladuino::findOver(const __FlashStringHelper* target, unsigned int timeSpam) {
	PGM_P p = reinterpret_cast<PGM_P>(target);
	size_t index = 0;
	int c;
	listening=true;
	while ((c = readOver(timeSpam)) >= 0) {

		if (c == pgm_read_byte(p + index)) {
			index++;
			if (pgm_read_byte(p + index) == (char)0){
				listening=false;
				return true;}
		}
		else
			index = 0;
	}
	listening=false;
	return false;
}

// lee el buffer 
// retorna el proximo carater de la cola
// retorna -1 si el tiempo limite de espera expira
char Parladuino::readOver(unsigned int timeSpam) {

	unsigned long timeLimit = millis() + timeSpam;
	char r;
	while (millis() <= timeLimit) {
		r = _esp8266->read();

		if (r > -1) {

			if (_urlDecode){
				if(r=='%'){
#ifdef DEBUGING

					Serial.write(r);
#endif
					r = char(decodeHex(_esp8266->read())*16+decodeHex(_esp8266->read()));
				} 
			}
#ifdef DEBUGING

			Serial.write(r);
#endif

			return  r;
		}

	}
	return -1;
}


// envia un mensaje serializado a parladuino
bool Parladuino::send(char(&data)[PARLADUINO_SERIALIZE_STRING_LENGTH]) {
	bool dispatched = false;

	uint8_t fin = 129;
	uint8_t mask = 128; //0x80;
	uint8_t key[4] = {
		(uint8_t)random(0, 255),
		(uint8_t)random(0, 255),
		(uint8_t)random(0, 255),
		(uint8_t)random(0, 255) 
	}
	;
	int byteLengthSize = 1;
	uint8_t byteLength[8];

	// preparo parte de la cabecera segun largo del mensaje
	if (strlen(data) <= 125) {
		byteLength[0] = mask + strlen(data);
	}
	else {
		if (strlen(data) >= 126 && strlen(data) <= 65535) {
			byteLengthSize = 3;
			byteLength[0] = mask + 126;
			byteLength[1] = (strlen(data) >> 8) & 255;
			byteLength[2] = (strlen(data)) & 255;
		}
		else {
			//mensaje muy largo
			return false;
		}
	}


	//envio mensaje
	_esp8266->print(F("AT+CIPSEND=1,"));
	_esp8266->println(5 + byteLengthSize + strlen(data));
	if (findOver(F(">"), 20)) {

		//Armo y envio cabecera
		_esp8266->print((char)fin);
		for (int i = 0; i < byteLengthSize; i++) {
			_esp8266->print((char)byteLength[i]);
		}
		_esp8266->print((char)key[0]);
		_esp8266->print((char)key[1]);
		_esp8266->print((char)key[2]);
		_esp8266->print((char)key[3]);

		//encripto y envio cuerpo del mensaje
		for (long i = 0; i < strlen(data); i++) {
			//obtengo la key para el siguienta caracter 
			byte j = i % 4;
			//hago el xor del caracter con la key correspondiente
			byte tt = data[i] ^ key[j];
			_esp8266->print((char)tt);
		}
		_lastPing = millis();
		dispatched = true;

		if (findOver(F("ERROR"), 20))
			dispatched = false;


	}
	return dispatched;
}

//Envia un mensaje a Parladuino
bool Parladuino::send(ParladuinoMessage &message) {
	char buff[PARLADUINO_SERIALIZE_STRING_LENGTH] = {};
	//transforma el mensaje en un array de caracteres
	message.serialize(buff);
#ifdef DEBUGING
	Serial.println(buff);
#endif
	return send(buff);
}

//verifica si hay conexion cada tantos milisegundos
bool Parladuino::sendPing() {
	bool dispatched = true;
	if ((millis() - _lastPing) > PARLADUINO_PING_LAPSE) {
		_lastPing = millis();
		//		_esp8266->println(F("AT+CWLIF"));
		//		if (!findOver(F("192."), 20)) {
		//
		//			_isSomeOne=false;
		//
		//#ifdef DEBUGING
		//			Serial.println("Modo web");
		//#endif

		_esp8266->print(F("AT+CIPSEND=1,"));
		_esp8266->println(7);
		if (!findOver(F("ERROR"), 200)) {
			_esp8266->print((char)(uint8_t)0x8a);
			_esp8266->print((char)(uint8_t)0x81);
			_esp8266->print((char)(uint8_t)0x37);
			_esp8266->print((char)(uint8_t)0x00);
			_esp8266->print((char)(uint8_t)0x00);
			_esp8266->print((char)(uint8_t)0x00);
			_esp8266->print((char)(uint8_t)0x7f);
		}
		else {

			_esp8266->println(F("AT+CWJAP?"));
			if (findOver(F("No AP"), 200)) {
				//no hay wifi. espera mensaje de cambio de acces point
				setAsServer();
				dispatched = true;
			}
			else {
				//hay wifi pero no internet
				dispatched = false;
			}

		}
		/*} else {
		_isSomeOne= true;;
		#ifdef DEBUGING
		Serial.println("Modo sofAP");
		#endif
		}*/

	}
	return dispatched;
}

// aplica el mensaje segun la accion
void Parladuino::apply(ParladuinoMessage &message) {

	if (message.action == PARLADUINO_ACTION_WRITE_SETTINGS) {
		_writeSettings=true;

	}


	byte i = 0;
	//trabajo sobre los pines analogos
	while (message.analogs[i].pin != -1) {
		// cualquier accion que implique escribir un pin
		if (message.action > PARLADUINO_ACTION_REPLY_TO_GROUP && message.action < PARLADUINO_ACTION_RESPOND_TO_ID) {
			analogWrite(message.analogs[i].pin, message.analogs[i].value);
#ifdef DEBUGING
			Serial.print("Pin");
			Serial.print(message.analogs[i].pin);
			Serial.print("=");
			Serial.println(message.analogs[i].value);
#endif
		}
		//actualizo el mensaje con el valor real del pin
		message.analogs[i].value = analogRead(message.analogs[i].pin);
		i++;
	}

	i = 0;
	//trabajo sobre los pines digitales
	while (message.digitals[i].pin != -1) {
		// cualquier accion que implique escribir un pin
		if (message.action > PARLADUINO_ACTION_REPLY_TO_GROUP && message.action < PARLADUINO_ACTION_RESPOND_TO_ID) {
			digitalWrite(message.digitals[i].pin, message.digitals[i].value);
#ifdef DEBUGING
			Serial.print("Pin");
			Serial.print(message.digitals[i].pin);
			Serial.print("=");
			Serial.println(message.digitals[i].value);
#endif

		}
		//actualizo el mensaje con el valor real del pin
		message.digitals[i].value = digitalRead(message.digitals[i].pin);
		i++;
	}

}


//chequea el buffer de wifi para ver si entra algun mensaje
void Parladuino::listen(ParladuinoMessage &message) {

	if (_esp8266->available()) {
		
		// si llega algun dato intenta crear un objeto mensaje
		message = ParladuinoMessage();
		
		if (!_writeSettings){
			if (findOver(F("+IPD,"),20)){
				if (readOver(20) != 49 ){ // si el ID de la conexion es distinto de 1 entonces decodifica la url
					_urlDecode = true;
#ifdef DEBUGING
					Serial.println("Url Decoding");
#endif
				}
			}
		}


		while (_esp8266->available() && message.empty) {
			char c = readOver(20);
			if (!_writeSettings){
				message.deserialize(c);
			} else {

				if (findOver(F("m="),20)){
#ifdef DEBUGING
					Serial.println("Writing EEPROM");

#endif


					byte index = 0;
					int addr = parladuinoEEPROMSettings[index];
					while(index<sizeof(parladuinoEEPROMSettings)){
						while(byte (c=readOver(20)) != 27){
							EEPROM.write(addr,c);
							addr++;
						}
						EEPROM.write(addr,0);
						index++;
						addr = parladuinoEEPROMSettings[index];
					}
				} 

				_esp8266->println(F("AT+CIPSEND=0,98"));
				if (findOver(F(">"), 1000)) {
					_esp8266->print(F("HTTP/1.1 200 OK"));
					//_esp8266->print(F("\r\nContent-Type: text/html; charset=UTF-8")); 
					_esp8266->print(F("\r\nAccess-Control-Allow-Origin: *"));
					_esp8266->print(F("\r\nContent-Length: 9"));
					_esp8266->print(F("\r\nConnection: close"));
					_esp8266->print(F("\r\n\r\n"));
					_esp8266->print(F("Todo OK\r\n"));
				}

				if (findOver(F("SEND OK"), 10000)) {}
				_urlDecode = false;
				/*_esp8266->println(F("AT+CIPCLOSE=0"));*/
				/*if (findOver(F("SEND OK"), 10000)) {}*/
				_esp8266->println(F("AT+CWQAP"));
				if (findOver(F("OK"), 1000)) {};
				initialize();
				_writeSettings=false;

			}

		}
		_lastPing = millis();
	}

	else {
		//envia ping para chequear la conexion
		//si no hay respuesta la inicializa

		
		message.empty = true;

		if (!sendPing()) {
			initialize();
		};
	}

}

byte Parladuino::decodeHex(char c){
	if (c>='0' && c<='9'){return (c-'0');}
	if (c>='A' && c<='F'){return 10+ (c-'A');}
}



//responde al remitente
void Parladuino::reply(ParladuinoMessage &message) {

	// si el mensaje viene con alguna accion que ordena respuesta
	if (message.action > PARLADUINO_ACTION_NONE && message.action < PARLADUINO_ACTION_WRITE_TO_ID) {
		message.setToID(message.fromID);
		message.setToGroup(char(0));
		char id[8]={};
		concatFromEEPROM(id,PARLADUINO_ID);
		message.setFromID(id);

		// si se debe responder al Id o al grupo
		if (message.action == PARLADUINO_ACTION_REPLY_TO_GROUP || message.action == PARLADUINO_ACTION_WRITE_TO_ID_AND_REPLY_TO_GROUP || message.action == PARLADUINO_ACTION_WRITE_TO_GROUP_AND_REPLY_TO_GROUP) {
			message.setToGroup(message.fromGroup);
			message.action = PARLADUINO_ACTION_RESPOND_TO_GROUP;
		}
		else {
			message.action = PARLADUINO_ACTION_RESPOND_TO_ID;
		}

		//envia la respuesta
		if (!send(message)) {//TODO: quï¿½ hacer en este caso de no poder enviar respuesta
		};
	}
}



