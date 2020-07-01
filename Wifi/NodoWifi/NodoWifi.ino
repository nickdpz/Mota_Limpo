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
#define MAX_DISTANCE    200
#define SERVER_IP "http://192.168.0.7:3000/routes/test"

const int timeThreshold = 150;
long startTime = 0;

NewPing sonar1(TRIGGER_PIN, ECHO_PIN_1, MAX_DISTANCE);
char stassid[] = "JAPEREZ";
char stapsk[] = "26071967";
char buffer[60] = "";
int distance = 0;
bool flagCollect = false;
bool flagRequest = false;
WiFiClient client;
HTTPClient http;
Ticker ticker;

void alert() {
  sprintf(buffer, "{\"eui\":\"%s\",\"password\":\"%s\",\"data\":%d}", stassid, stapsk, distance);
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, SERVER_IP); //HTTP
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

void IRAM_ATTR irq_button_c()
{
   if (millis() - startTime > timeThreshold)
   {
      Serial.println("IRQ");
      startTime = millis();
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
  if ((WiFi.status() == WL_CONNECTED)) {
    digitalWrite(LED_STATUS, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(LED_STATUS, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);// wait for a second
    distance = sonar1.ping_cm();
    if(distance==0){
    distance=200;  
    }
    Serial.print("Ping: ");
    Serial.print(distance);
    Serial.println("cm");
    if (distance < 37) {
      alert();
    }
  }
}
