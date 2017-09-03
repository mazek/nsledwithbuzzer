#include "config.h"

#include <ESP8266WiFi.h>

int sound_pin = 13;
int led_pin = 12;

void setup() {
  pinMode(12, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void start_sound (int freq, int sound_delay) {
  analogWriteFreq(100);
  analogWrite(sound_pin, freq);
  delay(sound_delay);
  analogWrite(sound_pin, 0);
}

void loop() {

  digitalWrite(led_pin, LOW);
  delay(500);                      
  start_sound(1500, 300);
  digitalWrite(led_pin, HIGH);  
  delay(1000);                      

}
