#ifndef FUNCTIONS_H
#define  FUNCTIONS_H

#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <Wire.h>

#define LED 2

void setup_ota(void);
String GetClientId(void);
void SystemStart(void);
void GPIOInit(void);
void SystemStart(void);
void ClientIdCreation(void);

#endif  /* FUNCTIONS_H */
