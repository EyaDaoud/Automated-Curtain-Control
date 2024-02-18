#include <ESP32Servo.h>
#include "RTClib.h"
#include <Wire.h>
#include <WiFi.h>
#include "PubSubClient.h"

#define DEVICE_LABEL "esp32_andika_rtc"
#define RED_PIN 27
#define GREEN_PIN 14
#define BLUE_PIN 26

const int servoPin = 23; // Define the servo motor pin
const int ldrPin = 32;   // Broche du capteur LDR
const int pirPin = 5;   // Broche du capteur PIR
int pirState = LOW;
int lastPirState = LOW;
int previousLdrvalue = 0;

bool etat = false; // fermé
bool automatique = true;
bool ouv = false ;
bool manuel = false;
bool oui = false;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "broker.hivemq.com";
int port = 1883;
String stMac;
char mac[50];
char clientId[50];

String msgStr = "";      // MQTT message buffer
int lum;
int timeOuv;
int timeFerm;

WiFiClient espClient;
PubSubClient client(espClient);

RTC_DS1307 rtc;
Servo myservo; // Create a Servo object



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
  pinMode(ldrPin, INPUT); // Déclaration de la broche de du LDR
  myservo.attach(servoPin, 500, 2400);  // Attach the servo to the pin
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
      client.subscribe("lum");
      //client.subscribe("lumi");
      client.subscribe("time_ouv");
      client.subscribe("time_ferm");
      client.subscribe("possible");

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

  if (String(topic) == "manuell") {
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

  if (String(topic) == "auto") {
    if (data == "ON") {
      automatique = true;
      manuel = false;
    }
  }


  if (String(topic) == "time_ouv") {
    timeOuv = data.toInt();
    Serial.println(timeOuv);
  }

  if (String(topic) == "time_ferm") {
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
    sprintf(buffer, "%d", currentLdrValue); // Convert integer to string
    client.publish("lum", buffer);
    previousLdrvalue = currentLdrValue;

  }


  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);


  // Check for changes in the PIR sensor state
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
        Serial.println("ouvert");
        msgStr = "ouvert";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";
        delay(1500);
        analogWrite(GREEN_PIN, 0);

      } else if (now.hour() >= 12 && now.hour() < 14  && now.minute() >= 0
                 && now.second() >= 0)
      {
        if (etat == false && (analogRead(ldrPin) - 4063) < lum ) {
          //while (analogRead(ldrPin) < 500) {
          for (int pos = 180; pos >= 0; pos -= 1)
          {
            if (abs(analogRead(ldrPin) - 4063) < lum ) {
              delay(30);
              myservo.write(pos);
              Serial.println("yet7al");
              Serial.println(analogRead(ldrPin));
            }
            //delay(30);
          }
          etat = true;
          analogWrite(BLUE_PIN, 255);
          Serial.println("semi ouvert");
          msgStr = "semi ouvert";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("etat", msg);
          msgStr = "";
          delay(2000);
          analogWrite(BLUE_PIN, 0);
        }
        else if (etat == false && (analogRead(ldrPin) - 4063) > lum ) {
          Serial.println("les rideaux sont déja fermés");
          msgStr = "les rideaux sont déja fermés";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("possible", msg);
          msgStr = "";

        }
        else if (etat == true && (analogRead(ldrPin) - 4063) > lum ) {
          //while (analogRead(ldrPin) > 500){
          for (int pos = 0; pos <= 180; pos += 1) {
            if (abs(analogRead(ldrPin) - 4063) > lum ) {
              delay(30);
              myservo.write(pos);
              Serial.println("yetsaker");
              Serial.println(analogRead(ldrPin));
            }
          }
          etat = true;
          analogWrite(BLUE_PIN, 255);
          Serial.println("semi ouvert");
          msgStr = "semi ouvert";
          byte arrSize = msgStr.length() + 1;
          char msg[arrSize];
          Serial.print("PUBLISH DATA: ");
          Serial.println(msgStr);
          msgStr.toCharArray(msg, arrSize);
          client.publish("etat", msg);
          msgStr = "";
          delay(2000);
          analogWrite(BLUE_PIN, 0);
        }
        else if (etat == true && (analogRead(ldrPin) - 4063) < lum ) {
          Serial.println("les rideaux sont déja ouverts");
          msgStr = "les rideaux sont déja ouverts";
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
        Serial.println("fermé");
        msgStr = "fermé";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";
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
        msgStr = "aucune personne : fermé";
        byte arrSize = msgStr.length() + 1;
        char msg[arrSize];
        Serial.print("PUBLISH DATA: ");
        Serial.println(msgStr);
        msgStr.toCharArray(msg, arrSize);
        client.publish("etat", msg);
        msgStr = "";
        delay(1500);
        analogWrite(RED_PIN, 0);
      }
    }
    lastPirState = pirState;
  }


  if (manuel == true && automatique == false) {
    if (  ouv == true) {
      for (int pos = 180; pos >= 0; pos -= 1) {
        myservo.write(pos);
        delay(30);
      }
      etat = true;
      analogWrite(GREEN_PIN, 255);
      Serial.println("ouvert");
      msgStr = "ouvert";
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
    }
    if (ouv == false  && oui == true )
    {
      for (int pos = 0; pos <= 180; pos += 1) {
        analogWrite(RED_PIN, 255);
        myservo.write(pos);
        delay(30);
      }
      etat = false;
      msgStr = "les ridaux sont fermés";
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
  delay(1000); // This delay speeds up the simulation
}


