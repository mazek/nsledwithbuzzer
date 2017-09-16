#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define DEBUG

#include "config.h"
#include "tools.h"


int min = 80;
int max = 160;

int redPin = 4;
int greenPin = 5;
int bluePin = 16;

// 0 - unknown, 1 - very low - red, 2 - too low - yellow, 3 - ok - green, 4 - too high - blue
int sugar_color = 0;

int sound_pin = 13;
int led_pin = 12;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

long rssi = 0;

String response;
int statusCode = 0;

char cur_time_s[13];
char read_time_s[13];

unsigned long  cur_time;
unsigned long  read_time;

unsigned long sugar_level;
long sugar_level_delta;

unsigned long parakeet_last_seen;


void setup() {
  pinMode(led_pin, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT); 
    
  Serial.begin(115200);
  delay(10);

  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(ssid);
    
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
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

void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

void parse_json(unsigned long&  cur_time, unsigned long&  read_time, unsigned long& sugar_level, long& sugar_level_delta, unsigned long& parakeet_last_seen) {

  //DynamicJsonBuffer  jsonBuffer;
  StaticJsonBuffer<1000>  jsonBuffer;
    
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

//  int a = Node.Heap();
//  DEBUG_PRINT("Free RAM: ");
//  DEBUG_PRINTLN(a);


//uint32_t free = system_get_free_heap_size();
      
  JsonObject& _data = jsonBuffer.parseObject(response);

  if (!_data.success()) {
    DEBUG_PRINTLN("parseObject() failed:( ");
    return;
  }
  else {
    DEBUG_PRINTLN("parseObject() success! ");
  }

  strncpy(cur_time_s, _data["status"][0]["now"],10);
  strncpy(read_time_s, _data["bgs"][0]["datetime"],10);  
  
  DEBUG_PRINT("1. cur_time_s: ");
  DEBUG_PRINT(cur_time_s);
  
  DEBUG_PRINT(", read_time_s: ");
  DEBUG_PRINTLN(read_time_s);
  
  cur_time = atof(cur_time_s);
  read_time = atof(read_time_s);
  
  DEBUG_PRINT("2. cur_time: ");
  DEBUG_PRINT(cur_time);
  
  DEBUG_PRINT(", read_time: ");
  DEBUG_PRINTLN(read_time);

  parakeet_last_seen = cur_time - read_time ;
  DEBUG_PRINT("I seen parakeet more then ");
  DEBUG_PRINT(parakeet_last_seen);
  DEBUG_PRINTLN(" seconds.");

  if (parakeet_last_seen > 900) {
    DEBUG_PRINTLN("I lost parakeet signal :(");
  }
  else {
    // Parakeet operational.
    DEBUG_PRINTLN("I got parakeet signal :)");
    sugar_level = _data["bgs"][0]["sgv"];
    DEBUG_PRINT("Sugar level: ");
    DEBUG_PRINTLN(sugar_level);
    sugar_level_delta = _data["bgs"][0]["bgdelta"];
    DEBUG_PRINT("sugar_level_delta: ");
    DEBUG_PRINTLN(sugar_level_delta);
  }
  //jsonBuffer.clear();
  return;
}

void loop() {

  DEBUG_PRINT("Free Heap: ");
  DEBUG_PRINTLN(ESP.getFreeHeap());
  
  rssi = WiFi.RSSI();  
  DEBUG_PRINT("RSSI: ");
  DEBUG_PRINTLN(rssi);
  
  parse_json(cur_time, read_time, sugar_level, sugar_level_delta, parakeet_last_seen);
  
  if (parakeet_last_seen > 900) {
    DEBUG_PRINTLN("I lost parakeet signal :(");
  }
  else {
    // Parakeet operational.
    DEBUG_PRINTLN("I got parakeet signal :)");
    
    if (sugar_level_delta > 0) {
      // Sugar is growing.
      //      sugar_level_delta_s = "+%s" % sugar_level_delta
    }
    else {
      // Sugar is dropping.
      //      sugar_level_delta_s = "%s" % sugar_level_delta
    }

    DEBUG_PRINT("Sugar: ");
    DEBUG_PRINT(sugar_level);
    DEBUG_PRINT(", change:  ");
    DEBUG_PRINTLN(sugar_level_delta);
    
    if (sugar_level <= min-20) {
      // Sugar way below minimum level.

      DEBUG_PRINTLN("Sugar below minimum level.");
      for (int x=0; x<5; x++ ) {
        digitalWrite(led_pin, HIGH);
        delay(500);                      
        start_sound(50, 300);
        digitalWrite(led_pin, LOW);  
        delay(800);
      }   
      sugar_color = 1;  
    }    
    else if (sugar_level > min-20 and sugar_level < min) {
      // Sugar below minimum level.
      DEBUG_PRINTLN("Sugar below minimum level.");
      for (int x=0; x<3; x++ ) {
        digitalWrite(led_pin, HIGH);
        delay(500);                      
        start_sound(100, 300);
        digitalWrite(led_pin, LOW);  
        delay(800);
      }
      sugar_color = 2;     
    }
    else if (sugar_level > max) {
      // Sugar above maximum level.
      DEBUG_PRINTLN("Sugar above maximum level.");
      for (int x=0; x<3; x++ ) {
        digitalWrite(led_pin, HIGH);
        delay(500);                      
        start_sound(1000, 300);
        digitalWrite(led_pin, LOW);  
        delay(800);
      }
      sugar_color = 4; 
    }
    else {
      // Sugar ok.
      sugar_color = 3; 
    }
    DEBUG_PRINTLN("-----");
    DEBUG_PRINTLN(sugar_level);
    DEBUG_PRINTLN("-----");
  }  
  DEBUG_PRINTLN("Wait fiveteen minutes");
  
  
  for (int x=0; x<3; x++) { // Three sessions.
    for (int z=0; z<30; z++){ // Blink the led with color according to sugar level.
      switch (sugar_color) {
        case 0:
          setColor(random(255), random(255), random(255));
          break;
        case 1:
          setColor(255, 0, 0);
          break;
        case 2:
          setColor(255, 255, 0);
          break;
        case 3:
          setColor(0, 15, 0);
          break;
        case 4:
          setColor(0, 0, 255);
          break;                              
      }
      delay(300);      
      setColor(0, 0, 0);  
      delay(300);
    }
         
    for (int y=0; y<x+1; y++) { // Blink the led 1, 2 or 3 times to show where we are.
      digitalWrite(led_pin, HIGH);
      delay(300);                      
      digitalWrite(led_pin, LOW);  
      delay(300);
    }
  }
}
