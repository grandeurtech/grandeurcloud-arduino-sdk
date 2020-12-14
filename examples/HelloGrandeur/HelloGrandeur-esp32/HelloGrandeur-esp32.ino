/**
 * @file HelloGrandeur-esp32.ino
 * @date 24.03.2020
 * @author Grandeur Technologies
 *
 * Copyright (c) 2019 Grandeur Technologies LLP. All rights reserved.
 * This file is part of the Arduino SDK for Grandeur.
 *
 * Grandeur.h is used for device's communication to Grandeur.
 * WiFi.h is used for handling device's WiFi.
 * 
 * This example is the HELLO WORLD of Grandeur.
*/

#include <Grandeur.h>
#include <WiFi.h>

// Device's connection configurations
String apiKey = "YOUR-PROJECT-APIKEY";
String deviceID = "YOUR-DEVICE-ID";
String token = "YOUR-ACCESS-TOKEN";
const char* ssid = "YOUR-WIFI-SSID";
const char* passphrase = "YOUR-WIFI-PASSWORD";

// Declaring and initializing other variables
Project myProject;
Device myDevice;

// Function prototypes
void setupWiFi(void);

void setup() {
  Serial.begin(9600);
  // This sets up the device WiFi.
  setupWiFi();
  // This initializes the SDK's configurations and returns a new object of Project class.
  myProject = grandeur.init(apiKey, token);
  // Getting object of Device class
  myDevice = myProject.device(deviceID);
  Serial.printf("\nDevice %s is saying hello to Grandeur using API Key %s and Access Token %s.\n", deviceID.c_str(), apiKey.c_str(), token.c_str());
}

void loop() {
  if(myProject.isConnected()) {
    // When the device's connection with Grandeur is established, this if-block runs.
    Serial.println("\nDevice has made a successful connection with Grandeur!");
    Serial.println("Grandeur says hi. Grandeur will now respond to your commands...");
    Serial.println("Try saving your device state to Grandeur and then retrieving it back.");
    Serial.println("Visit https://github.com/grandeurtech/arduino-sdk to learn how!");
    Serial.println("Also checkout other examples: \n- DashListening-Device \n- DashListening-App \n- CrossListening.\n");
  }
  // This runs the SDK only when the WiFi is connected.
  myProject.loop(WiFi.status() == WL_CONNECTED);
}

void setupWiFi(void) {
  // Disconnecting WiFi if it"s already connected
  WiFi.disconnect();
  // Setting it to Station mode which basically scans for nearby WiFi routers
  WiFi.mode(WIFI_STA);
  // Begin connecting to WiFi
  WiFi.begin(ssid, passphrase);
  Serial.printf("\nDevice is connecting to WiFi using SSID %s and Passphrase %s.\n", ssid, passphrase);
  // Device program gets in the while loop as long as the WiFi isn't connected.
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // This gets printed after the WiFi is connected.
  Serial.printf("\nDevice has successfully connected to WiFi. Its IP Address is: %s\n", WiFi.localIP().toString().c_str());
}