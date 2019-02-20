//------------------------------------------------------------------------------------
String modeOne   = "toggle";
String modeTwo   = "timed";
String modeThree = "hold";
String modeFour  = "toggled";

int timeOne   = 0;
int timeTwo   = 2500;
int timeThree = 3000;
int timeFour  = 5000;
//------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
int connectRange[4][2] = {{0, 1},{3, 4},{6, 7},{9, 10}};
int ranges[4][2] = {{19, 14},{26, 21},{33, 28},{40, 35}};
//------------------------------------------------------------------------------------
#define NUM_LEDS 42
#define DATA_PIN 4
CRGB leds[NUM_LEDS];
//------------------------------------------------------------------------------------
int timeOut = 2500;
int pingCheckInterval = 500;
long nextCheck;
//------------------------------------------------------------------------------------
char*       APssid;          // SERVER WIFI NAME
char*       APpass;          // SERVER PASSWORD
//------------------------------------------------------------------------------------
#define     maxClients 4               // MAXIMUM NUMBER OF CLIENTS
WiFiServer  APserver(9001);            // THE SERVER AND THE PORT NUMBER
WiFiClient  APclient[maxClients];      // THE SERVER CLIENTS
//====================================================================================

bool outState[] = {false, false, false, false};            //Current led state
int     pings[] = {false, false, false, false};            //Last ping time in ms
String  names[] = {"na", "na", "na", "na"};                //Client identity names
long  turnOff[] = {false, false, false, false};            //Timepoint to disable trigger

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  SetWifi("ESP_Relay", "");
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  setStat(0, false); //Set connection status to false (turn off connection LEDs)
  setStat(1, false); //Set connection status to false (turn off connection LEDs)
  setStat(2, false); //Set connection status to false (turn off connection LEDs)
  setStat(3, false); //Set connection status to false (turn off connection LEDs)
  nextCheck = millis() + timeOut;
}

//====================================================================================

void loop() {
  long currMill = millis();

  if (nextCheck < currMill) {
    nextCheck = currMill + pingCheckInterval;
    checkPings();
  }

  checkClient();
}

//====================================================================================

void SetWifi(char* Name, char* Password) {
  Serial.println("=========================================");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  Serial.println(" [>] WIFI Mode : AccessPoint Station");
  APssid  = Name;
  APpass  = Password;
  WiFi.softAP(APssid, APpass);
  Serial.println(" [>] " + String(APssid) + " started");
  delay(1000);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(" [>] AccessPoint IP : ");
  Serial.println(IP);
  APserver.begin();
  Serial.println(" [>] Server running...");
  Serial.println("=========================================");
}

//====================================================================================

void checkClient() {
  long currMill = millis();
  if (APserver.hasClient()) { //Check if client is actually connected
    for (int i = 0; i < maxClients; i++) {
      if (!APclient[i] || !APclient[i].connected()) {
        if (APclient[i]) {
          APclient[i].stop();
        }
        APclient[i] = APserver.available();
        continue;
      }
    }
    WiFiClient APclient = APserver.available();
    APclient.stop();
  }

  for (int i = 0; i < maxClients; i++) { //Check for data    
    if (APclient[i] && APclient[i].connected()) {
      checkTriggerTimeout(i, currMill);
      if (APclient[i].available()) {
        while (APclient[i].available()) {
          String clientMsg = APclient[i].readStringUntil('\n');
          clientMsg.trim();
          String ident = getValue(clientMsg, ':', 0);
          String event = getValue(clientMsg, ':', 1);

          if (event == "connect") {  //Handle new connection
            clientConnect(ident, i, currMill);
          } else if (event == "pressed") {
            clientPressed(ident, i, currMill);
          } else if (event == "released") {
            clientReleased(ident, i, currMill);
          } else if (event == "ping") {
            long lastPing =  currMill - pings[i];
            pings[i] = currMill;
            APclient[i].println("pong");
            //Serial.println(" [P] Ping after + " + String(lastPing) + "ms [" + ident + "]");
          } else {
            pings[i] = currMill;
            Serial.println(" [?] Unknown event: " + event + " [" + ident + "]");
          }
        }
      }
    }
  }
}

void clientConnect(String identity, int slot, long currTime) {
  Serial.println(" [o] Button connected [" + identity + " / " + getModeSetting(identity) + "]");
  pings[slot] = currTime;
  names[slot] = identity;
  setStat(nameAddress(identity), true);
}

void clientPressed(String identity, int slot, long currTime) {
  Serial.println(" [o] Button pressed [" + identity + "]");
  pings[slot] = currTime;
  if (getModeSetting(identity) == "toggle") {
    if(outState[slot]){
      trigger(nameAddress(identity), false, slot);
    } else {
      trigger(nameAddress(identity), true, slot);
    }
  } else if (getModeSetting(identity) == "hold") {
      trigger(nameAddress(identity), true, slot);
  } else if (getModeSetting(identity) == "timed") {
      trigger(nameAddress(identity), true, slot);
      turnOff[slot] = millis() + getTimeSetting(identity);
  }
}

void clientReleased(String identity, int slot, long currTime) {
  Serial.println(" [o] Button released [" + identity + "]");
  pings[slot] = currTime;

  if (getModeSetting(identity) == "hold") {
      trigger(nameAddress(identity), false, slot);
  }
}

void trigger(int number, bool newState, int slot){
  if(newState){
    Serial.println(" [C] Activated " + String(number + 1));
    outState[slot] = true;
    for (int i = ranges[number][0]; i >= ranges[number][1]; i--) {
      leds[i] = CRGB::White;
    }
    FastLED.show();
  } else {
    Serial.println(" [C] Deactivated " + String(number + 1));
    outState[slot] = false;
    for (int i = ranges[number][0]; i >= ranges[number][1]; i--) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
}

void checkTriggerTimeout(int slot, long currTime){
  if(turnOff[slot] && currTime > turnOff[slot]){
    trigger(nameAddress(names[slot]), false, slot);
    turnOff[slot] = false;
  }
}


void checkPings() {
  long currMill = millis();
  for (int i = 0; i < maxClients; i++) {
    if (pings[i]) {
      if ((currMill - pings[i]) > timeOut) {
        Serial.println(" [X] Disconnect [" + names[i] + "]");
        setStat(nameAddress(names[i]), false);
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

void setStat(int LEDNum, bool outStatus) {
  if (outStatus) {
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

String getModeSetting(String identity){
  if (identity == "1") {
    return modeOne;
  } else if (identity == "2") {
    return modeTwo;
  } else if (identity == "3") {
    return modeThree;
  } else if (identity == "4") {
    return modeFour;
  }
}

int getTimeSetting(String identity){
  if (identity == "1") {
    return timeOne;
  } else if (identity == "2") {
    return timeTwo;
  } else if (identity == "3") {
    return timeThree;
  } else if (identity == "4") {
    return timeFour;
  }
}

int nameAddress(String identity){
  if (identity == "1") {
    return 0;
  } else if (identity == "2") {
    return 1;
  } else if (identity == "3") {
    return 2;
  } else if (identity == "4") {
    return 3;
  }
}
