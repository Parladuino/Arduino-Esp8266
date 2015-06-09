#pragma once
#ifndef _PARLADUINO_h
#define _PARLADUINO_h


#include "Arduino.h"

// IP de Parladuino
#define PARLADUINO_IP "AT+CIPSTART=\"TCP\",\"www.Mind-Tech.com.ar\",80"

// PARA OPTIMIZAR:

// configurar cantidad maxima pines analogicos y digitales en el mensaje
// teniendo en cuenta la reducida mamoria del arduino. 
#define PARLADUINO_ANALOGS_COUNT 3
#define PARLADUINO_DIGITALS_COUNT 3

// 256 es el maximo, si la serializazacion del mensaje es pequeña se puede reducir
#define PARLADUINO_SERIALIZE_STRING_LENGTH 256 

// no cambiar
#define PARLADUINO_DESERIALIZE_BUFFER_LENGTH 10 
#define PARLADUINO_MAX_PROPERTY_NAME_LENGTH 40

// cantidad de milisengundos entre ping y ping
#define PARLADUINO_PING_LAPSE 14000 

// quitar para no debuguear
#define DEBUGING

// acciones permitidas en el protocolo
enum ParladuinoActions {
    PARLADUINO_ACTION_NONE,
    PARLADUINO_ACTION_REPLY_TO_ID,	//hacer una pregunta al ID
    PARLADUINO_ACTION_REPLY_TO_GROUP, //hacer una pregunta al Grupo
	PARLADUINO_ACTION_WRITE_TO_ID_AND_REPLY_TO_ID, //ejecutar escritura a un ID y solicitar respuesta 
    PARLADUINO_ACTION_WRITE_TO_ID_AND_REPLY_TO_GROUP, //ejecutar escritura a un ID y solicitar respuesta para mi GRUPO 
	PARLADUINO_ACTION_WRITE_TO_GROUP_AND_REPLY_TO_ID, //ejecutar escritura a un GRUPO y solicitar respuesta
    PARLADUINO_ACTION_WRITE_TO_GROUP_AND_REPLY_TO_GROUP, //ejecutar escritura a un GRUPO y solicitar respuesta para mi GRUPO
    PARLADUINO_ACTION_WRITE_TO_ID, //ejecutar escritura a un ID 
    PARLADUINO_ACTION_WRITE_TO_GROUP, //ejecutar escritura a un GRUPO
    PARLADUINO_ACTION_RESPOND_TO_ID, //responder a un ID 
    PARLADUINO_ACTION_RESPOND_TO_GROUP //responder a un GRUPO
};

//constantes utilies para serializacion y deserealizacion JSON
const prog_char PROGMEM PARLADUINO_QUOTATION  = '\"';
const prog_char PROGMEM PARLADUINO_COLON  = ':';
const prog_char PROGMEM PARLADUINO_OPEN_CURLY  = '{';
const prog_char PROGMEM PARLADUINO_CLOSE_CURLY  = '}';
const prog_char PROGMEM PARLADUINO_OPEN_BRACKET = '[';
const prog_char PROGMEM PARLADUINO_CLOSE_BRACKET = ']';
const prog_char PROGMEM PARLADUINO_COMMA = ',';


//representa un objeto serializable 
template <byte propCount>
class ParladuinoSerializable{

public:
	ParladuinoSerializable():_buffer(),_property (0),_itsIn(false) {
	}	

	//serializa una propiedad del tipo int 
	void serializeProperty( byte prop,int value,char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]){
		char buff[PARLADUINO_MAX_PROPERTY_NAME_LENGTH];
		itoa(value,buff,10);
		writePropertyName(prop,result);
		strncat (result,buff,PARLADUINO_SERIALIZE_STRING_LENGTH);

	}

	//serializa una propiedad del tipo char*
	void serializeProperty( byte prop,char* value,char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]){
		writePropertyName(prop,result);
		strncat (result,value,PARLADUINO_SERIALIZE_STRING_LENGTH);
		result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_QUOTATION));
	}

	//serializa un propiedad del tipo array. T es la cantidad de propiedades del objeto dentro del array
	template <class T>
	void serializeArrayProperty(byte prop,T* value,int index,char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]){
		result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_QUOTATION));
		writePropertyName(prop,result);
		int a=0;
		while (a < index ) {
			value[a].serialize(result);
			if (a == index -1 )
				break ;
			result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_COMMA));
			a++;
		}
		result[strlen(result)] = (char)pgm_read_byte(&(PARLADUINO_CLOSE_BRACKET));

	};

	// deserializa un mensaje
	void deserializeObject(char &c)	
	{
		
		if (!_itsIn){
			for (byte n=0;n<propCount;n++){
				if (c == _properties(n)[_indexes[n]]){
					_indexes[n]++;
					if (_properties(n)[_indexes[n]]==(char)0)	 {
						if (_indexes[n]>PARLADUINO_MAX_PROPERTY_NAME_LENGTH)
							break;
						_property = n;
						_itsIn=true;
#ifdef DEBUGING
						Serial.print("-------->");
						Serial.println(_property);
						Serial.print("length=");
						Serial.println(_indexes[n]);
#endif
					}
				} else {
					_indexes[n] = 0;					 
				}
			}				
		} else {
			if (c != '\"' && c!=',' && c!='}' && c!=']' ){
				_buffer[strlen(_buffer)]=c;
			}
			else {
#ifdef DEBUGING
				Serial.print("<--------");
				Serial.println(_property);
#endif
				setProperty();
				memset(&_buffer, (char)0, sizeof(_buffer));

				_itsIn=false;
			}
		} 
	} ;

	// quien herede debe implemntar el metodo serialize
	virtual void serialize(char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]) = 0;
	// quien herede debe implemntar el metodo deserialize
	virtual void deserialize(char &c) = 0;

