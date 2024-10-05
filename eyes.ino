#include <Arduino.h>
#include <Wire.h>

// Enable the use of the expander board BEFORE the hpp
#define USE_PCA9685_SERVO_EXPANDER
#include <ServoEasing.hpp>

uint8_t eyeIndexes[2];

uint8_t LRMin = 60;
uint8_t LRMax = 120;
uint8_t UDMin = 60;
uint8_t UDMax = 120;

ServoEasing EyeUpDown(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing EyeLeftRight(PCA9685_DEFAULT_ADDRESS, &Wire);

bool eyeMovementPending() {
  bool pending = false;
  for(uint8_t idx=0; idx < 2; idx++){
    pending = pending || servoInMotion[eyeIndexes[idx]];
  }
  return pending;
}

void EyeMotionHandler(ServoEasing *aServoEasingInstance) {
  if (!eyeMovementPending()) {
    Serial.println("All eye servos finished moving");
  }
  //   nextEyeMovement();
  // }
}

void lookAt(int x, int y){
  Serial.print("Looking at: ");
  Serial.print(x);
  Serial.print(" x, ");
  Serial.print(y);
  Serial.println("y.");
  Serial.print("Squint check: ");
  Serial.println(LEPosition[1]);
  if(LEPosition[1] == EYELID_SQUINT) { // Squinting, limit up/down
    int range = UDMax - UDMin;
    y = map(y, UDMin, UDMax, UDMin + (range / 4), UDMax - (range / 4));
  }
  // Likely will need a map here for coords to eye positions
  EyeLeftRight.setEaseTo(x + LR_TRIM, 100); // Blocking call, runs on all platforms
  EyeUpDown.setEaseTo(y + UD_TRIM, 100); // Blocking call, runs on all platforms
  enableServoEasingInterrupt();
}

void nextEyeMovement() {
  int x = random(LRMin, LRMax);
  int y = random(UDMin,UDMax);
  lookAt(x, y);
}

void initializeEyes() {
  Serial.println("Starting eye initialization");
    EyeUpDown.attach(0, UD_TRIM + UDMin + ((UDMax - UDMin)/2)); // Attach pin and go to initial position in middle of range
    EyeLeftRight.attach(1, LR_TRIM + LRMin + ((LRMax - LRMin)/2));
    EyeUpDown.setEasingType(easeTypeBounce);
    EyeLeftRight.setEasingType(easeTypeBounce);
    
    EyeUpDown.setTargetPositionReachedHandler(EyeMotionHandler);
    EyeLeftRight.setTargetPositionReachedHandler(EyeMotionHandler);

    eyeIndexes[0] = EyeUpDown.mServoIndex;
    eyeIndexes[1] = EyeLeftRight.mServoIndex;
}

int eyeLoopCounter = 0;

void eyeMoveLoop() {
  // Do nothing if eyes are still in motion
  if (eyeMovementPending()) {
    return;
  }

  // This is our main loop, where we randomly decide to blink
  eyeLoopCounter++;
  if (eyeLoopCounter >= 1000) {
    eyeLoopCounter = 0;
  }

  if(eyeLoopCounter % 10 == 0 && random(0,100) < 10) {
    nextEyeMovement();
  }
}