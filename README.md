# ESP8266_Client_Server_Relay

This sketch need to be written onto generic ESP8266s. There is one server and up to 4 clients. Each client has a button attached to GND and D4. Upon button press, the server triggers a section of an addressable LED Strip to be on/off.  

Includes ping/pong functionality to detect connects/disconnects instantly.

> Edit "clientName" in esp_client.ino to either ONE, TWO, THREE or FOUR in order to identify each button.