protected :

	//                Para la deserializacion 
	// los dos metodos siguientes deben ser implementados por la clase que herede

	// devuelve el nombre de la propiedad que se esta deserializando
	virtual char* _properties(byte index);
	// asigna un valor a la propiedad que se esta deserializando
	virtual void setProperty() = 0;

	// variables necesarias para la deserializacion
	byte _indexes[propCount];
	char _buffer[PARLADUINO_DESERIALIZE_BUFFER_LENGTH];
	bool _itsIn;
	byte _property ;
	
	
	//escribe el nombre de la propiedad en la serializacion
	void writePropertyName(byte index,char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]){
		byte p=0;
		while (_properties(index)[p] != (char)0)
			result[strlen(result)] = _properties(index)[p++] ;
	}

};


//Representa un pin serializable con tres propiedades (segun protocolo)
class  ParladuinoGenericPin:public ParladuinoSerializable<3>{
public:
	ParladuinoGenericPin();
	ParladuinoGenericPin(unsigned int pValue,char* pName,int8_t pPin);

	void setName(char* value) ;
	void setPin(char* value) ;
	void setValue(char* value) ;

	int value;
	char name[5];
	int8_t pin;


	void serialize(char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]);
	void deserialize(char &c);
	char* _properties(byte index);
	void setProperty();
protected:
	void init(unsigned int pValue,char* pName,int8_t pPin);

};


//Representa un pin analogo serializable
class  ParladuinoAnalogPin:public ParladuinoGenericPin{
public:
	ParladuinoAnalogPin():ParladuinoGenericPin(){}; 
	ParladuinoAnalogPin(unsigned int pValue,char* pName,int8_t  pPin):ParladuinoGenericPin(pValue,pName,pPin){}; 
};

//Representa un pin digital serializable
class  ParladuinoDigitalPin:public ParladuinoGenericPin{
public:
	ParladuinoDigitalPin():ParladuinoGenericPin(){}; 
	ParladuinoDigitalPin(unsigned int pValue,char* pName,int8_t pPin):ParladuinoGenericPin(pValue,pName,pPin){}; 
}; 

//Representa un mensaje serializable con siete propiedades (segun protocolo)
class ParladuinoMessage:public ParladuinoSerializable<7>{
public:
	ParladuinoMessage(); 
	ParladuinoMessage(char* pToID,char* pToGroup);

	void setFromID(char* value);
	void setFromGroup(char* value);
	void setToID(char* value);
	void setToGroup(char* value);
	void setAction(char* value);


	char fromID[9];
	char fromGroup[9];
	char toID[9];
	char toGroup[9];
	uint8_t action;

	ParladuinoAnalogPin analogs[PARLADUINO_ANALOGS_COUNT];
	ParladuinoDigitalPin digitals[PARLADUINO_DIGITALS_COUNT];


	int addDigital(ParladuinoDigitalPin digital){
		digitals[_digitalIndex] = digital;
		_digitalIndex++;
	};

	int addAnalog(ParladuinoAnalogPin analog){
		analogs[_analogIndex] = analog;
		_analogIndex++;
	};

	void serialize(char (&result)[PARLADUINO_SERIALIZE_STRING_LENGTH]);
	void deserialize(char &c);
	char* _properties(byte index);
	void setProperty();
	bool empty;

protected:
	int _analogIndex;
	int _digitalIndex;
	void init(char* pToID,char* pToGroup);

};

//Clase principal. 
//conecta con Parladuino
//mantiene la conexion activa
//envia mensajes.
//escucha e interpreta mensajes
//aplica las ordenes que vienen en el mensaje
//responde al emisor del mensaje si es necesario

class Parladuino {
public:
	Parladuino(Stream& serial):_lastPing (millis()) {_esp8266=&serial;};
	void initialize(char* ssid,char* pass,char* user,char* password,char* group,char* id);
	void initialize();
	bool joinAP();
	bool startClient();
	bool doHandShake();
	bool send(ParladuinoMessage &message);
	bool send(char (&data)[PARLADUINO_SERIALIZE_STRING_LENGTH]);
	Stream* _esp8266 ;
	//char* buildMessage (ParladuinoMessage &message);
	void listen(ParladuinoMessage &message);
	void apply(ParladuinoMessage &message);
	void reply(ParladuinoMessage &message);
	bool sendPing();
	void cleanBuffer();
protected:

	char readOver(unsigned int timeSpam);
	bool findOver(const __FlashStringHelper* target,unsigned int timeSpam);
	char* _SSID ;
	char* _PASS ;
	char* _USER ;
	char* _PASSWORD;
	char* _GROUP;
	char* _ID;

	unsigned long _lastPing;
};

#endif

