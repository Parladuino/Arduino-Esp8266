# Arduino-Esp8266
Instalano esta libreria en tu Arduino conectado a internet via Esp8266, puedes conectarte a Parladuino con facilidad y comenzar a la charla entre dispositivos inmediatamente.

Si quieres conocer mas acerca de Parladuino entra en:

http://www.mind-tech.com.ar/Parladuino


Para utilizarla necesitarás :

Compilar con IDE 1.0.6 de Arduino

Flashear tu Esp8266 con la version v0.9.5.2 del Firmware 

Libreria Base64
https://github.com/adamvr/arduino-base64

Libreria Sha1
https://github.com/Cathedrow/Cryptosuite/tree/master/Sha


Ademas deberas cambiar este paso en tu libreria SoftwareSerial.h

#define _SS_MAX_RX_BUFF 64 // RX buffer size

por 

#define _SS_MAX_RX_BUFF 256 // RX buffer size



