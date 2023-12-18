#include <Wire.h> 
#include "MAX30100_PulseOximeter.h"
PulseOximeter pox;
int BPM, SpO2;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
#include "I2Cdev.h"
#include "MPU6050.h"
//khaibao firebase
#include "WiFi.h"
const char* ssid = "robin ngoai dao hoang";
const char* password = "xinthicho";
#include <Firebase_ESP_Client.h>
#define FIREBASE_HOST "https://firebas-demo-esp32-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyBRXP6uI9HKOjDzfMaVWHFDaCrl5Sz6zmo"

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

MPU6050 accelgyro;
int16_t ax, ay, az;
#define OUTPUT_READABLE_ACCELGYRO
int stepCount = 0;
int previousReading = 0;
int stepThreshold = 2000; // Adjust this value as needed
int n = 0;
int BPM1 = 0, SpO21 = 0;
const unsigned long interval = 10000;  // Gửi dữ liệu mỗi 10 giây
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  //ket noi wifi
  delay(200);
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("Firebase client v%s\n\n", FIREBASE_CLIENT_VERSION);  // Kêt nối Firebase

    config.api_key = FIREBASE_AUTH;
    config.database_url = FIREBASE_HOST;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
Firebase.begin(&config, &auth);


  
  Firebase.reconnectWiFi(true);
  
   lcd.init();
  // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("BPM:");
    lcd.setCursor(8,0);
    lcd.print("SPO2:");
    lcd.setCursor(0,1);
    lcd.print("STEP:");
    Wire.begin();
   if (!pox.begin())
    {
         Serial.println("FAILED");
         for(;;);
    }
    else
    {
         Serial.println("SUCCESS");
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // initialize device
    accelgyro.initialize();
    
}
void loop() {
  unsigned long currentMillis = millis();
    pox.update();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    Serial.println(BPM);
    //Serial.print("SpO2: "); // set text
    Serial.println(SpO2);
    lcd.setCursor(4,0);
    lcd.print(BPM);
    lcd.setCursor(13,0);
    lcd.print(SpO2);
    lcd.setCursor(8,1);
    lcd.print(stepCount);
    if (BPM < 10){
    lcd.setCursor(5,0);
    lcd.print("  ");
    }
    if (BPM < 100){
    lcd.setCursor(6,0);
    lcd.print(" ");
    }
    if (SpO2 < 10){
    lcd.setCursor(14,0);
    lcd.print("  ");
    }
     if (SpO2 < 100){
    lcd.setCursor(15,0);
    lcd.print(" ");
    }
     if (stepCount < 10){
    lcd.setCursor(9,1);
    lcd.print("  ");
    }
    if (stepCount < 100){
    lcd.setCursor(10,1);
    lcd.print(" ");
    }
    accelgyro.getAcceleration(&ax, &ay, &az);
    if (n == 0){
      previousReading = ay;
    }
    n++;
    
    int currentReading = ay;
    if (currentReading > previousReading + stepThreshold || previousReading > currentReading + stepThreshold) {
      // Increment the step count
      stepCount++;
      // Update the previous reading with the current reading
      previousReading = currentReading;
      
    }
  else {
      // Update the previous reading without incrementing the step count
      previousReading = currentReading;
      }

    if(BPM != BPM1 || SpO2 != SpO21){
      BPM1 = BPM;
        Firebase.RTDB.setInt(&firebaseData, "BPM", BPM1);
        Firebase.RTDB.setInt(&firebaseData, "stepCount", stepCount);
        Firebase.RTDB.setInt(&firebaseData, "SpO2", SpO2);
    }

    //delay(50);
}
