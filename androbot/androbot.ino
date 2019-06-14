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
#define FIREBASE_HOST "androbot-1f671.firebaseio.com"             // project host link
#define FIREBASE_AUTH "hUsGMLUDD7cdrMuCYVBca4ENHzzu0bS8gwxybMqn"  // project authentication link
#define WIFI_SSID "rsurya07"                                      // wifi SSID
#define WIFI_PASSWORD "1904Nansu"                                 // wifi password

void setup() {
  
  Serial.begin(115200);                                          // set UART baudrate

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                         // conntect to wifi
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());                               // print the IP
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);                 // authenticate to firebase

  Firebase.setInt("X", 500);                                    // set the starting cordinates to 500, 500
  Firebase.setInt("Y", 500);
   
}

unsigned int byte_sent = 0;

unsigned char byte_recv = 0;

unsigned int heading = 0;
int n = 0, x = 0, y = 0;

void loop() {

  
  n = Firebase.getInt("botOn");                             // read the ON/OFF switch data fromfirebase
  byte_sent = n * 100;                                      // multiply this data by 100

  n = Firebase.getInt("autoMode");                          // read the mode of operation
  byte_sent = byte_sent + (n * 10);                         // multiply by 10 and add this to byte_sent 

  n = Firebase.getInt("directionfromApp");                  // read the direction from app

  byte_sent = byte_sent + n;                                // add this data to unit's place of data

  //Serial.print(" firebase data is - ");
  
  Serial.write(byte_sent);                                  // send the byte of data to nexys4 board

  if(Serial.available())                                    // 1st latching of serial port
  {

    while(Serial.available())                               // 2nd latching of serial port
    {
      byte_recv = Serial.read();                            // read the data sent from Nexys4 board
      
    }

    Serial.end();                                           // end the serial connection to clear Rx buffer
    Serial.begin(115200);                                   // initialise the serial port and assign baudrate
  }

  heading = (int)byte_recv;

  if(heading == 1)                                          // if the heading is east
  {
    x = Firebase.getInt("X");                               // read the previous X cordinate
    if( x<1000 && x>0)                                      // check if bot is crossing the map boundry
      x = x + 20;                                           // increment the pixelcount by 20
    Firebase.setInt("X", x);                                // update the new X cordinate
    Firebase.setInt("pointSet", 1);                         // set update bit to 1 so that android app
                                                            // knows new data is being written
  }

  if(heading == 2)                                          // if heading is north
  {   
    y = Firebase.getInt("Y");                               // read the previous Y cordinate
    if( y<1000 && y>0)                                      // check if bot is crossing the map boundry
      y = y - 20;                                           // decrement the pixelcount by 20
    Firebase.setInt("Y", y);                                // update the new Y cordinate
    Firebase.setInt("pointSet", 1);                         // set update bit to 1 so that android app
                                                            // knows new data is being written
  }

  if(heading == 3)                                          // if heading is West
  {
    x = Firebase.getInt("X");                               // read the previous X cordinate
    if( x<1000 && x>0)                                      // check if bot is crossing the map boundry
      x = x - 20;                                           // decrement the pixelcount by 20
    Firebase.setInt("X", x);                                // update the new X cordinate
    Firebase.setInt("pointSet", 1);                         // set update bit to 1 so that android app
                                                            // knows new data is being written
  }

  if(heading == 4)                                          // if heading is south
  {
    y = Firebase.getInt("Y");                               // read the previous Y cordinate
    if( y<1000 && y>0)                                      // check if bot is crossing the map boundry
      y = y + 20;                                           // increment the pixelcount by 20
    Firebase.setInt("Y", y);                                // update the new Y cordinate
    Firebase.setInt("pointSet", 1);                         // set update bit to 1 so that android app
                                                            // knows new data is being written
  }

  if(heading == 5)                                          // if data received is stop bit
  {
    continue;                                               // continue statement is used because 
                                                            // we don't want toupdate cordinates
  }

  Serial.flush();                                         // flush TX buffer of uart
  delay(100);                                             // delay of 100 ms
  
 
}
