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

//String WebPage = "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"><head><meta charset=\"utf-8\"><title>Lighting</title><style>.btn {width: 30vw;height: 30vh;}</style><script>var Socket, toSend, colorSend;function start() {var r = [255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 125, 255, 255];var g = [255, 214, 147, 0, 125, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0];var b = [255, 170, 71, 0, 0, 0, 0, 0, 125, 255, 255, 255, 255, 255, 125];for (var i = 0; i < r.length; i++) {var color = 'rgb(' + r[i] + ',' + g[i] + ',' + b[i] + ')';var btn = document.createElement(\"button\");btn.id = color;btn.className = \"btn\";btn.addEventListener(\"click\", function(e) {colorSend = 'rgba' + this.id.slice(3, -1);changeColor();});btn.style.background = color;var theEnd = document.getElementById(\"theEnd\");theEnd.insertAdjacentElement(\"beforeend\", btn);}Socket = new WebSocket('wss://172.20.10.5:81/');Socket.onopen = function(event) {console.log(\"WebSocket is open now.\");};}function changeColor() {toSend = colorSend + ','+ document.getElementById('a').value + ')';console.log(toSend);document.body.style.backgroundColor = toSend;Socket.send(toSend);}</script></head><body onload=\"javascript:start();\"><center id=\"theEnd\"><input type=\"range\" min=\"0\" max=\"1\" value=\"0.5\" step = \"0.01\" class=\"slider\" id=\"a\" oninput=\"changeColor()\"><br></center></body></html>";

String WebPage = "<!DOCTYPE html><html><style>input[type=\"text\"]{width: 90%; height: 3vh;}input[type=\"button\"]{width: 9%; height: 3.6vh;}.rxd{height: 90vh;}textarea{width: 99%; height: 100%; resize: none;}</style><script>var Socket;function start(){Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function(evt){document.getElementById(\"rxConsole\").value +=evt.data;}}function enterpressed(){Socket.send(document.getElementById(\"txbuff\").value); document.getElementById(\"txbuff\").value=\"\";}</script><body onload=\"javascript:start();\"> <div><input class=\"txd\" type=\"text\" id=\"txbuff\" onkeydown=\"if(event.keyCode==13) enterpressed();\"><input class=\"txd\" type=\"button\" onclick=\"enterpressed();\" value=\"Send\" > </div><br><div class=\"rxd\"> <textarea id=\"rxConsole\" readonly></textarea> </div></body></html>";

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
