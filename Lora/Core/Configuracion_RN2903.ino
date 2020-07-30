#define RESET 15
void setup()
{
  pinMode(2, OUTPUT);
  led_on();
  Serial.begin(115200);
  Serial2.begin(57600);
  delay(1000);
  Serial.println("Inicio");
  Ini_RN();
  led_off();
  delay(2000);  
}
void led_on()
{
  digitalWrite(2,1);
}
void led_off()
{
  digitalWrite(2,0);
}
void Ini_RN()
{
  pinMode (RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);  
  delay(100);
  Serial2.flush ();
  Config_Lora();
}
void RX_Lora()
{
  String input;
  String input2;
  delay(1000);
    while (Serial2.available())
    {
      input=char(Serial2.read());
      input2 =input2 + "" +input;
    }
  Serial.println(input2+"Listo");
  input2="";
}
void Config_Lora ()
{
  delay(1000);
  Serial2.println("sys get reset");
  Serial.println("sys get reset:");
  RX_Lora();
  delay(1000);
  Serial2.println("sys get hweui");
  Serial.println("sys get hweui:");
  RX_Lora();
  delay(1000);
  Serial2.println("sys get ver");
  Serial.println("sys get ver:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac get deveui");
  Serial.println("mac get deveui:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac set nwkskey 297DFB4BE07B4908DDA5D1C2B7509C90");
  Serial.println("mac set nwkskey 297DFB4BE07B4908DDA5D1C2B7509C90:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac set appskey 57F95428F49CFC3F271A061D05DAB6C7");
  Serial.println("mac set appskey 57F95428F49CFC3F271A061D05DAB6C7:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac set devaddr 00117EAE");
  Serial.println("mac set devaddr 00117EAE:");
  RX_Lora();
  delay(1000);
  Serial2.println("radio set pwr 20");
  Serial.println("radio set pwr 20:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac save");
  Serial.println("mac save:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac join abp");
  Serial.println("mac join abp:");
  RX_Lora();
  delay(1000);
  Serial2.println("mac set dr 4");
  Serial.println("mac set dr 4:");
  RX_Lora();
  delay(1000);
  Serial.println("Ya se configuro RN2903.");
}
void loop() 
{
  led_on();
  delay(2000);
  Serial.println("TX");
  led_off();
  delay(2000);
}
