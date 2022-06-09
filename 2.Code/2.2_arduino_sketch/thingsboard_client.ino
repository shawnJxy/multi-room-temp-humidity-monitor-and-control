#include "arduino_secrets.h"
// ArduinoJson - Version: Latest 
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// PubSubClient - Version: Latest 
#include <PubSubClient.h>

// ThingsBoard - Version: Latest 
#include <ThingsBoard.h>

// ArduinoHttpClient - Version: Latest 
#include <ArduinoHttpClient.h>

// Servo - Version: Latest
#include <Servo.h>

// DHT sensor library - Version: Latest
#include <DHT.h>
#include <DHT_U.h>

// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define DHTPIN 5 //digital pin connected to DHT sensor
#define DHTTYPE DHT11 //define DHT sensor type

Servo servo1;

/*
  ArduinoMqttClient - WiFi Simple Sender

  This example connects to a MQTT broker and publishes a message to
  a topic once a second.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board

  This example code is in the public domain.
*/

#include <ArduinoMqttClient.h>
#include <WiFi101.h>

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD    9600

///////you may enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient wifiClient;
//MqttClient mqttClient(wifiClient);

//make sure the TOKEN matches with the acess token generated on ThingsBoard
#define TOKEN               "oMwggDe4HTVGjRZNh1wQ"
// ThingsBoard server instance.
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD    115200

// Initialize ThingsBoard instance
ThingsBoard tb(wifiClient);

const long interval = 1000;
unsigned long previousMillis = 0;

// Main application loop delay
int quant = 20;
// Time passed after temperature/humidity data was sent, milliseconds.
int send_passed = 0;
// Period of sending a temperature/humidity data.
int send_delay = 2000;
// Set to true if application is subscribed for the RPC messages.
bool subscribed = false;

//define dht and servo motor devices 
DHT dht(DHTPIN, DHTTYPE);

int servopin = 2;

//set variable to store cloud commands
bool switch_Set = false;
//set variable to store last switch state
bool switch_last = false;
//set variable to store last switch state
bool switch_Set_local = false;
// Time passed after temperature/humidity data was sent, milliseconds.
int switch_Set_passed = 0;
// Period of sending a temperature/humidity data.
int switch_Set_delay = 60000;

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processDelayChange(const RPC_Data &data)
{
  Serial.println("Received the set delay RPC method");

  // Process data
  //tmp = String(data);
  //Serial.println(tmp);
  switch_Set = data;
  switch_Set_passed = 0;

  return RPC_Response(NULL, switch_Set);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processGetDelay(const RPC_Data &data)
{
  Serial.println("Received the get value method");

  return RPC_Response(NULL, switch_last);
}

// RPC handlers
RPC_Callback callbacks[] = {
  { "setValue",         processDelayChange },
  { "getValue",         processGetDelay },
};

void setup() {
  //servo pin setup
  servo1.attach(servopin);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //Disable following for running with USB power supply
  //while (!Serial) {
    //; // wait for serial port to connect. Needed for native USB port only
  //}

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();
  
  dht.begin();
}


void loop() {
  delay(quant);
  send_passed += quant;
  switch_Set_passed += quant;
  
  //check DHT11 reading
  float h = dht.readHumidity();        // read humidity
  float t = dht.readTemperature();     // read temperature
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // for debug use
  // Serial.print(h);
  // Serial.print("\t");
  // Serial.println(t);
  
  //auto fan control
  if (switch_Set_passed > switch_Set_delay) {
    if (h > 65) {
      switch_Set_local = true;
    } else if (h <= 50 ){
      switch_Set_local = false;
    } else {
      ;
    }
    if (switch_Set_local != switch_last){
      switchControl(switch_Set_local);
      switch_last = switch_Set_local;
    } else {
      ;
    }
  } else {
    //Serial.println(switch_Set);
    //Serial.println(switch_last);
    if (switch_Set == switch_last ) {
      ;
    } else {
      Serial.println("Manual Override");
      switchControl(switch_Set);
    }
  }
  
  // // Reconnect to WiFi, if needed
  // if (WiFi.status() != WL_CONNECTED) {
  //   reconnect();
  //   return;
  // }

  // Reconnect to ThingsBoard, if needed
  if (!tb.connected()) {
    subscribed = false;

    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

    // Subscribe for RPC, if needed
  if (!subscribed) {
    Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // callbacks as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    Serial.println("Subscribe done");
    subscribed = true;
  }

    // Check if it is a time to send DHT22 temperature and humidity
  if (send_passed > send_delay) {
    Serial.println("Sending data...");

    // Uploads new telemetry to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 
    // for more details

    if (isnan(h) || isnan(t) ) {
      Serial.println(F("Failed to read from DHT sensor!"));
    } else {
      tb.sendTelemetryFloat("temperature", t);
      tb.sendTelemetryFloat("humidity", h);
    }

    send_passed = 0;
  }

  // Process messages
  tb.loop();
  
  //delay(2000);
}

void switchControl(bool cmd) {
  Serial.println(cmd);
  if (cmd == 1) {
    servo1.write(145);
    switch_last = true;
    Serial.println("Bath Fan Switch On");
  } else if (cmd == 0){
    servo1.write(10);
    switch_last = false;
    Serial.println("Bath Fan Switch Off");
  }
}