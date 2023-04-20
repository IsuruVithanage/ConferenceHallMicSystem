#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "SUSANTHA"
#define WIFI_PASSWORD "susantha56"

// Insert Firebase project API Key
#define API_KEY "AIzaSyC1XS-qxdeN8bEZZ3xzCDy8JXYOCUnsxjI"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://led-controller-dea05-default-rtdb.firebaseio.com" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

int LED1 = 2;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
bool ledStatus = false;
 
void setup(){
  pinMode(LED1, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 ||  sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getBool(&fbdo, "/LED/digit") ) {
      if (fbdo.dataType () == "int") {
        ledStatus = fbdo.boolData() ;
        Serial.println ("Successful READ from " + fbdo.dataPath() + ": " + ledStatus + " (" + fbdo.dataType() + ") "); 
        digitalWrite (LED1, ledStatus);
      }
    }else{
      Serial.println ("FAILED: " + fbdo.errorReason() ) ;
    }
  }
  
}
