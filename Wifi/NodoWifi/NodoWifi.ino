#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NewPing.h>
// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "user_interface.h"
}

#define LED_STATUS     0//0 - D3
#define LED_WIFI       2//2 - D4
#define TRIGGER_PIN    15//15 - D8
#define ECHO_PIN_1     13//13 - D7
#define ECHO_PIN_2     12//12 - D6
#define BUTTON_C       5//16 - D0//5 - D1
#define MAX_DISTANCE   500
#define GPIO_AS_INPUT(gpio_bits) gpio_output_conf(0, 0, 0, gpio_bits)

#define SERVER_IP_ALERT "http://192.168.0.7:3000/routes"
#define SERVER_IP_TEST "http://192.168.0.7:3000/routes/test"
#define SERVER_IP_SUCCESS "http://192.168.0.7:3000/routes/success"

NewPing sonar1(TRIGGER_PIN, ECHO_PIN_1, MAX_DISTANCE);
const char* stassid = "JAPEREZ";
const char* stapsk = "26071967";

const int timeThreshold = 150;
long startTime = 0;
char buffer[60] = "";
int distance = 0;
int bootCount = 0;
bool flagOpen = false;
bool flagSuccess = false;
bool flagAlert1 = false;
bool flagInterrupt = true;
int count = 0;
WiFiClient client;
HTTPClient http;

void alert() {
  sprintf(buffer, "{\"eui\":\"%s\",\"psk\":\"%s\",\"data\":%d}", stassid, stapsk, distance);
  Serial.print("[HTTP] Alert...\n");
  Serial.print("[HTTP] begin...\n");
  http.begin(client, SERVER_IP_TEST); //HTTP
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(buffer);
  String payload = http.getString();
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void success() {
  if (flagSuccess) {

    sprintf(buffer, "{\"eui\":\"%s\",\"psk\":\"%s\"}", stassid, stapsk);
    Serial.print("[HTTP] Success...\n");
    http.begin(client, SERVER_IP_SUCCESS); //HTTP
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(buffer);
    String payload = http.getString();
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    digitalWrite(LED_STATUS, LOW);
    flagSuccess = false;
  }
}

void IRAM_ATTR irq_button_c()
{
  if ((millis() - startTime > timeThreshold) && (flagInterrupt))
  {
    flagInterrupt = false;
    Serial.println("IRQ");
    if (flagAlert1) {
      if (flagOpen) {
        Serial.println("Close Container");
        distance = sonar1.ping_cm();
        if (distance > 35) {
          flagSuccess = true;
          flagAlert1 = false;
        }
      } else {
        Serial.println("Open Container");
      }
      flagOpen = !flagOpen;
      startTime = millis();
    }
  }
}

void initWiFi() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  wifi_fpm_close();
  WiFi.mode(WIFI_STA);
  WiFi.begin(stassid, stapsk);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  digitalWrite(LED_STATUS, flagAlert1);
}

void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(BUTTON_C, INPUT_PULLUP);
  digitalWrite(LED_WIFI, LOW);
  digitalWrite(LED_STATUS, LOW);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(BUTTON_C), irq_button_c, FALLING);
  Serial.println("Finish Config");
}

void set_light_sleep() {
  Serial.println("Enter light sleep mode");
  detachInterrupt(digitalPinToInterrupt(BUTTON_C));
  uint32_t sleep_time_in_ms = 60000;
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_open();
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);//LIGHT_SLEEP_T//MODEM_SLEEP_T
  digitalWrite(LED_WIFI, HIGH);
  wifi_fpm_do_sleep(sleep_time_in_ms * 1000 );
  delay(sleep_time_in_ms + 1);
  attachInterrupt(digitalPinToInterrupt(BUTTON_C), irq_button_c, FALLING);
  Serial.println("Exit light sleep mode");
}

void loop() {
  if (!flagOpen) {
    success();
    initWiFi();
    distance = sonar1.ping_cm();
    for (uint8_t count = 0; count < 40; count++) {
      if ((!flagOpen) && (WiFi.status() == WL_CONNECTED)) {
        distance = (sonar1.ping_cm() + distance) / 2;
        if (distance == 0) {
          distance = 200;
        }
        Serial.print("D: ");
        Serial.println(distance);
        if (distance < 37) {
          flagAlert1 = true;
          digitalWrite(LED_STATUS, flagAlert1);
        }
        statusOn(10);
      }
    }
    if (!flagOpen) {
      alert();
      set_light_sleep();
    }
  } else {
    statusOn(1);
  }
}

void statusOn(int time) {
  digitalWrite(LED_WIFI, HIGH);
  delay(time * 100);
  digitalWrite(LED_WIFI, LOW);
  delay(time * 100);
  flagInterrupt = true;
}
