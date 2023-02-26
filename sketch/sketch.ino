/*
  ESP 32 sketch for railroad modules.

  Documentation: https://github.com/krasilnikov-spb/railroad-esp32
  Author: Alexander Krasilnikov <alexander@krasilnikov.spb.ru>
*/

// Load Wi-Fi library
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include "ArduinoJson.h"

#define BUTTON_PIN_CONFIG_MODE 23 // GPIO23 pin connected to GND to enable the settings menu

// Default network credentials to config network
const char* ssid     = "RR-Config-Access-Point";
const char* password = "RRpassword";

AsyncWebServer server(80);

String config = "";
DynamicJsonDocument cfg(4096);


/*
 * Setup function on board startup
 */
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  EEPROM.begin(4096);
  InitGetConfig();

  if (IsInConfigMode()){
    // up WiFi as hotspot
    SetupConfigWiFi();

  }else{
    // normal mode

  }
  SetupWebServer();
}


/*
 * Function for infinite loop
 */
void loop() {
  
}


/*
 * Determine if board should be loaded in configuration mode
 * Will return true if pin BUTTON_PIN_CONFIG_MODE connected to GND 
 *
 * @return Boolean 
 */
bool IsInConfigMode(){
  pinMode(BUTTON_PIN_CONFIG_MODE, INPUT_PULLUP);
  sleep(2); // wait till board is loading, otherwise can get wrong answer
  return (digitalRead(BUTTON_PIN_CONFIG_MODE) == 0 ? true : false);
}


/*
 * Create Wi-Fi network with SSID and password 
 * used in configuration mode.
 */
void SetupConfigWiFi(){
  Serial.println("Setting AP (Access Point) in config mode");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}


/*
 * Web server setup
 */
void SetupWebServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("GET /");
    String resp = "";
    resp += "<!DOCTYPE html>";
    resp += "<html><head>";
    resp += "</head><body>RailRoad server OK.";
    resp += "<form method=post action=/save>";
    resp += "<textarea name=config>" + config + "</textarea>";
    resp += "<input type=submit value=Update!>";
    resp += "</form></body></html>";
    resp += "";
    request->send(200, "text/html", resp);  
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("POST /save");
    String newconfig = "";
    int params = request->params();
      for (int i = 0; i < params; i++)
        {
        AsyncWebParameter* p = request->getParam(i);
        if (String(p->name().c_str())=="config"){
            newconfig = String(p->value().c_str());
        }
      }

    EEPROM.writeString(0, newconfig);
    EEPROM.commit();
    Serial.println("Saved config: " + newconfig);
    
    String resp = "";
    resp += "<!DOCTYPE html>";
    resp += "<html><head>";
    resp += "</head><body>Saved. Do you want to <a href=/restart>restart</a> your board?</body></html>";
    resp += "";
    request->send(200, "text/html", resp);  
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("GET /restart");

    String resp = "";
    resp += "<!DOCTYPE html>";
    resp += "<html><head>";
    resp += "</head><body>";
    resp += "Restart OK.";
    resp += "</body></html>";
    resp += "";
    
    request->send(200, "text/html", resp);  
    ESP.restart();
  });

  server.begin();
}


/*
 * Get initial configuration
 */
void InitGetConfig(){ 
  config = EEPROM.readString(0);
  deserializeJson(cfg, config);
}
