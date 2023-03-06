int soundSensor1 = 34;
int soundSensor2 = 33;
int LED1 = 5;
int LED2 = 19; // The analog input pin on the ESP32 board
unsigned int statusSensor1;
unsigned int statusSensor2; // The analog input value from the sound sensor
float voltage1 = 0;
float voltage2 = 0; // The voltage value calculated from the analog input value
float dB1 = 0;
float dB2 = 0;
unsigned long previousMillis = 0;  // variable to store the previous time
const unsigned long interval = 60000;  // interval in milliseconds (10 minutes)

void setup() {

  Serial.begin(9600);
  
  pinMode(soundSensor1, INPUT);
  pinMode(LED1, OUTPUT);

  pinMode(soundSensor2, INPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  statusSensor1 = analogRead(soundSensor1);
  statusSensor2 = analogRead(soundSensor2);
  unsigned long currentMillis = millis();
  convertToDecibel();
  

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    checkTheMics();
  }

  if (dB1 > 50.0)
  { 
    digitalWrite(LED1, HIGH);  
  }
  if (dB2 > 50.0)
  {   
    digitalWrite(LED2, HIGH);  
  } 

}

void convertToDecibel(){
  voltage1 = statusSensor1 * (3.3 / 4095.0); // Convert the analog input value to voltage
  dB1 = 20 * log10(voltage1 / 0.001);
  Serial.print("Decibel Value: ");
  Serial.println(dB1);

  voltage2 = statusSensor2 * (3.3 / 4095.0); // Convert the analog input value to voltage
  dB2 = 20 * log10(voltage2 / 0.001); // Convert the voltage value to decibel value
  Serial.print("Decibel Value: ");
  Serial.println(dB2);
  
}

void checkTheMics(){
  if (dB1 < 50.0)
  {
    digitalWrite(LED1, LOW);     

  }

  if (dB2 < 50.0)
  {
    digitalWrite(LED2, LOW);     

  }
  
}

