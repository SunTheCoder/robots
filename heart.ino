
#include <Stepper.h>
#include "LedControl.h"

/*----- Stepper Setup -----*/
#define STEPS_PER_REV 32
#define DISPENSE_STEPS 64

Stepper seed_dispenser(STEPS_PER_REV, 8, 10, 9, 11);

/*----- LED Matrix Setup (MAX7219) -----*/
// Pins: DIN = 5, CLK = 6, CS = 7
LedControl lc = LedControl(5, 6, 7, 1);

/*----- Distance Simulation -----*/
int distanceTravelled = 0;
const int plantingInterval = 20;
const int maxDistance = 200;

/*----- Timing -----*/
unsigned long lastMoveTime = 0;
const int moveDelay = 200;

/*----- Heart Pattern (8x8) -----*/
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

void setup() {
  Serial.begin(9600);

  // Stepper
  seed_dispenser.setSpeed(100);

  // LED Matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8); // 0–15 brightness
  lc.clearDisplay(0);

  Serial.println("Starting simulated planting run...");
}

/*----- Display heart animation -----*/
void showHeart() {
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, heart[row]);
  }
  delay(500); // show heart for 0.5 seconds
  lc.clearDisplay(0);
}

void loop() {
  unsigned long now = millis();

  if (now - lastMoveTime >= moveDelay) {
    lastMoveTime = now;
    distanceTravelled++;

    Serial.print("Distance travelled: ");
    Serial.print(distanceTravelled);
    Serial.println(" cm");

    if (distanceTravelled % plantingInterval == 0) {
      Serial.println("→ Planting seed...");

      // Rotate the stepper
      seed_dispenser.step(DISPENSE_STEPS);
      delay(100);

      // Show heart on LED matrix
      showHeart();

      Serial.println("✓ Seed planted!\n");
    }

    if (distanceTravelled >= maxDistance) {
      Serial.println("== Reached end of row. Resetting simulation. ==");
      distanceTravelled = 0;
      delay(2000);
    }
  }
}
