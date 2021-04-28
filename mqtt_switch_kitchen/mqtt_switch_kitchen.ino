/*
  Projet d'apprentissage d'un objet connecté (IoT)  pour réaliser une sonde de température
  ESP8266 + DHT22 + LED + MQTT + Home-Assistant
  Projets DIY (http://www.projetsdiy.fr) - Mai 2016
  Licence : MIT
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>  // Für OTA
#include <WiFiUdp.h>      // Für OTA
#include <ArduinoOTA.h>   // Für OTA


#define wifi_ssid "mimimimiii"
#define wifi_password "dontcry4meargentina"

#define mqtt_server "192.168.1.102"
#define mqtt_user "thing"      // if exist
#define mqtt_password "5m4r72552"  //idem
#define mqtt_client "kitchen_sw_media"  //client name

#define switch_topic "kitchen_sd_media/cmnd/POWER"  //Topic tst
#define kitchen_sd_topic "kitchen_sd_media/HELP/#"  //Topic tst



// constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 2; // the number of the pushbutton pin
const int buttonPin2 = 5; // the number of the Ledoutput
//const int buttonPin4 = 4;     // the number of the pushbutton pin

// variables will change:
int buttonState1 = 0;         // variable for reading the pushbutton status
//int buttonState4 = 0;
int lastButtonState1 = 0;
//int lastButtonState4 = 0;

String PL = "";
//Buffer to decode MQTT messages
char message_buff[100];

long lastMsg = 0;   
long lastRecu = 0;
bool debug = false;  //Display log message if True

// Create abjects     
WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin1, INPUT_PULLUP); // initialize the pushbutton pin as an input :
  pinMode(buttonPin2, OUTPUT); // initialize the pushbutton pin as an input and pullup:
  Serial.begin(9600);
  setup_wifi();           //Connect to Wifi network
  client.setServer(mqtt_server, 1883);    // Configure MQTT connexion
  client.setCallback(callback);  //MQTT LIsten Funktion
}

//////////////////////////////////////
//Connexion au réseau WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.hostname("kitchen_sw_media");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");
  Serial.print(WiFi.localIP());


  ArduinoOTA.setHostname("kitchen_sw_main");
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


//////////////////////////////////////
//MQTT Connect innk reconnet
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {
      Serial.println("OK");
      client.subscribe(kitchen_sd_topic);
      Serial.println(kitchen_sd_topic);
    } else {
      Serial.print("KO, error : ");
      Serial.println(client.state());
      Serial.println(" Wait 5 secondes before to retry");
      delay(5000);
    }
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {    // MQTT konnektieren
  reconnect();
  }
  ArduinoOTA.handle();
  client.loop();
  buttonState1 = digitalRead(buttonPin1);  // read the state of the pushbutton value:

    // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState1 != lastButtonState1) {
    if (buttonState1 == LOW)  {
      client.publish(switch_topic, "TOGGLE", true);
      Serial.println ("bildschirm state"); 
      Serial.println (buttonState1);

    } 
  lastButtonState1 = buttonState1;
  }
     client.loop(); // Schleife für MQTT
   // int pli = PL.toInt();
  
   if (PL == "ON"){
        digitalWrite(buttonPin2, HIGH);
        Serial.println("Turn on Button 1 led");
        PL = "";
    } 
   if (PL == "OFF"){
        digitalWrite(buttonPin2, LOW);
        Serial.println("Turn off Button 1 led");
        PL = "";
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
