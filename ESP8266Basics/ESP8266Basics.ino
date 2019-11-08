#include <ESP8266WiFi.h>

#define LED_Pin D4
void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_Pin, OUTPUT);

  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("ESP Network");
  if(result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }
}

void loop()
{
  Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
  delay(3000);
  if (WiFi.softAPgetStationNum() > 0) {
    digitalWrite(LED_Pin, LOW);
  } else {
    digitalWrite(LED_Pin, HIGH);
  }
}
