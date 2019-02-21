# ESP8266_Client_Server_Relay

This sketch needs to be written onto generic ESP8266 NodeMCUs. There is one server and up to 4 clients. Each client has a button attached to GND and D4. Upon button press, the server triggers a section of an addressable LED Strip to be on/off.  

## Clients: 
1. Download esp_client.ino and edit the first line ("clientName") to a string number from "1" to "4". This will configure, which server output (relay/led) will be triggered.
2. Flash the file to the NodeMCU
3. Attach a button to GND and D4
4. Done!

## Server: 
1. Download esp_relay_server.ino and edit the first lines (modeOne, modeTwo, ...). These can be set to either "toggle", "hold" or "timed". While toggle and hold should be self explaining, times will use the variables below (timeOne, timeTwo, ...). These are milliseconds. A button press will enable the relay channel and disable it again after a set amount of milliseconds.
2. Flash your NodeMCU
3. Done!

> When powering up, the clients will blink while trying to find the server. When the server is in range, they will connect to it and stop blinking. Now you can use the push buttons to toggle, hold or enable your channels for X milliseconds.

### Notes:
+ Buttons and server will identify a lost connection and reconnect by themselves (uses a ping/pong system)

