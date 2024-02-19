#include <ESP32Servo.h>
#include "RTClib.h"
#include <Wire.h>
#include <WiFi.h>
#include "PubSubClient.h"

#define DEVICE_LABEL "esp32_andika_rtc"
#define RED_PIN 27
#define GREEN_PIN 14
#define BLUE_PIN 26

const int servoPin = 23; 
const int ldrPin = 32;   
const int pirPin = 5;  

int pirState = LOW;
int lastPirState = LOW;
int previousLdrvalue = 0;
int lum;
int timeOuv;
int timeFerm;

bool etat = false; 
bool automatique = true;
bool ouv = false ;
bool manuel = false;
bool oui = false;

String msgStr = ""; // MQTT message buffer
String msgStr1 = "";     

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "broker.hivemq.com";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DS1307 rtc;
Servo myservo; 



void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0));

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiConnect();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  stMac = WiFi.macAddress();
  stMac.replace(":", "_");
  Serial.println(stMac);

  client.setServer(mqttServer, port);
  client.setCallback(callback);

  Serial.println("LDR readings...");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(ldrPin, INPUT); 
  myservo.attach(servoPin, 500, 2400); 
  rtc.begin();
  DateTime now = rtc.now();

}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}


void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println("MQTT connected");
      client.subscribe("etat");
      client.subscribe("manuell");
      client.subscribe("auto");
      client.subscribe("time_ouv");
      client.subscribe("time_ferm");
      client.subscribe("possible");
      client.subscribe("mode");

      Serial.println("Topic Subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // wait 5sec and retry
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String data = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println();

  if (String(topic) == "lum") {
    lum = data.toInt();
    Serial.println(lum);
  }
  
  if (String(topic) == "manuell") { //manual mode
    if (data == "ON") {
      automatique = false;
      manuel = true;
      ouv = true;
    } else if (data == "OFF") {
      automatique = false;
      manuel = true;
      ouv = false;
      oui = true;
    }
  }

  if (String(topic) == "auto") { //automatic mode
    if (data == "ON") {
      automatique = true;
      manuel = false;
    }
  }


  if (String(topic) == "time_ouv") { // Opening Hour
    timeOuv = data.toInt();
    Serial.println(timeOuv);
  }

  if (String(topic) == "time_ferm") { //Closing Hour
    timeFerm = data.toInt();
    Serial.println(timeFerm);
  }
}

