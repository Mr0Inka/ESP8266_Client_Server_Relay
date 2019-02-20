const String  clientName = "1";
//------------------------------------------------------------------------------------
#include <Wire.h>
#include <ESP8266WiFi.h>
//------------------------------------------------------------------------------------
#define       BUTTON    D4        // Connectivity ReInitiate Button
#define       TWI_FREQ  400000L   // I2C Frequency Setting To 400KHZ
//------------------------------------------------------------------------------------
long          nextPing;
int           pingInterval = 2000;
long          lastPong;
int           lastButtonState   = HIGH;
//------------------------------------------------------------------------------------
char*         serverSSID;
char*         serverPassword;
IPAddress     serverAddr(192, 168, 4, 1);
WiFiClient    btnClient;
//====================================================================================

void setup() {
  Serial.println("");
  Serial.println("=========================================");
  Wire.begin();                   // Begin I2C
  Wire.setClock(TWI_FREQ);        // Set frequency
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_STA);            // Avoid broadcasting SSID
  WiFi.begin("ESP_Relay");        // Server SSID
  Serial.println(" [>] Connecting to " + WiFi.SSID());
  CheckConnectivity();            // Checking For Connection
  Serial.println(" [>] Client connected");

  Serial.println(" [>] Connected To      : " + String(WiFi.SSID()));
  Serial.println(" [>] Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
  Serial.print  (" [>] Server IP Address : ");
  Serial.println(serverAddr);
  Serial.print  (" [>] Device IP Address : ");
  Serial.println(WiFi.localIP());
  Serial.println("=========================================");

  nextPing = millis() + pingInterval;
  lastPong = millis();

  sendReq();
}

//====================================================================================

void loop()
{
  long currMill = millis();

  if (currMill > (lastPong + 2500)) {
    Serial.println(" [>] Connection lost - Restarting...");
    WiFi.disconnect();
    delay(1000);
    setup();
  }

  if (currMill > nextPing) {
    nextPing = millis() + pingInterval;
    pingServer();
  }

  readServer();
  readBtn();
}

//====================================================================================

void readBtn() {
  int reading = digitalRead(BUTTON);
  if (reading != lastButtonState) {
    if (reading == HIGH) {
      Serial.println(" [>] Button released");
      btnClient.println(clientName + ":released");
      btnClient.flush();
    } else {
      Serial.println(" [>] Button pressed");
      btnClient.println(clientName + ":pressed");
      btnClient.flush();
    }
    lastButtonState = reading;
  }
}

//====================================================================================

void CheckConnectivity() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(" [>] Finding server...");
    digitalWrite(2, LOW);
    delay(250);
    digitalWrite(2, HIGH);
    delay(250);
  }
}

//====================================================================================

void sendReq() {
  btnClient.stop();
  if (btnClient.connect(serverAddr, 9001)) {
    Serial.println    (" [+] " + clientName + ":connect");
    btnClient.println (clientName + ":connect");
  }
}

void pingServer() {
  Serial.println(" [>] " + clientName + ":ping");
  btnClient.println(clientName + ":ping");
  btnClient.flush();
}

void readServer() {
  if (btnClient && btnClient.connected()) {
    if (btnClient.available()) {
      while (btnClient.available()) {
        String clientMsg = btnClient.readStringUntil('\n');
        clientMsg.trim();
        if (clientMsg == "pong") {
          lastPong = millis();
        }
        Serial.println(" [>] Server:" + clientMsg);
      }
    }
  } else {
    Serial.println(" [>] Lost connection...");
  }
}
//====================================================================================
