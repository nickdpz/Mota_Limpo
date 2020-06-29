#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NewPing.h>

//#define SERVER_IP "http://ritaportal.udistrital.edu.co:10280/routes/"
#define SERVER_IP "http://ritaportal.udistrital.edu.co:10280/routes/"


#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif
#define TRIGGER_PIN  12
#define ECHO_PIN_2     11
#define ECHO_PIN_1     10
#define MAX_DISTANCE 200

char eui[] = "00081";
char pass[] = "ababa";
int distance = 0;
char buffer[60] = "";
NewPing sonar1(TRIGGER_PIN, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN, ECHO_PIN_2, MAX_DISTANCE);

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

}

void alert() {
  WiFiClient client;
  HTTPClient http;
  sprintf(buffer, "{eui:%d,password:%s,data:%d} ",eui,pass,distance);
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, SERVER_IP); //HTTP
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(buffer);

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

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    distance=sonar1.ping_cm();
    if(sonar2.ping_cm()>distance){
      distance=sonar2.ping_cm() 
    }
    Serial.print("Ping: ");
    Serial.print();
    Serial.println("cm");
    if(distance<12){
      alert(distance);
    }
  }
  delay(10000);
}