void loop() {
  delay(10);
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  pirState = digitalRead(pirPin);

  int currentLdrValue = abs(analogRead(ldrPin) - 4063);
  if (currentLdrValue != previousLdrvalue ) {
    Serial.print("LDR Value :");
    Serial.println(currentLdrValue);
    char buffer[16];
    sprintf(buffer, "%d", currentLdrValue); 
    client.publish("lum", buffer);
    previousLdrvalue = currentLdrValue;

  }


  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);


  
  if (pirState != lastPirState && automatique == true) {
    if (pirState == HIGH ) {
      // Motion detected
      if (now.hour() >= timeOuv && now.hour() < 12 && now.minute() >= 0
          && now.second() >= 0 && etat == false )
      { 
        for (int pos = 180; pos >= 0; pos -= 1) {
          myservo.write(pos);
          delay(20);
        }
        etat = true;
        analogWrite(GREEN_PIN, 255);
        Serial.println("Temps d'ouverture dépassé: ouvert");
        msgStr = "Opened";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";

        msgStr1 = "Opening Hour Exceeded";
          byte arrSize1 = msgStr1.length() + 1;
      char msg1[arrSize1];
      Serial.print("PUBLISH DATA: ");
      Serial.println(msgStr1);
      msgStr1.toCharArray(msg1, arrSize1);
      client.publish("possible", msg1);
      msgStr1 = "";

        delay(1500);
        analogWrite(GREEN_PIN, 0);

      } else if (now.hour() >= 12 && now.hour() < timeFerm  && now.minute() >= 0
                 && now.second() >= 0)
      { Serial.println(etat);
      Serial.println(lum);
        if (etat == false && abs(analogRead(ldrPin) - 4063) < lum ) {
          // the amount of room brightness is less than the brightness required
          // and the curtains are closed
          for (int pos = 180; pos >= 0; pos -= 1)
          {

            if (abs(analogRead(ldrPin) - 4063) < lum ) {
              delay(30);
              myservo.write(pos);
              Serial.println("Opening Loading");
              Serial.println(analogRead(ldrPin));
            }
            
          }
          etat = true;
          analogWrite(BLUE_PIN, 255);
          Serial.println("semi ouvert1");
          msgStr = "Opened";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("etat", msg);
          msgStr = "";

          msgStr1 = "Brightness Required Attained";
          byte arrSize1 = msgStr1.length() + 1;
          char msg1[arrSize1];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr1);
          msgStr1.toCharArray(msg1, arrSize1);
          client.publish("possible", msg1);
          msgStr1 = "";

          delay(2000);
          analogWrite(BLUE_PIN, 0);
        }
        else if (etat == false && abs(analogRead(ldrPin) - 4063) > lum ) {
           // the amount of room brightness is more than the brightness required
          // and the curtains are closed
          Serial.println("les rideaux sont déja fermés");
          msgStr = "Curtains Already Closed";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("possible", msg);
          msgStr = "";

        }
        else if (etat == true && abs(analogRead(ldrPin) - 4063) > lum ) {
          // the amount of room brightness is more than the brightness required
          // and the curtains are opened
          for (int pos = 0; pos <= 180; pos += 1) {
            if (abs(analogRead(ldrPin) - 4063) > lum ) {
              delay(30);
              myservo.write(pos);
              Serial.println("Closing Loading");
              Serial.println(analogRead(ldrPin));
            }
          }
          etat = true;
          analogWrite(BLUE_PIN, 255);
          Serial.println("semi ouvert");
          msgStr = "Opened";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("etat", msg);
          msgStr = "";

           msgStr1 = "Brightness Required Attained";
          byte arrSize1 = msgStr1.length() + 1;
          char msg1[arrSize1];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr1);
          msgStr1.toCharArray(msg1, arrSize1);
          client.publish("possible", msg1);
          msgStr1 = "";

          delay(2000);
          analogWrite(BLUE_PIN, 0);
        }
        else if (etat == true && abs(analogRead(ldrPin) - 4063) < lum ) {
          // the amount of room brightness is less than the brightness required
          // and the curtains are opened
          Serial.println("les rideaux sont déja ouverts");
          msgStr = "Curtains Already Opened";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("possible", msg);
          msgStr = "";

        }

      } else {
        if ( etat == true) {
          for (int pos = 0; pos <= 180; pos += 1) {
            myservo.write(pos);
            delay(20);
          }
        }
        analogWrite(RED_PIN, 255);
        Serial.println("fermé : heure de fermeture dépassée");
        msgStr = "Closed";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";

        msgStr1 = "Closing Hour Attained";
        byte arrSize1 = msgStr1.length() + 1;
        char msg1[arrSize1];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr1);
        msgStr1.toCharArray(msg1, arrSize1);
        client.publish("possible", msg1);
        msgStr1 = "";
        
        delay(1500);
        analogWrite(RED_PIN, 0);
      }

    } else {
      // Motion not detected
      delay(7000);
      if (etat == true)
      {
        for (int pos = 0 ; pos <= 180; pos += 1)
        {
          myservo.write(pos);
          delay(15);
        }

        etat = false;
        analogWrite(RED_PIN, 255);
        Serial.println("aucune personne : fermé");
        msgStr = "Closed";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";

        msgStr1 = "No Motion Detected";
        byte arrSize1 = msgStr1.length() + 1;
        char msg1[arrSize1];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr1);
        msgStr1.toCharArray(msg1, arrSize1);
        client.publish("possible", msg1);
        msgStr1 = "";

        delay(1500);
        analogWrite(RED_PIN, 0);
      }
    }
    lastPirState = pirState;
  }


  if (manuel == true && automatique == false) { // Manual mode activated 
       Serial.println("Mode Manuel Activé");

      msgStr1 = "Manual Mode Activated";
      byte arrSize1 = msgStr1.length() + 1;
      char msg1[arrSize1];
      Serial.print("PUBLISH DATA: ");
      Serial.println(msgStr1);
      msgStr1.toCharArray(msg1, arrSize1);
      client.publish("mode", msg1);
      msgStr1 = "";

    if ( ouv == true) {

      for (int pos = 180; pos >= 0; pos -= 1) {
        myservo.write(pos);
        delay(30);
      }
      etat = true;
      analogWrite(GREEN_PIN, 255);
      Serial.println("ouvert");
      msgStr = "Opened";
      byte arrSize = msgStr.length() + 1;
      char msg[arrSize];
      Serial.print("PUBLISH DATA: ");
      Serial.println(msgStr);
      msgStr.toCharArray(msg, arrSize);
      client.publish("etat", msg);
      msgStr = "";
    
      delay(1500);
      analogWrite(GREEN_PIN, 0);
      manuel = false;
      oui = false;
      lastPirState = HIGH;
    }
    if (ouv == false  && oui == true )
    {
      for (int pos = 0; pos <= 180; pos += 1) {
        analogWrite(RED_PIN, 255);
        myservo.write(pos);
        delay(30);
      }
      etat = false;
      msgStr = "Closed";
      byte arrSize = msgStr.length() + 1;
      char msg[arrSize];
      Serial.print("PUBLISH DATA: ");
      Serial.println(msgStr);
      msgStr.toCharArray(msg, arrSize);
      client.publish("etat", msg);
      msgStr = "";

      delay(1500);
      analogWrite(RED_PIN, 0);
      oui = false ;
    }
  }

  if (manuel == false && automatique == true){ //Automatic Mode Activated
    Serial.println("Mode automatique Activé");
    msgStr1 = "Automatic Mode Activated";
      byte arrSize1 = msgStr1.length() + 1;
      char msg1[arrSize1];
      Serial.print("PUBLISH DATA: ");
      Serial.println(msgStr1);
      msgStr1.toCharArray(msg1, arrSize1);
      client.publish("mode", msg1);
      msgStr1 = "";

  }
  delay(1000); 
}


