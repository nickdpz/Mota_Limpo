#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <NewPing.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>
#include <FS.h>
// Required for LIGHT_SLEEP_T delay mode
extern "C"
{
#include "user_interface.h"
}

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define LED_STATUS 14  //0 - D3//14 - D5
#define LED_WIFI 2     //2 - D4
#define TRIGGER_PIN 15 //15 - D8
#define ECHO_PIN_1 13  //13 - D7
#define ECHO_PIN_2 12  //12 - D6
#define BUTTON_C 0     //16 - D0//5 - D1
#define MAX_DISTANCE 200

#define WAKE_UP_PIN 0 // D3/GPIO0, can also force a serial flash upload with RESET
// you can use any GPIO for WAKE_UP_PIN except for D0/GPIO16 as it doesn't support interrupts
#define SERVER_HOST "https://limpo.com.co"
#define SERVER_IP_ROUTE "https://limpo.com.co/routes"
//#define SERVER_IP_ROUTE "http://192.168.0.7:3000/routes"
#define SERVER_IP_WIFI "https://limpo.com.co/nodes/network"
//#define SERVER_IP_WIFI "http://192.168.0.7:3000/nodes/network"
#define SERVER_IP_TEST "https://limpo.com.co/test"

//81:F1:30:54:A6:72:95:9C:EA:5B:CA:35:09:19:85:A9:B5:B5:12:10
//#define SERVER_IP_TEST "http://192.168.0.7:3000/routes/test"

char stateNode[] = "f";
char stassid[] = "xxxxxxxxxxxxxxxx";
char stapsk[] = "xxxxxxxxxxxxxxxx";

char ssid_d[] = "limpo";
char pass_d[] = "00000000";

const char *eui = "00050";
const char *euipsk = "._6mj&.dsh97kkb";

const int timeThreshold = 150;
long startTime = 0;
char bufferP[200] = "";
int distance = 0;
int aux1 = 0;
int aux2 = 0;
bool flagOpen = false;
bool flagClose = false;
bool flagAlert1 = false;
bool flagInterrupt = false;
bool flagSend = false;
bool flagMesuare = true;
int battery = 10;
int rssi = 0;
const char* fingerprint = "81 F1 30 54 A6 72 95 9C EA 5B CA 35 09 19 85 A9 B5 B5 12 10";

WiFiClientSecure client;
HTTPClient http;
NewPing sonar1(TRIGGER_PIN, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN, ECHO_PIN_2, MAX_DISTANCE);

void alert()
{
  flagSend = true;
  display.setTextSize(1);
  display.fillRect(0, 12, 128, 8, BLACK);
  display.setCursor(4, 12);
  display.println("Send Mesuares ..");
  display.display();
  display.fillRect(0, 25, 128, 8, BLACK);
  sprintf(bufferP, "Dm:%i cm B:%i per", distance, battery, aux2);
  display.setCursor(0, 25);
  display.print(bufferP);
  display.display();
  client.setInsecure(); //the magic line, use with caution
  client.connect(SERVER_HOST, 443);
  sprintf(bufferP, "{\"eui\":\"%s\",\"pass\":\"%s\",\"content\":%i,\"battery\":%i}", eui, euipsk, distance, battery);
  Serial.print("[HTTP] Send Measure...\n");
  http.begin(client, SERVER_IP_ROUTE); //HTTP
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(bufferP);
  String payload = http.getString();
  if (httpCode > 0)
  {
    Serial.printf("[HTTP] Response Measure Code: %d\n Payload: ", httpCode);
    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      Serial.println(payload);
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  flagSend = false;
  http.end();
}

bool updateWiFi()
{
  client.setInsecure(); //the magic line, use with caution
  client.connect(SERVER_HOST, 443);
  sprintf(bufferP, "{\"eui\":\"%s\",\"pass\":\"%s\"}", eui, euipsk);
  Serial.print("[HTTP] Update Network...\n");
  http.begin(client, SERVER_IP_WIFI);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(bufferP);
  String payload = http.getString();
  if (httpCode > 0)
  {
    Serial.printf("[HTTP] Update Network Response Code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
      }
      //Serial.println(doc["message"]["idWiFi"].as<char*>());
      //Serial.println(doc["message"]["passWiFi"].as<char*>());
      sprintf(stassid, "%s", doc["message"]["idWiFi"].as<char *>());
      sprintf(stapsk, "%s", doc["message"]["passWiFi"].as<char *>());
      Serial.print("Id WiFi:");
      Serial.println(stassid);
      Serial.print("Password WiFi:");
      Serial.println(stapsk);
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.getString().c_str());
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    return false;
  }
  http.end();
  return true;
}

void IRAM_ATTR irq_button_c()
{
  if ((millis() - startTime > timeThreshold) && (flagInterrupt))
  {
    flagInterrupt = false;
    Serial.println("IRQ");
    if (flagOpen)
    {
      Serial.println("Close Container");
    }
    else
    {
      Serial.println("Open Container");
    }
    flagClose = true;
    flagOpen = !flagOpen;
    startTime = millis();
    flagInterrupt = true;
  }
}

