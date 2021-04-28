#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#define wifi_ssid "mimimimiii"
#define wifi_password "dontcry4meargentina"
#define wifi_hostname "kitchenswitch"

#define mqtt_server "192.168.1.102"
#define mqtt_user "thing"      // if exist
#define mqtt_password "5m4r72552"  //idem
#define mqtt_client "kitchen_sw_media"
#define MQTT_topic "kitchen_sd_media/stat/POWER"  //Topic tst


String PL = "";               // Payload from MQTT
int button = 2;             //pinnumber lesbar machen
int tasterlicht = 5;        //pinnumber lesbar machen

boolean buttonActive = false;         //var fuer longpress funktion
boolean longPressActive = false;      //var fuer longpress funktion
long buttonTimer = 0;                 //var fuer longpress funktion
long longPressTime = 1000;             //var fuer longpress funktion


// Create abjects     
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(tasterlicht, OUTPUT); //D1 on WEMOS D1 mini, led from button
  pinMode(button, INPUT_PULLUP); //D4 on WEMOS D1 mini 
  digitalWrite(tasterlicht, HIGH);
  Serial.begin(115200);
  setup_wifi();           //Connect to Wifi network
  client.setServer(mqtt_server, 1883); // Configure MQTT connexion
  client.setCallback(callback);
}

//Connexion au réseau WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.hostname(wifi_hostname);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");
  Serial.print(WiFi.localIP());


  ArduinoOTA.setHostname(wifi_hostname);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



}




//Reconnexion
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {
      Serial.println("OK");
      client.subscribe(MQTT_topic);
      Serial.println(MQTT_topic);
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" Wait 5 secondes before to retry");
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()) {
  reconnect();
  }
  ArduinoOTA.handle();
   client.loop(); // Schleife für MQTT
  if (digitalRead(button) == LOW) {  
      if (buttonActive == false) {  
        buttonActive = true;
        buttonTimer = millis();  
      }  
      if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {  //Hier ist longpress gesetzt
        longPressActive = true;
        client.publish("kitchen/media", "music");
        if (PL == "OFF") {
        client.publish("kitchen_sd_media/cmnd/POWER", "ON"); 
        } 
        digitalWrite(tasterlicht, LOW);
        delay(250);
        digitalWrite(tasterlicht, HIGH);
        delay(250);
        digitalWrite(tasterlicht, LOW);
        delay(250);
        digitalWrite(tasterlicht, HIGH);
      }  
    } else {                      //wird ausgeführt wenn der taster nicht gedrueckt wird
      if (buttonActive == true) {  //schlaufe nur aktiv wenn das erste mal der taster losgelassen wird
        if (longPressActive == true) {  
          longPressActive = false;  
        } else {  
        client.publish("kitchen_sd_media/cmnd/POWER", "TOGGLE");
        delay(50);    
        }  
        buttonActive = false;  
      }  
    }
} 
// ===========================================================
// Callback Funktion von MQTT. Die Funktion wird aufgerufen
// wenn ein Wert empfangen wurde.
// ===========================================================
void callback(char* topic, byte* payload, unsigned int length) {
 // Zähler
 int i = 0;
 // Hilfsvariablen für die Convertierung der Nachricht in ein String
 char message_buff[100];
 
 Serial.println("Message arrived: topic: " + String(topic));
 Serial.println("Length: " + String(length,DEC));
 
 // Kopieren der Nachricht und erstellen eines Bytes mit abschließender \0
 for(i=0; i<length; i++) {
 message_buff[i] = payload[i];
 }
 message_buff[i] = '\0';
 
 // Konvertierung der nachricht in ein String
 String msgString = String(message_buff);
 Serial.println("Payload: " + msgString);
 PL = msgString;
 Serial.print("PayL: " + PL);
}
  


//}
