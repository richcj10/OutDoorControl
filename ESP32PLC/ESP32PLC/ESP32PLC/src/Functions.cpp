#include "Functions.h"
#include <rom/rtc.h>
#include "Oled.h"
#include "Sensors.h"
#include "MQTT.h"
#include "Menu.h"

unsigned long lastMsg = 0;
unsigned long lastRS485 = 0;
unsigned long lastUpdate = 0;
char CPUOneResetReason = 0;
char CPUTwoResetReason = 0;
String clientId = "";
char PinLastVal = 0;

char WiFiConnected = 0;

void print_reset_reason(RESET_REASON reason);

void SystemStart(){
  Serial.begin(115200);
  Serial1.begin(115200,SERIAL_8N1,RXD2,TXD2);
  SPIFFS.begin(true);  // Will format on the first run after failing to mount
  Wire.begin(21, 22);   // sda= GPIO_21 /scl= GPIO_22 
}

void GPIOInit(){
  pinMode(USER_SW, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(MP1INPUT, INPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(DIR, HIGH);
}

void SyncLoop(){
  ArduinoOTA.handle();
  unsigned long now = millis();
  if (now - lastUpdate > 700){
    lastUpdate = now;
    digitalWrite(LED,!digitalRead(LED));
    ReadDS18B20OneWire();
    readDeviceClimate();
    DisplayManager();
    char PinValue = digitalRead(MP1INPUT);
    if(PinValue != PinLastVal){
      if(PinValue == LOW){
        SendChestPower(1);
      }
      else{
        SendChestPower(0);
      }
      PinLastVal = PinValue;
    }
    if ((WiFi.status() != WL_CONNECTED)) {
      Serial.println("Rebooting - Lost WiFi");
      delay(1000);
      ESP.restart();
    }
  }
  if (now - lastMsg > 10000){
    lastMsg = now;
    SendDeviceEnviroment();
    SendChestFreezer();
  }
  if (now - lastRS485 > 1000){
    lastRS485 = now;
    Serial1.print(0xAA,HEX);
    Serial1.print(0xFF,HEX);
  }
}

void setup_ota() {
  ArduinoOTA.setHostname(GetClientId().c_str());
  ArduinoOTA.setPassword(WiFiSettings.password.c_str());
  ArduinoOTA.begin();
}

void ConnectUpdate(){
  digitalWrite(LED,!digitalRead(LED));
  WiFiConnectAnimation();
}

void WiFiFaiure(){
  WiFiAP(1);
  WiFiConnected = 0;
}

unsigned long Countdown = 0;

void WiFiStart(void){
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(clientId.c_str());
  //WiFiSettings.hostname(clientId);
  WiFiSettings.secure = false;
  WiFiSettings.onWaitLoop = []() {ConnectUpdate(); return 100; };
  //WiFiSettings.onSuccess  = []() {  };
  WiFiSettings.onFailure  = []() { WiFiFaiure(); };
  WiFiSettings.onPortal = []() {
    setup_ota();
    WiFiStreanthDisplay(3);
    Countdown = millis();
    Countdown+=100000;
  };

  WiFiSettings.onPortalWaitLoop = []() {

    ArduinoOTA.handle();
    long TimeToReset =  (Countdown - millis());
    DisplayTimeToReset(TimeToReset);
    if(TimeToReset <= 0){
      Serial.println("Rebooting - Lost WiFi");
      delay(1000);
      ESP.restart(); 
    }
    delay(10);
  };
  
  WiFiSettings.connect();
  
  Serial.print("Password: ");
  Serial.println(WiFiSettings.password);
  WiFiStreanthDisplay(3);
  WiFiConnected = 1;
  setup_ota();  // If you also want the OTA during regular execution
  Serial.println(WiFi.localIP());
}

char GetWiFiStatus(void){
  return(WiFiConnected);
}

String GetClientId(void){
  return(clientId);
}

void ClientIdCreation(void){
  byte mac[6];
  WiFi.macAddress(mac);
  clientId = "ESPPLC-";
  clientId = clientId + String(mac[1]) + String(mac[0]);
  
  Serial.print("ClientID = ");
  Serial.println(clientId);
}

void SaveResetReason(void){
  print_reset_reason(rtc_get_reset_reason(0));
  CPUOneResetReason = rtc_get_reset_reason(0);
  print_reset_reason(rtc_get_reset_reason(1));
  CPUTwoResetReason = rtc_get_reset_reason(1);
}

char GetResetReason(char cpucore){
  if(cpucore == 1){
    return CPUOneResetReason;
  }
  else if(cpucore == 2){
    return CPUTwoResetReason;
  }
  else{
    return 255;
  }
}


void print_reset_reason(RESET_REASON reason){
  switch ( reason){
    case 1 : 
      Serial.println ("POWERON_RESET");
      break;          /**<1,  Vbat power on reset*/
    case 3 : 
      Serial.println ("SW_RESET");
      break;               /**<3,  Software reset digital core*/
    case 4 : 
      Serial.println ("OWDT_RESET");
      break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : 
      Serial.println ("DEEPSLEEP_RESET");
      break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : 
      Serial.println ("SDIO_RESET");
      break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : 
      Serial.println ("TG0WDT_SYS_RESET");
      break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : 
      Serial.println ("TG1WDT_SYS_RESET");
      break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : 
      Serial.println ("RTCWDT_SYS_RESET");
      break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : 
      Serial.println ("INTRUSION_RESET");
      break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : 
      Serial.println ("TGWDT_CPU_RESET");
      break;       /**<11, Time Group reset CPU*/
    case 12 : 
      Serial.println ("SW_CPU_RESET");
      break;          /**<12, Software reset CPU*/
    case 13 : 
      Serial.println ("RTCWDT_CPU_RESET");
      break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : 
      Serial.println ("EXT_CPU_RESET");
      break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : 
      Serial.println ("RTCWDT_BROWN_OUT_RESET");
      break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : 
      Serial.println ("RTCWDT_RTC_RESET");
      break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : 
      Serial.println ("NO_MEAN");
  }
}