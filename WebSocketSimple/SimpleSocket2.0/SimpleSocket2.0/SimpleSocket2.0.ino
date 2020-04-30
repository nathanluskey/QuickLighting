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
        server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"><head><meta charset=\"utf-8\"><title>Lighting</title><style>.button {height: 10vh;width: 80vw;border-radius: 50px;background-color: black;border: 10px solid rgb(0,0,0);color: white;text-align: center;display: inline-block;font-size: 7vh;}.slidecontainer {width: 100}.slider {-webkit-appearance: none;width: 95vw;height: 10vh;border-radius: 50px;background: #d3d3d3;outline: none;opacity: 0.7;-webkit-transition: .2s;transition: opacity .2s;}.slider:hover {opacity: 1;}.slider::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 10vw;height: 10vh;border-radius: 50px;background: rgb(0,0,0);cursor: pointer;}.slider::-moz-range-thumb {width: 10vw;height: 10vh;border-radius: 50px;background: rgb(0,0,0);cursor: pointer;}</style></head><body onload = \"start()\"><center><button type=\"button\" id = \"button\" onclick=\"lightIt()\" class = \"button\">Light</button><br><input type=\"range\" min=\"0\" max=\"1\" value=\"0.5\" step = \"0.01\" class=\"slider\" id=\"brightness\" oninput=\"changeColor()\"><br><canvas id=\"canvas\" width=\"300px\" height=\"500px\"></canvas><br><p id=\"testing\"> </p></center><script type=\"text/javascript\">var canvas = document.getElementById(\"canvas\");var ctx = canvas.getContext(\"2d\");var width, height, length;var color, r, g ,b, a;var Socket;function start() {Socket = new WebSocket(\"ws://172.20.10.5:81/\");Socket.onopen = function(event) {console.log(\"WebSocket is open now.\");};canvas.addEventListener(\"mousedown\", function(e){getColor(canvas, e);});window.addEventListener(\"resize\", function(e) {adjustCanvas();fillColorWheel();});adjustCanvas();fillColorWheel();}function adjustCanvas() {width = window.innerWidth;height = window.innerHeight;length = width >= height ? height : width;length = 0.9 * length;canvas.width = length;canvas.height = length;}function fillColorWheel() {var rOuter = [255, 255, 255, 125, 0, 0, 0, 0, 0, 125, 255, 255];var gOuter = [0, 125, 255, 255, 255, 255, 255, 125, 0, 0, 0, 0];var bOuter = [0, 0, 0, 0, 0, 125, 255, 255, 255, 255, 255, 125];var radius = length / 2 - 5;ctx.strokeStyle = \"black\";for (var i = 0; i < rOuter.length; i++) {ctx.beginPath();ctx.moveTo(length / 2, length / 2);ctx.arc(length / 2, length / 2, radius , i * 2 * Math.PI / rOuter.length - Math.PI / 2, (i + 1) * 2* Math.PI / rOuter.length - Math.PI / 2);ctx.closePath();ctx.fillStyle = \"rgb(\" + rOuter[i] + \" ,\" + gOuter[i] + \",\" + bOuter[i] + \")\";ctx.fill();}var rInner = [255, 255, 255];var gInner = [255, 214, 147];var bInner = [255, 170, 71];var innerRadius = 0.5 * radius;for (var i = 0; i < rInner.length; i++) {ctx.beginPath();ctx.moveTo(length / 2, length / 2);ctx.arc(length / 2, length / 2, innerRadius, i * 2 * Math.PI / rInner.length - Math.PI / 2, (i + 1) * 2* Math.PI / rInner.length - Math.PI / 2);ctx.closePath();ctx.fillStyle = \"rgb(\" + rInner[i] + \" ,\" + gInner[i] + \",\" + bInner[i] + \")\";ctx.fill();}}function getColor(canvas, event) {let rect = canvas.getBoundingClientRect();let x = event.clientX - rect.left;let y = event.clientY - rect.top;var imgData = ctx.getImageData(x, y, 1, 1);r =  imgData.data[0];g =  imgData.data[1];b =  imgData.data[2];changeColor();}function changeColor() {a = document.getElementById(\"brightness\").value;color = \"rgba(\" + r + \",\" + g + \",\" + b + \",\" + a + \")\";var complimentColor = \"rgb(\" + (255 - r) + \",\" + (255 - g) + \",\" + (255 - b) + \")\";document.body.style.backgroundColor = color;document.getElementById(\"button\").style.background = color;document.getElementById(\"button\").style.color = complimentColor;document.getElementById(\"button\").style.border = \"10px solid \" + complimentColor;}function lightIt() {console.log(\"Color Sent: rgba(\" + r + \", \" + g + \", \" + b + \", \" + a + \")\")Socket.send(color);}</script></body></html>");
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