bool initWiFi()
{
  bool flagOut = true;
  if (WiFi.status() != WL_CONNECTED)
  {
    wifi_set_sleep_type(NONE_SLEEP_T);
    wifi_fpm_close();
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 4);
    display.print("Connecting to");
    display.setCursor(0, 16);
    Serial.print("Connecting to : ");
    Serial.println(stassid);
    Serial.print("Pass : ");
    Serial.println(stapsk);
    display.println(stassid);
    display.display();
    WiFi.begin(stassid, stapsk);
    uint8_t count = 0;
    while ((WiFi.status() != WL_CONNECTED) && (flagOut))
    {
      display.print(".");
      display.display();
      delay(500);
      if (count > 50)
      {
        flagOut = false;
      }
      count++;
    }
    if (flagOut)
    {
      display.clearDisplay();
      display.setCursor(0, 4);
      display.setTextSize(2);
      display.println("Connected");
      display.setTextSize(1);
      display.setCursor(6, 22);
      display.print("to ");
      display.print(stassid);
      display.display();
      digitalWrite(LED_STATUS, flagAlert1);
      delay(3000);
    }
    else
    {
      display.clearDisplay();
      display.setCursor(0, 4);
      display.setTextSize(2);
      display.println("No Connect");
      display.setTextSize(1);
      display.setCursor(6, 22);
      display.print("set default WiFi");
      display.display();
      digitalWrite(LED_STATUS, false);
    }
    return flagOut;
  }
}

void set_light_sleep()
{
  Serial.println("Enter light sleep mode");
  updateNodeState();
  printModeSleep();
  detachInterrupt(digitalPinToInterrupt(BUTTON_C));
  digitalWrite(LED_WIFI, HIGH);
  uint32_t sleep_time_in_ms = 400000;
  WiFi.mode(WIFI_OFF); // you must turn the modem off; using disconnect won't work
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T); //LIGHT_SLEEP_T//MODEM_SLEEP_T
  gpio_pin_wakeup_enable(GPIO_ID_PIN(BUTTON_C), GPIO_PIN_INTR_LOLEVEL);
  wifi_fpm_open();
  wifi_fpm_do_sleep(sleep_time_in_ms * 1000);
  delay(sleep_time_in_ms + 1);
  attachInterrupt(digitalPinToInterrupt(BUTTON_C), irq_button_c, FALLING);
  Serial.println("Exit light sleep mode");
}

void updateWiFiCredentials(uint8_t def)
{
  File testFile = SPIFFS.open(F("/credentials_wifi.txt"), "w");
  if (testFile)
  {
    Serial.println("[dB] Write credential wifi file");
    if (def == 1)
    {
      sprintf(bufferP, "%s\n%s\n", ssid_d, pass_d);
    }
    else
    {
      sprintf(bufferP, "%s\n%s\n", stassid, stapsk);
    }
    testFile.print(bufferP);
    testFile.close();
  }
  else
  {
    Serial.println("[dB] Problem on create file");
  }
}

void getWiFiCredentials()
{
  File testFile = SPIFFS.open(F("/credentials_wifi.txt"), "r");
  if (testFile)
  {
    Serial.println("[dB] Read credential wifi file");
    uint8_t i = 0;
    while (testFile.available())
    {
      if (i == 0)
      {
        sprintf(stassid, "%s", testFile.readStringUntil('\n').c_str());
      }
      if (i == 1)
      {
        sprintf(stapsk, "%s", testFile.readStringUntil('\n').c_str());
      }
      i++;
    }
    Serial.println("...........");
    Serial.println(stassid);
    Serial.println(stapsk);
    Serial.println("...........");
    testFile.close();
  }
  else
  {
    Serial.println("[dB] Problem on read credential wifi file ");
    updateWiFiCredentials(1); //se hace la primera vez
  }
}

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
    Serial.println("-----------");
    Serial.println(flagAlert1);
    digitalWrite(LED_STATUS, flagAlert1);
    testFile.close();
  }
  else
  {
    Serial.println("[dB] Problem on read state node file ");
  }
}

void setup()
{
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(BUTTON_C, INPUT_PULLUP);
  digitalWrite(LED_STATUS, HIGH);
  delay(200);
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_STATUS, LOW);
  attachInterrupt(digitalPinToInterrupt(BUTTON_C), irq_button_c, FALLING);
  Serial.begin(9600);
  Serial.println("Start ...");
  SPIFFS.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.display();
  getWiFiCredentials();
  updateNodeState();
  //getNodeState();
  //updateWiFiCredentials(1);
  if (!initWiFi())
  {
    updateWiFiCredentials(1);
    delay(6000000);
    while (true); //si no se puede conectar se queda en un ciclo infinito
  }
  if (updateWiFi())
  {
    updateWiFiCredentials(0);
  }
}

