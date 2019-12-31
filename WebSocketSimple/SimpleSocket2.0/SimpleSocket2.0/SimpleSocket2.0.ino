/* https://github.com/Links2004/arduinoWebSockets/blob/master/examples/esp8266/WebSocketServer_LEDcontrol/WebSocketServer_LEDcontrol.ino
 * WebSocketServer_LEDcontrol.ino
 *
 *  Created on: 26.11.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#define LED_RED     16
#define LED_GREEN   2
#define LED_BLUE    17

#define USE_SERIAL Serial


ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

                analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
                analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
                analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
            }

            break;
    }

}

void setup() {
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_BLUE, 1);

    //Configure a static IP on a network
    IPAddress staticIP(172, 20, 10, 5); //ESP static ip
    IPAddress gateway(172, 20, 10, 1);   //IP Address of your WiFi Router (Gateway)
    IPAddress subnet(255, 255, 255, 0);  //Subnet mask
    IPAddress dns(8, 8, 8, 8);  //DNS
    WiFi.config(staticIP, subnet, gateway, dns);
    WiFi.begin("Nathan's IPhone", "password");

    //WifiMulti
    WiFiMulti.addAP("Nathan's IPhone", "password");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        USE_SERIAL.print(".");
    }
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    if(MDNS.begin("esp8266")) {
        USE_SERIAL.println("MDNS responder started");
    }

    // handle index
    server.on("/", []() {
        server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"><head><meta charset=\"utf-8\"><title>Lighting</title><style>.btn {width: 30vw;height: 30vh;}</style><script>var Socket, toSend, colorSend;function start() {var r = [255, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0, 0, 125, 255, 255];var g = [255, 214, 147, 0, 125, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0];var b = [255, 170, 71, 0, 0, 0, 0, 0, 125, 255, 255, 255, 255, 255, 125];for (var i = 0; i < r.length; i++) {var color = 'rgb(' + r[i] + ',' + g[i] + ',' + b[i] + ')';var btn = document.createElement(\"button\");btn.id = color;btn.className = \"btn\";btn.addEventListener(\"click\", function(e) {colorSend = 'rgba' + this.id.slice(3, -1);changeColor();});btn.style.background = color;var theEnd = document.getElementById(\"theEnd\");theEnd.insertAdjacentElement(\"beforeend\", btn);}Socket = new WebSocket('ws://172.20.10.5:81/');Socket.onopen = function(event) {console.log(\"WebSocket is open now.\");};}function changeColor() {toSend = colorSend + ','+ document.getElementById('a').value + ')';console.log(toSend);document.body.style.backgroundColor = toSend;Socket.send(toSend);}</script></head><body onload=\"javascript:start();\"><center id=\"theEnd\"><input type=\"range\" min=\"0\" max=\"1\" value=\"0.5\" step = \"0.01\" class=\"slider\" id=\"a\" oninput=\"changeColor()\"><br></center></body></html>");
    });

    server.begin();

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_BLUE, 0);

}

void loop() {
    webSocket.loop();
    server.handleClient();
}
