#include "FileSystem.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "Define.h"
#include "Functions.h"

const char *WiFifilename = "/WiFiconfig.json";  // <- SD library uses 8.3 filenames
const char *MQTTfilename = "/MQTTconfig.json";  // <- SD library uses 8.3 filenames
const char *MQTTTopicsfilename = "/MQTTcTopics.json";  // <- SD library uses 8.3 filenames

void listFilesInDir(File dir, int numTabs);

bool DefaultsLoaded = 0;

char FileSystemInit(struct WiFiConfig* WFC,struct MQTTConfig* MQC){
  if(SPIFFS.begin(true)){
    File dir = SPIFFS.open("/");
    listFilesInDir(dir, 1);

    SPIFFS.format();
    LOG(" Config Check \r"); 
    // Serial.println(F(" Config Check   ")); 
    WifiComfig(WFC);
    MqttComfig(MQC);
    PrintWiFiConfigStruct(WFC);
    return 1;
  }
  else{
    LOG("File Sytem Failed to mount!!!!!! Reformat!\r");
    SPIFFS.format();
    return 0;
  }
}

void WifiComfig(struct WiFiConfig* WFC){
  if(!SPIFFS.exists(WiFifilename)){
    //Config Doc dson't exist, wite one!
    LOG("No Wifi Config! Save One!....");
    WifisaveConfiguration(WFC);
    LOG("Now Load Config! \r");            
    WifiloadConfiguration(WFC);
    delay(100);
    PrintWiFiConfigStruct(WFC);
    delay(100);
  }
  else{
    LOG("Found File.....Load Config!\r");
    WifiloadConfiguration(WFC);
    delay(100);
    PrintWiFiConfigStruct(WFC);
    delay(100);
  }
}

void MqttComfig(struct MQTTConfig* MQC){
  if(!SPIFFS.exists(MQTTfilename)){
    //Config Doc dson't exist, wite one!
    LOG("No MQTT Config! Save One!....");
    MqttsaveConfiguration(MQC);
    LOG("Now Load Config! \r");            
    MqttloadConfiguration(MQC);
    delay(100);
    PrintMqttConfigStruct(MQC);
    delay(100);
  }
  else{
    LOG("Found File.....Load Config!\r");
    MqttloadConfiguration(MQC);
    delay(100);
    PrintMqttConfigStruct(MQC);
    delay(100);
  }
}

// Loads the configuration from a file
void WifiloadConfiguration(struct WiFiConfig* WFC) {
  // Open file for reading
  File file = SPIFFS.open(WiFifilename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    LOG("WiFi - File Read Error, Rebuilding file from defults ****Rebooting****\r");
    SPIFFS.remove(WiFifilename);
    delay(1000);
    ESP.restart(); //Reboot the device and load defaults. 
  }
  else{
    // Copy values from the JsonDocument to the Config
    strlcpy(WFC->SSID,doc["SSID"],sizeof(WFC->SSID));
    strlcpy(WFC->Passcode,doc["Passcode"],sizeof(WFC->Passcode));
    strlcpy(WFC->Host,doc["Host"],sizeof(WFC->Host));
    strlcpy(WFC->IP,doc["IP"],sizeof(WFC->IP));
    strlcpy(WFC->DefultGateway,doc["DefultGateway"],sizeof(WFC->DefultGateway));
    strlcpy(WFC->SubMask,doc["SubMask"],sizeof(WFC->SubMask));

    WFC->WIFIMode = doc["WIFIMode"];
    WFC->SSIDLN = doc["SSIDLN"];
    WFC->PswdLN = doc["PswdLN"];
    WFC->HoastLN = doc["HoastLN"];
    WFC->DHCP = doc["DHCP"];
    // Close the file (Curiously, File's destructor doesn't close the file)
    //SPIFFS.close();
    file.close();
  }
}

