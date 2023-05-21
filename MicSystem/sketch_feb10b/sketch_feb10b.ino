#include <driver/i2s.h>
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
#define API_KEY "AIzaSyBnI_v1i40XVrWCxPYcCGvFmsB0ayq4qaM"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://finalproject-8a759-default-rtdb.asia-southeast1.firebasedatabase.app" 

// Connections to INMP441 I2S microphone
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32

// Connections to MAX98357 amplifier
#define MAX98357_DOUT 26
#define MAX98357_BCLK 27
#define MAX98357_LRC 14

// Use I2S Processor 0 for microphone and Processor 1 for amplifier
#define I2S_PORT_MIC I2S_NUM_0
#define I2S_PORT_AMP I2S_NUM_1

// Define input buffer length
#define bufferLen 64
int16_t sBuffer[bufferLen];

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
int volume=100;

void i2s_install() {
  // Set up I2S Processor configuration for microphone
  const i2s_config_t i2s_mic_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  // Set up I2S Processor configuration for amplifier
  const i2s_config_t i2s_amp_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT_MIC, &i2s_mic_config, 0, NULL);
  i2s_driver_install(I2S_PORT_AMP, &i2s_amp_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration for microphone
  const i2s_pin_config_t mic_pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  // Set I2S pin configuration for amplifier
  const i2s_pin_config_t amp_pin_config = {
    .bck_io_num = MAX98357_BCLK,
    .ws_io_num = MAX98357_LRC,
    .data_out_num = MAX98357_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_set_pin(I2S_PORT_MIC, &mic_pin_config);
  i2s_set_pin(I2S_PORT_AMP, &amp_pin_config);
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  Serial.println(" ");

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

  delay(1000);

  // Set up I2S
  i2s_install();
  i2s_setpin();

  delay(500);
}

void loop() {

  // Get I2S data from the microphone
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT_MIC, &sBuffer, bufferLen * 2, &bytesIn, portMAX_DELAY);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 ||  sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/LED/digit")) {
    if (fbdo.dataType() == "int") {
        volume = fbdo.intData(); // Retrieve the integer value
      }
    }else{
      Serial.println ("FAILED: " + fbdo.errorReason() ) ;
    }
  }

  if (result == ESP_OK) {
    int samples_read = bytesIn / 2;
    if (samples_read > 0) {
      // Adjust volume by scaling the audio samples
      //int volume = 70; // Adjust the volume (0-100)
      for (int i = 0; i < samples_read; i++) {
        sBuffer[i] = (sBuffer[i] * volume) / 100;
      }

      // Write audio data to MAX98357 amplifier
      i2s_write(I2S_PORT_AMP, &sBuffer, samples_read * 2, &bytesIn, portMAX_DELAY);
    }
  }
}
