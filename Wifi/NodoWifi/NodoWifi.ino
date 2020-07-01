#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NewPing.h>
#include <Ticker.h>

#define LED_STATUS     0//0 - D3
#define LED_WIFI       2//2 - D4
#define TRIGGER_PIN    15//15 - D8
#define ECHO_PIN_1     13//13 - D7
#define ECHO_PIN_2     12//12 - D6
#define BUTTON_C       5//16 - D0//5 - D1
#define MAX_DISTANCE    500

#define SERVER_IP_ALERT "http://192.168.0.7:3000/routes"
#define SERVER_IP_TEST "http://192.168.0.7:3000/routes/test"
#define SERVER_IP_SUCCESS "http://192.168.0.7:3000/routes/success"

const int timeThreshold = 150;
long startTime = 0;

NewPing sonar1(TRIGGER_PIN, ECHO_PIN_1, MAX_DISTANCE);
char stassid[] = "JAPEREZ";
char stapsk[] = "26071967";
char buffer[60] = "";
int distance = 0;
bool flagCollect = false;
bool flagSuccess = false;
bool flagAlert1 = false;
bool flagAlert2 = false;
bool flagInterrupt = true;
WiFiClient client;
HTTPClient http;
Ticker ticker;

void alert() {
  sprintf(buffer, "{\"eui\":\"%s\",\"password\":\"%s\",\"data\":%d}", stassid, stapsk, distance);
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, SERVER_IP_TEST); //HTTP
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(buffer);
  //int httpCode = http.POST("{\"hello\":\"world\"}");
  String payload = http.getString();
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
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
  sprintf(buffer, "{\"eui\":\"%s\",\"password\":\"%s\"}", stassid, stapsk);
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, SERVER_IP_SUCCESS); //HTTP
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(buffer);
  String payload = http.getString();
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
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
}

void IRAM_ATTR irq_button_c()
{
  if ((millis() - startTime > timeThreshold) && (flagInterrupt))
  {
    flagInterrupt = false;
    Serial.println("IRQ");
    if (flagAlert1) {
      if (flagCollect) {
        flagSuccess = true;
        flagAlert1 = false;
        flagAlert2 = false;
      }
      flagCollect = !flagCollect;
      startTime = millis();
    }
  }
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(BUTTON_C, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_C), irq_button_c, FALLING);
  Serial.begin(9600);


  WiFi.begin(stassid, stapsk);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_WIFI, HIGH);
}

// the loop function runs over and over again forever
void loop() {
  if ((WiFi.status() == WL_CONNECTED) && (!flagCollect)) {
    statusOn();
    distance = sonar1.ping_cm();
    if (distance == 0) {
      distance = 200;
    }
    Serial.print("Ping: ");
    Serial.print(distance);
    Serial.println("cm");
    if (flagSuccess) {
      if (distance > 40) {
        success();
      }
      flagSuccess = false;
    }
    if ((distance < 15) && (flagAlert1)) {
      alert();
      flagAlert2 = true;
    }
    if ((distance < 37) && (!flagAlert1)&&(!flagAlert2)) {
      alert();
      digitalWrite(LED_STATUS, HIGH);
      flagAlert1 = true;
    }


  }
}
void statusOn() {
  digitalWrite(LED_WIFI, HIGH);
  delay(1000);
  digitalWrite(LED_WIFI, LOW);
  delay(1000);
  flagInterrupt = true;
}
