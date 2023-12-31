#include "Functions.h"
#include "Oled.h"
#include "MQTT.h"
#include "Sensors.h"
#include "Webportal.h"
#include "Menu.h"
// Start ArduinoOTA via WiFiSettings with the same hostname and password

void setup() {
  GPIOInit();
  SystemStart();
  InitSensors();
  ClientIdCreation();
  DisplayInit();
  WiFiStart();
  //MQTTStart();
  WebStart();
  Serial.println("Setup Done!");
  DisplayCenterClear();
  Serial.println(WiFi.getHostname());
  DisplayTimeoutReset(); //This allows the display to be shown for 10 seconds afer reboot. 
}

void loop() {
  MqttLoop();
  DislaySaver();
  SyncLoop();
  WebHandel();
}
