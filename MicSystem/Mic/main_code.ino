#include <driver/i2s.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <WiFiManager.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB1_GNfIJCKvmulXU8aaLmU2XSzA42Crak"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://digimic-default-rtdb.asia-southeast1.firebasedatabase.app" 

// Connections to INMP441 I2S microphone
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32

// Connections to receiving ESP32
#define RECEIVING_ESP32_BCLK_PIN 27
#define RECEIVING_ESP32_LRCK_PIN 14
#define RECEIVING_ESP32_DOUT_PIN 26

// Use I2S Processor 0 for microphone and Processor 1 for receiving ESP32
#define I2S_PORT_MIC I2S_NUM_0
#define I2S_PORT_RECEIVE I2S_NUM_1

// Define input buffer length
#define bufferLen 1024
#define buffercount 10
int16_t sBuffer[bufferLen];

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
int volume=100;
int mic2v;
bool isMute=false;
int button;

void i2s_install() {
  // Set up I2S Processor configuration for microphone
  const i2s_config_t i2s_mic_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 10,
    .dma_buf_len = bufferLen,
    .use_apll = true
  };

  // Set up I2S Processor configuration for receiving ESP32
  const i2s_config_t i2s_receive_config = {
   .mode = i2s_mode_t(I2S_MODE_SLAVE| I2S_MODE_TX),
   .sample_rate = 44100,
   .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
   .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
   .communication_format = I2S_COMM_FORMAT_I2S_MSB,
   .intr_alloc_flags = 0,
   .dma_buf_count = buffercount,
   .dma_buf_len = bufferLen,
   .use_apll = true
  };

  i2s_driver_install(I2S_PORT_MIC, &i2s_mic_config, 0, NULL);
  i2s_driver_install(I2S_PORT_RECEIVE, &i2s_receive_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration for microphone
  const i2s_pin_config_t mic_pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  // Set I2S pin configuration for receiving ESP32
  const i2s_pin_config_t receive_pin_config = {
    .bck_io_num = RECEIVING_ESP32_BCLK_PIN,
    .ws_io_num = RECEIVING_ESP32_LRCK_PIN,
    .data_out_num = RECEIVING_ESP32_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_set_pin(I2S_PORT_MIC, &mic_pin_config);
  i2s_set_pin(I2S_PORT_RECEIVE, &receive_pin_config);
}

void setup() {

  pinMode(4, INPUT_PULLUP);

  WiFiManager wm;

  bool res;
  res = wm.autoConnect("MicSystem");

  if(!res) {
    ESP.restart();
  }else{
    bool currentValue = Firebase.getBool(fbdo, "MIC/WifiStat");
    if (firebaseData.dataType() == "boolean") {
    // Toggle the boolean value
    bool newValue = !currentValue;
    Firebase.setBool(fbdo, PATH_TO_BOOL_VALUE, newValue);
    Serial.print("Boolean value updated to: ");
    Serial.println(newValue);
  }
  }

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    signupOK = true;
  }
  else{
    signupOK = false;
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  delay(1000);

  // Set up I2S
  i2s_install();
  i2s_setpin();

  delay(500);

}

void checkVolume(){

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 ||  sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/MIC/MIC_ONE")) {
      if (fbdo.dataType() == "int") {
        volume = fbdo.intData();
      }
    }
  }

}

void loop() {
  // Get I2S data from receiving ESP32
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT_MIC, &sBuffer, bufferLen * 2, &bytesIn, portMAX_DELAY);

  checkVolume();
  
  button=digitalRead(4);
  
  if (button==LOW){
    if(isMute==true){
      if (Firebase.RTDB.setInt(&fbdo, "/MIC/MIC_ONE", mic2v)){
        volume=mic2v;
        isMute=false;
      }
    }else{
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setInt(&fbdo, "/MIC/MIC_ONE", 0)){
        mic2v=volume; 
        volume=0;
        isMute=true;
      }
    }
  }


  if (result == ESP_OK) {
    int samples_read = bytesIn / 2;
    if (samples_read > 0) {
      // Adjust volume by scaling the audio samples
      //int volume = 70; // Adjust the volume (0-100)
      for (int i = 0; i < bufferLen; i++) {
        sBuffer[i] = (sBuffer[i] * volume) / 100;
      }

      // Write audio data to MAX98357 amplifier
      i2s_write(I2S_PORT_RECEIVE, &sBuffer, samples_read * 2, &bytesIn, portMAX_DELAY);
    }
  }
}


