#include <Wire.h>
#include <MPU6050.h>
#include "DHT.h"
#include <WiFi.h>
//#include <BlynkSimpleEsp32.h>
#include <BlynkTimer.h>

#define BLYNK_TEMPLATE_ID "TMPL3RewDogoW"
#define BLYNK_TEMPLATE_NAME "package security system"
#define BLYNK_AUTH_TOKEN "IVTPZTectsdk7QDkGLAKCu--3aIxv62f"
// ---------- Pin Definitions ----------
#define PIR_PIN 15
#define BUZZER_PIN 17
#define LED_PIN 2
#define DHTPIN 14
#define DHTTYPE DHT22
#define FALL_PIN V5  // Virtual pin for fall detection

MPU6050 mpu;
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

// ---------- Function to Read Sensors and Send Data ----------
void sendDataToBlynk() {
  // --- MPU6050 ---
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float mpuTemp = (mpu.getTemperature() / 340.0) + 36.53;

  // --- DHT22 ---
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();

  // --- PIR Motion ---
  int motion = digitalRead(PIR_PIN);

  // --- Send data to Blynk ---
  Blynk.virtualWrite(V1, temp);       // DHT Temp
  Blynk.virtualWrite(V2, humi);       // DHT Humidity
  Blynk.virtualWrite(V3, mpuTemp);    // MPU Temp
  Blynk.virtualWrite(V4, motion);     // PIR Motion LED

  // --- Fall Detection ---
  if (abs(ax_g) > 1.5 || abs(ay_g) > 1.5 || abs(az_g) > 1.5) {
    Blynk.virtualWrite(FALL_PIN, 1);  // Trigger Eventor notification
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("⚠️ Package Fallen Down!");
  } else {
    Blynk.virtualWrite(FALL_PIN, 0);  // Reset fall pin
  }

  // --- Serial Output ---
  Serial.print("DHT Temp: "); Serial.print(temp); Serial.print("°C  |  Humidity: "); Serial.print(humi); Serial.print("%  |  MPU Temp: "); Serial.print(mpuTemp); Serial.println("°C");
  Serial.print("Accel X: "); Serial.print(ax_g); Serial.print("g  Y: "); Serial.print(ay_g); Serial.print("g  Z: "); Serial.println(az_g);  
  Serial.println(motion ? "Motion Detected!" : "No Motion");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA, SCL

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  Serial.println("Package Security Monitoring System");
  Serial.println("Initializing MPU6050...");
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }
  Serial.println("MPU6050 ready.");
  delay(2000);

  // Blynk Connection
  Blynk.begin(auth, ssid, pass);

  // Timer to send data every 2 seconds
  timer.setInterval(2000L, sendDataToBlynk);
}

// ---------- Loop ----------
void loop() {
  Blynk.run();
  timer.run();

  // PIR motion LED & buzzer
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}
