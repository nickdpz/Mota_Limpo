#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NewPing.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>
#include <FS.h>
char stateNode[] = "t";
bool flagAlert1 = true;
char bufferP[200] = "";
void updateNodeState()
{
  File testFile = SPIFFS.open(F("/node_state.txt"), "w");
  if (testFile)
  {
    Serial.println("[dB] Write file content");
    if (flagAlert1) {
      sprintf(bufferP, "t\n");
    } else
    {
      sprintf(bufferP, "f\n");
    }
    testFile.print(bufferP);
    testFile.close();
  }
  else
  {
    Serial.println("[dB] Problem on create file");
  }
}

void getNodeState()
{
  File testFile = SPIFFS.open(F("/node_state.txt"), "r");
  if (testFile)
  {
    Serial.println("[dB] Read file content 1");
    sprintf(stateNode, "%s", testFile.readStringUntil('\n').c_str());
    if (strcmp(stateNode, "t") == 0)
    {
      flagAlert1 = true;
    }
    else
    {
      flagAlert1 = false;
    }
    Serial.println("***********");
    Serial.println(stateNode);
    Serial.println("***********");
    Serial.println(flagAlert1);
    Serial.println("-----------");
    testFile.close();
  }
  else
  {
    Serial.println("[dB] Problem on read state node file ");
  }
}


void setup() {
  Serial.begin(9600);
  SPIFFS.begin();
  Serial.println("Inicialize");
  updateNodeState();
  delay(3000);
  getNodeState();
}

void loop() {

}
