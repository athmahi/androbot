//
// Copyright 2015 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// FirebaseDemo_ESP8266 is a sample that demo the different functions
// of the FirebaseArduino API.

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST "androbot-1f671.firebaseio.com"
#define FIREBASE_AUTH "hUsGMLUDD7cdrMuCYVBca4ENHzzu0bS8gwxybMqn"
#define WIFI_SSID "PMK"
#define WIFI_PASSWORD "prasanna"

void setup() {
  Serial.begin(115200);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
   
}

unsigned int byte_sent = 0;

unsigned char byte_recv = 0;

unsigned int heading = 0;
int n = 0, x = 0, y = 0;

void loop() {

  
  n = Firebase.getInt("botOn");
  byte_sent = n * 100;

  n = Firebase.getInt("autoMode");
  byte_sent = byte_sent + (n * 10);

  n = Firebase.getInt("directionfromApp");

  byte_sent = byte_sent + n;

  //Serial.print(" firebase data is - ");
  Serial.write(byte_sent);

  if(Serial.available())
  {

    while(Serial.available())
    {
      byte_recv = Serial.read();
      
    }

    Serial.end();
    Serial.begin(115200);
  }

  heading = (int)byte_recv;

  if(heading == 1)
  {
    x = Firebase.getInt("X");
    x = x + 1;
    Firebase.setInt("X", x);
    Firebase.setInt("pointSet", 1);
  }

  if(heading == 2)
  {
    y = Firebase.getInt("Y");
    y = y + 1;
    Firebase.setInt("Y", y);
    Firebase.setInt("pointSet", 1);
  }

  if(heading == 3)
  {
    x = Firebase.getInt("X");
    x = x - 1;
    Firebase.setInt("X", x);
    Firebase.setInt("pointSet", 1);
  }

  if(heading == 4)
  {
    y = Firebase.getInt("Y");
    y = y - 1;
    Firebase.setInt("Y", y);
    Firebase.setInt("pointSet", 1);
  }
  
  
  Firebase.setInt("pointSet", 0);
  Serial.flush();
  delay(100);
  
 
}