void loop()
{
  if (!flagOpen)
  {
    initWiFi();
    initOled();
    levelBattery(100);
    levelWiFi(100);
    getDistance();
    distance = (aux1 + aux2) * 0.5;
    for (uint8_t count = 0; count < 40; count++)
    {
      if ((!flagOpen) && (WiFi.status() == WL_CONNECTED))
      {
        measureDistance();
        if (flagAlert1 && flagClose)
        { //si acabo de cerrar contenedor
          if (distance > 35)
          {
            flagClose = false;
            flagAlert1 = false;
            digitalWrite(LED_STATUS, false);
            printCharacter(' ', 80, 0);
          }
        }
        printMeasure();
        statusOn(10);
      }
    }
    if (!flagOpen)
    {
      if (distance < 37)
      {
        flagAlert1 = true;
        digitalWrite(LED_STATUS, true);
        printCharacter('A', 80, 0);
      }
      alert();
      set_light_sleep();
    }
  }
  else
  {
    if (flagClose)
    {
      printOpenContainer();
      flagClose = false;
    }
    statusOn(1);
  }
}

void statusOn(int time)
{
  digitalWrite(LED_WIFI, HIGH);
  delay(time * 100);
  digitalWrite(LED_WIFI, LOW);
  delay(time * 100);
  flagInterrupt = true;
}

void getDistance()
{
  aux1 = sonar1.ping_cm();
  if (aux1 == 0)
  {
    aux1 = 200;
  }
  delay(10);
  aux2 = sonar2.ping_cm();
  if (aux2 == 0)
  {
    aux2 = 200;
  }
}

void measureDistance()
{
  getDistance();
  if (abs(aux2 - aux1) > 30)
  {
    printCharacter('S', 70, 0);
  }
  else
  {
    display.fillRect(70, 0, 7, 8, BLACK);
  }
  if (distance < 37)
  {
    printCharacter('A', 80, 0);
  }
  else
  {
    printCharacter(' ', 80, 0);
  }
  distance = ((aux1 + aux2) * 0.5 + distance) * 0.5;
  display.fillRect(84, 12, 8, 8, BLACK);
  if (flagMesuare) {
    printCharacter('.', 84, 12);
    flagMesuare = false;
  } else {
    printCharacter('*', 84, 12);
    flagMesuare = true;
  }
}

void printMeasure()
{
  battery = round(map(analogRead(A0), 0, 1023, 0, 100)); // leer conversor
  rssi = round(map(WiFi.RSSI(), -90, -55, 0, 100));
  sprintf(bufferP, "Dm:%i D1:%i D2:%i", distance, aux1, aux2);
  Serial.println(bufferP);
  display.fillRect(0, 25, 128, 8, BLACK);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print(bufferP);
  display.display();
  levelBattery(battery);
  levelWiFi(rssi);
}

void printCharacter(char a, int x, int y)
{
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(a);
  display.display();
}

void printModeSleep()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(5, 4);
  display.println("Mode Sleep");
  display.setTextSize(1);
  display.println("       zzz....");
  display.display();
}

void printOpenContainer()
{
  display.setCursor(4, 10);
  display.fillRect(0, 10, 128, 22, BLACK);
  display.setTextSize(2);
  display.setCursor(0, 12);
  display.println("  Open ..");
  display.display();
}

void levelBattery(uint8_t level)
{
  display.fillRect(98, 1, 24, 6, BLACK);
  if (level > 10)
  {
    display.fillRect(98, 1, 6, 6, WHITE);
  }
  if (level > 50)
  {
    display.fillRect(104, 1, 6, 6, WHITE);
  }
  if (level > 75)
  {
    display.fillRect(110, 1, 6, 6, WHITE);
  }
  if (level > 90)
  {
    display.fillRect(116, 1, 6, 6, WHITE);
  }
  display.display();
}

void levelWiFi(uint8_t level)
{
  display.fillRect(33, 0, 27, 8, BLACK);
  if (level > 10)
  {
    display.fillRoundRect(33, 0, 5, 8, 2, WHITE);
  }
  if (level > 50)
  {
    display.fillRoundRect(40, 2, 5, 6, 2, WHITE);
  }
  if (level > 75)
  {
    display.fillRoundRect(47, 4, 5, 4, 2, WHITE);
  }
  if (level > 90)
  {
    display.fillRoundRect(53, 6, 5, 2, 2, WHITE);
  }
  display.display();
}

void initOled()
{
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  //Wi-Fi
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("Wi-Fi");
  display.display();
  // Battery
  display.drawRoundRect(97, 0, 26, 8, 2, WHITE);
  display.display();
  // Measuaring
  if (!flagSend) {
    display.setTextSize(1);
    display.setCursor(4, 12);
    display.println("Measuring ...");
    display.display();
  }
}
