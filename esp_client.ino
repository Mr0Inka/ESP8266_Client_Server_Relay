//------------------------------------------------------------------------------------
#include <Wire.h>
#include <ESP8266WiFi.h>
//------------------------------------------------------------------------------------
#define       BUTTON    D4        // Connectivity ReInitiate Button
#define       TWI_FREQ  400000L   // I2C Frequency Setting To 400KHZ
//------------------------------------------------------------------------------------
int           ButtonState;
int           LastButtonState   = LOW;
int           LastDebounceTime  = 0;
int           DebounceDelay     = 25;
const String  clientName       = "THREE";
//------------------------------------------------------------------------------------
char*         serverSSID;
char*         serverPassword;
IPAddress     serverAddr(192, 168, 4, 1);
WiFiClient    btnClient;
//====================================================================================

long lastPing;
int pingInterval = 2000;

long lastPong;


void setup()
{
  Wire.begin();                   // Begginning The I2C
  Wire.setClock(TWI_FREQ);        // Setting The Frequency MPU9250 Require
  Serial.begin(115200);           // Computer Communication
  pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity

  WiFi.mode(WIFI_STA);            // To Avoid Broadcasting An SSID
  WiFi.begin("ESP_Relay");          // The SSID That We Want To Connect To
  Serial.println("!--- Connecting To " + WiFi.SSID() + " ---!");
  CheckConnectivity();            // Checking For Connection
  Serial.println("!-- Client Device Connected --!");

  Serial.println("Connected To      : " + String(WiFi.SSID()));
  Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
  Serial.print  ("Server IP Address : ");
  Serial.println(serverAddr);
  Serial.print  ("Device IP Address : ");
  Serial.println(WiFi.localIP());
  lastPing = millis() + pingInterval;
  lastPong = millis();
  sendReq();
}

//====================================================================================

void loop()
{
  long currMill = millis();
  if(currMill > (lastPong + 2500)){
    Serial.println("Connection lost");
    CheckConnectivity();
  }
  if (currMill > lastPing) {
    lastPing = millis() + pingInterval;
    pingServer();
  }
  readServer();
  ReadButton();
}

//====================================================================================

void ReadButton()
{
  int reading = digitalRead(BUTTON);
  if (reading != LastButtonState) {
    LastDebounceTime = millis();
  }

  if ((millis() - LastDebounceTime) > DebounceDelay) {
    if (reading != ButtonState) {
      ButtonState = reading;

      if (ButtonState == LOW) {
        Serial.println(clientName);
        btnClient.println(clientName + ":pulse");
        btnClient.flush();
      }
    }
  }
  LastButtonState = reading;
}

//====================================================================================

void CheckConnectivity() {
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < 10; i++) {
      Serial.print(".");
      delay(30);
    }
    Serial.println("");
  }
}

//====================================================================================

void sendReq()
{
  btnClient.stop();
  if (btnClient.connect(serverAddr, 9001)) {
    Serial.println    (clientName + ":connect");
    btnClient.println (clientName + ":connect");
  }
}

void pingServer() {
  btnClient.println(clientName + ":ping");
  btnClient.flush();
  //Serial.println("PING");
}

void readServer() {
  if (btnClient && btnClient.connected()) {
    if (btnClient.available()) {
      while (btnClient.available()) {
        String clientMsg = btnClient.readStringUntil('\n');
        clientMsg.trim();
        if(clientMsg == "pong"){
          lastPong = millis();
        }
        //Serial.println(clientMsg);
      }
    }
  } else {
    Serial.println("Somehow got disconnected");
  }
}

//====================================================================================
