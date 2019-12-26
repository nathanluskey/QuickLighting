/*
 * https://www.hackster.io/brzi/nodemcu-websockets-tutorial-3a2013
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

const char* ssid = "Nathan's IPhone";
const char* password = "password";

//Static IP address configuration: https://circuits4you.com/2018/03/09/esp8266-static-ip-address-arduino-example/
IPAddress staticIP(172, 20, 10, 5); //ESP static ip
IPAddress gateway(172, 20, 10, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS
const char* deviceName = "MyRoomLighting.local";

//Our HTML webpage contents in program memory
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en" dir="ltr">
<head>
    <meta charset="utf-8">
    <title>Arduino Webpage</title>
</head>
<body onload="start()">
    <a href="https://nilnate.github.io/QuickLighting/Final/Website.html">link text</a>
    <script type="text/javascript">
    function start() {
        //window.location.href = "https://nilnate.github.io/QuickLighting/Final/Website.html";
        console.log("Hello");
    }
    </script>
</body>
</html>
)=====";

String WebPage = MAIN_page;

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

void setup() {
  WiFi.disconnect();
  Serial.begin(115200);
  WiFi.hostname(deviceName);
  WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   }
    
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.on("/", [](){
    server.send(200, "text/html", WebPage);
    });
    
    server.begin();
    
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
    server.handleClient();
    if (Serial.available() > 0){
      char c[] = {(char)Serial.read()};
      webSocket.broadcastTXT(c, sizeof(c));
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
   if (type == WStype_TEXT){
    for(int i = 0; i < length; i++) Serial.print((char) payload[i]);
    Serial.println();
   }
}
