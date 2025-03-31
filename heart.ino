#include <Stepper.h>
#include <LedControl.h>
#include <dht_nonblocking.h>
#include <Servo.h>
#include "SR04.h"

// === DHT11 Sensor ===
#define DHT_SENSOR_TYPE DHT_TYPE_11
const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// === Stepper Motor ===
#define STEPS_PER_REV 32
#define DISPENSE_STEPS 64
Stepper seed_dispenser(STEPS_PER_REV, 8, 10, 9, 11); // Uses pin 11

// === MAX7219 LED Matrix ===
LedControl lc = LedControl(A1, A2, A3, 1); // DIN, CLK, CS

// === Servo Motor ===
Servo myservo; // Connected to pin 6

// === Ultrasonic Sensor ===
#define TRIG_PIN 7
#define ECHO_PIN 5
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
long distance;
bool triggered = false;
unsigned long lastTriggerTime = 0;
const unsigned long cooldown = 5000; // 5 sec between triggers

// === Heart Pattern ===
byte heart[8] = {
  B00000000,
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000
};

// === Smiley Pattern ===
byte smiley[8] = {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
};

void setup() {
  Serial.begin(9600);
  delay(2000); // Sensor warm-up

  // Stepper
  seed_dispenser.setSpeed(100);

  // MAX7219
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  // Servo
  myservo.attach(6);
  myservo.write(90); // Center

  Serial.println("🤖 SeedHorn initialized and ready.");
}

void showHeart() {
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, heart[row]);
  }
  delay(500);
  lc.clearDisplay(0);
}

void showSmiley() {
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, smiley[row]);
  }
  delay(500);
  lc.clearDisplay(0);
}

void plantSeed() {
  Serial.println("🌱 Planting...");

  // Stepper
  seed_dispenser.step(DISPENSE_STEPS);

  // Heart
  showHeart();

  // Servo
  Serial.println("🤖 Moving servo...");
  myservo.write(60);
  delay(400);
  myservo.write(120);
  delay(400);
  myservo.write(90);

  // Smiley
  showSmiley();
}

void checkHumidity() {
  float temperature, humidity;
  if (dht_sensor.measure(&temperature, &humidity)) {
    Serial.print("🌡 Temp: ");
    Serial.print(temperature, 1);
    Serial.print(" °C | 💧 Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
  }
}

void loop() {
  // Let DHT poll freely (non-blocking)
  checkHumidity();

  // Check distance from ultrasonic
  distance = sr04.Distance();
  Serial.print("📏 Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  unsigned long now = millis();

  if (distance > 0 && distance < 15 && !triggered && now - lastTriggerTime > cooldown) {
    triggered = true;
    lastTriggerTime = now;

    plantSeed();
    Serial.println("✅ Sequence complete.");
  }

  if (triggered && (now - lastTriggerTime > 1000)) {
    triggered = false; // Reset trigger after short cooldown
  }

  delay(250); // Chill loop speed
}