// Saves the configuration to a file
void WifisaveConfiguration(struct WiFiConfig* WFC) {
  // Delete old file for updating.
  SPIFFS.remove(WiFifilename);
  DefaultsLoaded = 1;
  // Open file for writing
  File file = SPIFFS.open(WiFifilename, "w");
  if (file) {
    LOG("Opened File! \r");

    StaticJsonDocument<512> doc; // Allocate a temporary JsonDocument

    /*
    * This is to save the autogenerated host name into data dictonary
    * This is ran on inital boot or file system couruption. 
    */

    strlcpy(WFC->Host,GetClientId().c_str(),strlen(GetClientId().c_str())); // copy generated string into array
    WFC->HoastLN = strlen(GetClientId().c_str()); // Save lenth

    // Set the values in the document
    doc["WIFIMode"] = WFC->WIFIMode;
    doc["SSID"] = WFC->SSID;
    doc["SSIDLN"] =  WFC->SSIDLN;
    doc["Passcode"] =  WFC->Passcode;
    doc["PswdLN"] =  WFC->PswdLN;
    doc["Host"] =  WFC->Host;
    doc["HoastLN"] =  WFC->HoastLN;
    doc["DHCP"] =  WFC->DHCP;
    doc["IP"] =  WFC->IP;
    doc["DefultGateway"] =  WFC->DefultGateway;
    doc["SubMask"] =  WFC->SubMask;
    // Serialize JSON to file
    //serializeJsonPretty(doc, Serial);
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    file.close();
  }
  else{
    LOG("File not able to be opened :(");
  }
}

void MqttloadConfiguration(struct MQTTConfig* MQC) {
  // Open file for reading
  File file = SPIFFS.open(MQTTfilename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error){
    LOG("MQTT - File Read Error, Rebuilding file from defults ****Rebooting****\r");
    SPIFFS.remove(MQTTfilename);
    delay(1000);
    ESP.restart(); //Reboot the device and load defaults. 
  }
  else{
    // Copy values from the JsonDocument to the Config
    strlcpy(MQC->MQTTIP,doc["MQTTIP"],sizeof(MQC->MQTTIP));
    strlcpy(MQC->MQTTPassword,doc["MQTTPassword"],sizeof(MQC->MQTTPassword));
    MQC->MQTTEnabble = doc["MQTTEnabble"];
    MQC->MQTTPasswordLN = doc["MQTTPasswordLN"];
    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
  }
}

void MqttsaveConfiguration(struct MQTTConfig* MQC) {
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(MQTTfilename);

  // Open file for writing
  File file = SPIFFS.open(MQTTfilename, "w");
  if (file) {
    LOG("Opened MQTT File! \r");
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<256> doc;

    doc["MQTTEnabble"] = MQC->MQTTEnabble;
    // Set the values in the document
    doc["MQTTIP"] = MQC->MQTTIP;
    doc["MQTTPassword"] = MQC->MQTTPassword;
    doc["MQTTPasswordLN"] = MQC->MQTTPasswordLN;
    // Serialize JSON to file
    //serializeJsonPretty(doc, Serial);
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write MQTT file"));
    }
    file.close();
  }
  else{
    LOG("MQTT File not able to be opened :(");
  }
}

void PrintWiFiConfigStruct(struct WiFiConfig* WFC){
  Serial.print("AccsessPoint: ");
  volatile unsigned char i = 0;
  for(i = 0; i < WFC->SSIDLN; i++){
    Serial.print(WFC->SSID[i]);
  }
  Serial.println();
  Serial.print("Passcode: ");
  for(i = 0; i < WFC->PswdLN; i++){
    Serial.print(WFC->Passcode[i]);
  }
  Serial.println();
  Serial.print("Host: ");
  for(i = 0; i < WFC->HoastLN; i++){
    Serial.print(WFC->Host[i]);
  }
  Serial.println();
  Serial.print("DHCP : "); Serial.println(WFC->DHCP);
  Serial.print("IP: ");
  for(i = 0; i < 16; i++){
    Serial.print(WFC->IP[i]);
  }
  Serial.println();
  Serial.print("DefultGateway: ");
  for(i = 0; i < 16; i++){
    Serial.print(WFC->DefultGateway[i]);
  }
  Serial.println();
  Serial.print("SubMask: ");
  for(i = 0; i < 16; i++){
    Serial.print(WFC->SubMask[i]);
  }
  Serial.println();
  Serial.println();
}

void PrintMqttConfigStruct(struct MQTTConfig* MQC){
  Serial.print("Mqtt IP: ");
  volatile unsigned char i = 0;
  for(i = 0; i < 16; i++){
    Serial.print(MQC->MQTTIP[i]);
  }
  Serial.println();
  Serial.print(" MQtt Passcode: ");
  for(i = 0; i < MQC->MQTTPasswordLN; i++){
    Serial.print(MQC->MQTTPassword[i]);
  }
  Serial.println();
  Serial.println();
}

void listFilesInDir(File dir, int numTabs) {
  Serial.println("\r Files: \r");
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files in the folder
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listFilesInDir(entry, numTabs + 1);
    } else {
      // display size for file, nothing for directory
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  Serial.println("\r");
}