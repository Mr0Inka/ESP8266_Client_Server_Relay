//------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
//------------------------------------------------------------------------------------
#define NUM_LEDS 42
#define DATA_PIN 4
CRGB leds[NUM_LEDS];
//------------------------------------------------------------------------------------
char*       APssid;          // SERVER WIFI NAME
char*       APpass;          // SERVER PASSWORD
//------------------------------------------------------------------------------------
#define     maxClients     4           // MAXIMUM NUMBER OF CLIENTS
WiFiServer  APserver(9001);            // THE SERVER AND THE PORT NUMBER
WiFiClient  APclient[maxClients];      // THE SERVER CLIENTS
//====================================================================================

int pings[] = {false, false, false, false};
String names[] = {"na", "na", "na", "na"};
int connectRange[4][2] = {
  {0, 1},
  {3, 4},
  {6, 7},
  {9, 10},
};
bool ledStat[] = {false, false, false, false};
int ranges[4][2] = {
  {19, 14},
  {26, 21},
  {33, 28},  
  {40, 35}
};

int timeOut = 2500;
int pingCheckInterval = 500;

long nextCheck;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  nextCheck = millis() + timeOut;
  Serial.begin(115200);
  delay(500);
  SetWifi("ESP_Relay", "");  //WIFI NAME AND PASSWORD
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  setStat(0, false);
  setStat(1, false);
  setStat(2, false);
  setStat(3, false);
}

//====================================================================================

void loop() {
  long currMill = millis();
  if (nextCheck < currMill) {
    nextCheck = currMill + pingCheckInterval;
    checkPings();
  }
  IsClients();
}

//====================================================================================

void SetWifi(char* Name, char* Password) {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  Serial.println("WIFI Mode : AccessPoint Station");
  APssid  = Name;
  APpass  = Password;
  WiFi.softAP(APssid, APpass);
  Serial.println("WIFI < " + String(APssid) + " > ... Started");
  delay(1000);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);
  APserver.begin();
  Serial.println("Server Started");
}

//====================================================================================

void IsClients() {
  long currMill = millis();
  if (APserver.hasClient()) {
    for (int i = 0; i < maxClients; i++) {  //find free/disconnected spot
      if (!APclient[i] || !APclient[i].connected()) {
        if (APclient[i]) {
          APclient[i].stop();
        }
        APclient[i] = APserver.available();
        continue;
      }
    }
    // no free / disconnected spot so reject
    WiFiClient APclient = APserver.available();
    APclient.stop();
  }

  //check clients for data -------------------------------------------------------

  for (int i = 0; i < maxClients; i++) {
    if (APclient[i] && APclient[i].connected()) {
      if (APclient[i].available()) {
        while (APclient[i].available()) {
          String clientMsg = APclient[i].readStringUntil('\n');
          clientMsg.trim();
          String ident = getValue(clientMsg, ':', 0);
          String event = getValue(clientMsg, ':', 1);
          if (event == "connect") {
            Serial.println(" [o] Connect [" + ident + "]");
            pings[i] = currMill;
            names[i] = ident;
            if (ident == "ONE") {
              setStat(0, true);
            };
            if (ident == "TWO") {
              setStat(1, true);
            };
            if (ident == "THREE") {
              setStat(2, true);
            };
            if (ident == "FOUR") {
              setStat(3, true);
            };
          } else if (event == "pulse") {
            //Serial.println(" [+] Pulse [" + ident + "]");
            if (ident == "ONE") {
              triggerLed(0);
            };
            if (ident == "TWO") {
              triggerLed(1);
            };
            if (ident == "THREE") {
              triggerLed(2);
            };
            if (ident == "FOUR") {
              triggerLed(3);
            };
          } else if (event == "ping") {
            long lastPing =  currMill - pings[i];
            pings[i] = currMill;
            APclient[i].println("pong");
            //Serial.println(" [P] Ping after + " + String(lastPing) + "ms [" + ident + "]");
          } else if(event == "holdstart"){
            pings[i] = currMill;
            Serial.println(" [+] Holding [" + ident + "]");
            if (ident == "ONE") {
              forceLED(0, true);
            };
            if (ident == "TWO") {
              forceLED(1, true);
            };
            if (ident == "THREE") {
              forceLED(2, true);
            };
            if (ident == "FOUR") {
              forceLED(3, true);
            };
          } else if(event == "holdend"){
            pings[i] = currMill;
            Serial.println(" [+] Stopped [" + ident + "]");
            if (ident == "ONE") {
              forceLED(0, false);
            };
            if (ident == "TWO") {
              forceLED(1, false);
            };
            if (ident == "THREE") {
              forceLED(2, false);
            };
            if (ident == "FOUR") {
              forceLED(3, false);
            };
          } else {
            pings[i] = currMill;
            Serial.println(" [?] Unknown event: " + event + " [" + ident + "]");
          }
        }
      }
    }
  }
}

void checkPings() {
  long currMill = millis();
  for (int i = 0; i < maxClients; i++) {
    if (pings[i]) {
      if ((currMill - pings[i]) > timeOut) {
        Serial.println(" [X] Disconnect [" + names[i] + "]");
        if (names[i] == "ONE") {
          setStat(0, false);
        };
        if (names[i] == "TWO") {
          setStat(1, false);
        };
        if (names[i] == "THREE") {
          setStat(2, false);
        };
        if (names[i] == "FOUR") {
          setStat(3, false);
        };
        pings[i] = false;
        names[i] = "na";
        APclient[i].stop();
      }
    }
  }
}


String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void triggerLed(int LEDRange) {
  if (ledStat[LEDRange]) {
    Serial.println(" [L] Turned off LED_" + String(LEDRange + 1));
    ledStat[LEDRange] = false;
    for (int i = ranges[LEDRange][0]; i >= ranges[LEDRange][1]; i--) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  } else {
    Serial.println(" [L] Turned on LED_" + String(LEDRange + 1));
    ledStat[LEDRange] = true;
    for (int i = ranges[LEDRange][0]; i >= ranges[LEDRange][1]; i--) {
      leds[i] = CRGB::White;
      FastLED.show();
    }
  }
}

void forceLED(int LEDRange, bool onMode) {
  if (onMode == false) {
    Serial.println(" [L] Forced off LED_" + String(LEDRange + 1));
    ledStat[LEDRange] = false;
    for (int i = ranges[LEDRange][0]; i >= ranges[LEDRange][1]; i--) {
      leds[i] = CRGB::Black;
      FastLED.show();
    }
  } else {
    Serial.println(" [L] Forced on LED_" + String(LEDRange + 1));
    ledStat[LEDRange] = true;
    for (int i = ranges[LEDRange][0]; i >= ranges[LEDRange][1]; i--) {
      leds[i] = CRGB::White;
      FastLED.show();
    }
  }
}

void setStat(int LEDNum, bool LEDstatus) {
  if (LEDstatus) {
    for (int i = connectRange[LEDNum][0]; i <= connectRange[LEDNum][1]; i++) {
      leds[i] = CRGB::Blue;
    }
  } else {
    for (int i = connectRange[LEDNum][0]; i <= connectRange[LEDNum][1]; i++) {
      leds[i] = CRGB::Red;
    }
  }
  FastLED.show();
}
//====================================================================================
