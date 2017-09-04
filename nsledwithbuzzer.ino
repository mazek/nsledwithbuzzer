#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define DEBUG

#include "config.h"

int min = 80;
int max = 160;

int sound_pin = 13;
int led_pin = 12;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

String response;
int statusCode = 0;

DynamicJsonBuffer  jsonBuffer;

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  delay(10);

  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(ssid);
    
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN(WiFi.localIP());
}

void start_sound (int freq, int sound_delay) {
  analogWriteFreq(freq);
  analogWrite(sound_pin, freq);
  delay(sound_delay);
  analogWrite(sound_pin, 0);
}

void loop() {
  
  DEBUG_PRINTLN("making GET request");
  client.get("/pebble");

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();

  DEBUG_PRINT("statusCode: ");
  DEBUG_PRINTLN(statusCode);

  DEBUG_PRINT("Response: -");
  DEBUG_PRINT(response);
  DEBUG_PRINTLN("-");
      
  JsonObject& _data = jsonBuffer.parseObject(response);

  if (!_data.success()) {
    DEBUG_PRINTLN("parseObject() failed:( ");
    delay(5000);
    return;
  }
  else {
    DEBUG_PRINTLN("parseObject() success! ");
  }
 
  String cur_time_s = _data["status"][0]["now"];
  String read_time_s = _data["bgs"][0]["datetime"];

  DEBUG_PRINT("1. cur_time_s: ");
  DEBUG_PRINT(cur_time_s);
  
  DEBUG_PRINT(", read_time_s: ");
  DEBUG_PRINTLN(read_time_s);
  
  cur_time_s = cur_time_s.substring(0, cur_time_s.length()-3);
  read_time_s = read_time_s.substring(0, read_time_s.length()-3);

  DEBUG_PRINT("2. cur_time_s: ");
  DEBUG_PRINT(cur_time_s);
  
  DEBUG_PRINT(", read_time_s: ");
  DEBUG_PRINTLN(read_time_s);
  
  unsigned long  cur_time = cur_time_s.toFloat();
  unsigned long  read_time = read_time_s.toFloat();
  
  DEBUG_PRINT("3. cur_time: ");
  DEBUG_PRINT(cur_time);
  
  DEBUG_PRINT(", read_time: ");
  DEBUG_PRINTLN(read_time);

  unsigned long parakeet_last_seen = cur_time - read_time ;
  DEBUG_PRINT("I seen parakeet more then ");
  DEBUG_PRINT(parakeet_last_seen);
  DEBUG_PRINTLN(" seconds.");
  
  if (parakeet_last_seen > 900) {
    DEBUG_PRINTLN("I lost parakeet signal :(");
  }
  else {
    // Parakeet operational.
    DEBUG_PRINTLN("I got parakeet signal :)");
    unsigned long bwpo = _data["bgs"][0]["sgv"];
    DEBUG_PRINT("BWPO: ");
    DEBUG_PRINTLN(bwpo);
    long bgdelta = _data["bgs"][0]["bgdelta"];
    DEBUG_PRINT("BGDELTA: ");
    DEBUG_PRINTLN(bgdelta);

    
    if (bgdelta > 0) {
      // Sugar is growing.
      //      bgdelta_s = "+%s" % bgdelta
    }
    else {
      // Sugar is dropping.
      //      bgdelta_s = "%s" % bgdelta
    }
    
    if (bwpo < min) {
      // Sugar below minimum level.
      DEBUG_PRINT("Sugar: ");
      DEBUG_PRINT(bwpo);
      DEBUG_PRINT(", change:  ");
      DEBUG_PRINTLN(bgdelta);
      DEBUG_PRINTLN("Sugar below minimum level.");
      for (int x=0; x<3; x++ ) {
        digitalWrite(led_pin, HIGH);
        delay(500);                      
        start_sound(100, 300);
        digitalWrite(led_pin, LOW);  
        delay(1000);
      }     
    }
    else if (bwpo > max) {
      // Sugar above maximum level.
      DEBUG_PRINT("Sugar: ");
      DEBUG_PRINT(bwpo);
      DEBUG_PRINT(", change:  ");
      DEBUG_PRINTLN(bgdelta);
      DEBUG_PRINTLN("Sugar above maximum level.");
      for (int x=0; x<3; x++ ) {
        digitalWrite(led_pin, HIGH);
        delay(500);                      
        start_sound(1000, 300);
        digitalWrite(led_pin, LOW);  
        delay(1000);
      } 
    }
    else {
      // Sugar ok. 
    }
    DEBUG_PRINTLN("-----");
    DEBUG_PRINTLN(bwpo);
    DEBUG_PRINTLN("-----");
  }  
  DEBUG_PRINTLN("Wait fiveteen minutes");
  delay(900000);
  // Blink - I'm alive :)
  for (int x=0; x<3; x++) {
    digitalWrite(led_pin, HIGH);
    delay(200);                      
    digitalWrite(led_pin, LOW);  
    delay(200);
  }
}
