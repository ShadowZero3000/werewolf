#include <Arduino.h>
#include <Wire.h>

// Enable the use of the expander board BEFORE the hpp
#define USE_PCA9685_SERVO_EXPANDER
#include <ServoEasing.hpp>

uint8_t blinkState = 0;
uint16_t blinkSpeed = 700;
uint16_t squintSpeed = 300;
uint8_t eyelidOpenPercent = EYELID_NORMAL;
uint8_t eyelidIndexes[4];

ServoEasing LeftUpperEyelid(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing RightUpperEyelid(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing LeftLowerEyelid(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing RightLowerEyelid(PCA9685_DEFAULT_ADDRESS, &Wire);

uint8_t LEUpperClosedPosition = 120;
uint8_t LEUpperOpenPosition = 45;
uint8_t REUpperClosedPosition = 45;
uint8_t REUpperOpenPosition = 120;

uint8_t LELowerClosedPosition = 15;
uint8_t LELowerOpenPosition = 90;
uint8_t RELowerClosedPosition = 90;
uint8_t RELowerOpenPosition = 15;

// Array, index 0 is the current targeted position (where we were most recently in motion to)
// index 1 is the "normal" position to return to after motion completes (If equal, don't move)
uint8_t LEPosition[2] = {EYELID_NORMAL, EYELID_NORMAL};
uint8_t REPosition[2] = {EYELID_NORMAL, EYELID_NORMAL};

int eyelidLoopCounter = 0;
int lastBlinkDelay = -1;
int minBlinkDelay=800;
int maxBlinkDelay=1200;

// These two functions control each eye independently, and set how open the eyelids are based on a percent (0-100) at a set speed
// Separate functions because you can wink, but you'd never want to operate a single eyelid at a time.
void LeftEyeSetOpen(uint8_t percent, uint16_t speed) {
  uint8_t upper = map(percent, 0, 100, LEUpperClosedPosition + LEU_TRIM, LEUpperOpenPosition + LEU_TRIM);
  uint8_t lower = map(percent, 0, 100, LELowerClosedPosition + LEL_TRIM, LELowerOpenPosition + LEL_TRIM);
  LEPosition[0] = percent;
  servoInMotion[LeftUpperEyelid.mServoIndex] = true;
  servoInMotion[LeftLowerEyelid.mServoIndex] = true;
  LeftUpperEyelid.setEaseTo(upper, speed); 
  LeftLowerEyelid.setEaseTo(lower, speed);
}

void RightEyeSetOpen(uint8_t percent, uint16_t speed) {
  uint8_t upper = map(percent, 0, 100, REUpperClosedPosition + REU_TRIM, REUpperOpenPosition + REU_TRIM);
  uint8_t lower = map(percent, 0, 100, RELowerClosedPosition + REL_TRIM, RELowerOpenPosition + REL_TRIM);
  REPosition[0] = percent;
  servoInMotion[RightUpperEyelid.mServoIndex] = true;
  servoInMotion[RightLowerEyelid.mServoIndex] = true;
  RightUpperEyelid.setEaseTo(upper, speed); 
  RightLowerEyelid.setEaseTo(lower, speed);
}

// Evaluates whether you've got an eyelid in motion still (to prevent scheduling more eyelid movements)
bool eyelidMovementPending() {
  bool pending = false;
  for(uint8_t idx=0; idx < 4; idx++){
    pending = pending || servoInMotion[eyelidIndexes[idx]];
  }
  return pending;
}

void eyelidMotionProceedToNext() {
  if (eyelidMovementPending()) {
    Serial.println("Attempted to proceed with eyelid motion while servos are still going. Ignoring.");
    return;
  }
  bool motionRequested = false;
  if(LEPosition[0] != LEPosition[1]) {
    // We just finished left eye motion, and need to return to normal
    LeftEyeSetOpen(LEPosition[1], blinkSpeed);
    motionRequested = true;
  }
  if(REPosition[0] != REPosition[1]) {
    // We just finished right eye motion, and need to return to normal
    RightEyeSetOpen(REPosition[1], blinkSpeed);
    motionRequested = true;
  }
  if (motionRequested) {
    enableServoEasingInterrupt();
  }
}

void EyelidMotionHandler(ServoEasing *aServoEasingInstance) {
  Serial.print("Servo finished movement: ");
  Serial.print(aServoEasingInstance->mServoIndex);
  Serial.print(" is now at: ");
  Serial.println(aServoEasingInstance->getCurrentAngle());
  
  servoInMotion[aServoEasingInstance->mServoIndex] = false;
  if (!eyelidMovementPending()) {
    Serial.println("All eyelid motion done");
    eyelidMotionProceedToNext();
  }
}

void initializeEyelids() {
  Serial.println("Starting eyelid initialization");
  LeftUpperEyelid.attach(3, LEUpperClosedPosition + LEU_TRIM); 
  LeftUpperEyelid.setTargetPositionReachedHandler(EyelidMotionHandler);
  LeftLowerEyelid.attach(2, LELowerClosedPosition + LEL_TRIM);
  LeftLowerEyelid.setTargetPositionReachedHandler(EyelidMotionHandler);

  RightLowerEyelid.attach(6, RELowerClosedPosition + REL_TRIM);
  RightLowerEyelid.setTargetPositionReachedHandler(EyelidMotionHandler);
  RightUpperEyelid.attach(7, REUpperClosedPosition + REU_TRIM); 
  RightUpperEyelid.setTargetPositionReachedHandler(EyelidMotionHandler);

  // LeftUpperEyelid.setEasingType(easeTypeBounce);
  // RightUpperEyelid.setEasingType(easeTypeBounce);

  eyelidIndexes[0] = LeftUpperEyelid.mServoIndex;
  eyelidIndexes[1] = LeftLowerEyelid.mServoIndex;
  eyelidIndexes[2] = RightUpperEyelid.mServoIndex;
  eyelidIndexes[3] = RightLowerEyelid.mServoIndex;
}

void blink() {
  if (LEPosition[0] != LEPosition[1] || REPosition[0] != REPosition[1]){
    Serial.println("Attempt to blink while eyelids still in motion");
    return;
  }
  RightEyeSetOpen(EYELID_CLOSED, blinkSpeed);
  LeftEyeSetOpen(EYELID_CLOSED, blinkSpeed);
  enableServoEasingInterrupt();
}

void wink(char eye) {
  if (eye == "l") {
    LeftEyeSetOpen(EYELID_CLOSED, blinkSpeed);
    enableServoEasingInterrupt();
  }
  if (eye == "r") {
    RightEyeSetOpen(EYELID_CLOSED, blinkSpeed);
    enableServoEasingInterrupt();
  }
}

void setEyelidPosition(int endstate) {
  LEPosition[1] = endstate;
  REPosition[1] = endstate;
}

void squint() {
  LEPosition[1] = EYELID_SQUINT;
  REPosition[1] = EYELID_SQUINT;
  
  LeftEyeSetOpen(EYELID_SQUINT, squintSpeed);
  RightEyeSetOpen(EYELID_SQUINT, squintSpeed);
  enableServoEasingInterrupt();
}

void unsquint() {
  LEPosition[1] = EYELID_NORMAL;
  REPosition[1] = EYELID_NORMAL;
  
  LeftEyeSetOpen(EYELID_NORMAL, squintSpeed);
  RightEyeSetOpen(EYELID_NORMAL, squintSpeed);
  enableServoEasingInterrupt();
}

void toggleSquint() {
  if (LEPosition[1] == EYELID_SQUINT) {
    unsquint();
  } else {
    squint();
  }
}

void blinkPause(){
  lastBlinkDelay = random(minBlinkDelay, maxBlinkDelay);
}

void eyelidLoop() {
  if (lastBlinkDelay == -1) {
    //Initial loop, set a random wait
    blinkPause();
  }
  // Normal human blinks are roughly 14 times per minute, or about once every four seconds.
  // Since this isn't actually a human, we're going to aim for once every ten seconds
  // This loop should occur roughly every 10ms, so we should blink every 1000 loops roughly. 
  // We'll give it a random range of 800-1400 loops pause between blinks
  // That said, we _sometimes_ double blink. In that situation we'll pause a bit extra.
  // So the logic is: blink, set the timer to 4000 (above what we might normally consider)
  // If we choose to double blink, do that once it's done (timer still at 4000) and then set the timer
  // to a random number between 1600-2200
  // If we do not choose to double blink, set the timer to a number between 800-1400
  // Then count down

  // Do nothing if eyelids are still in motion
  if (eyelidMovementPending()) {
    return;
  }

  // This is our main loop, where we randomly decide to blink
  eyelidLoopCounter++;
  if (eyelidLoopCounter >= 1000) {
    eyelidLoopCounter = 0;
  }

  if (lastBlinkDelay == 5*maxBlinkDelay) {
    // When we reach 1000, there's a _chance_ for us to blink a second time
    int chanceToDoubleBlink = random(0,100);
    blinkPause();
    if (chanceToDoubleBlink > 90) {
      // We choose to blink again
      blink();
      lastBlinkDelay = 1.5 * lastBlinkDelay;
    }
    Serial.print("Holding off on blinking again for ");
    Serial.print(lastBlinkDelay);
    Serial.println("0 ms");
  }

  if (lastBlinkDelay == 1) {
    Serial.println("Now allowed to blink again");
  }

  if (lastBlinkDelay > 0) {
    lastBlinkDelay--;
  }

  if (lastBlinkDelay == 0) {
    // We've waited long enough, we _can_ blink again
    if (random(0,100) > 50) {
      // Decide if we want to blink or wink
      if (random(0, 100) > 90) {
        if (random(0,2) == 0) {
          wink("l");
        } else {
          wink("r");
        }
        blinkPause();
      } else {
        blink();
        lastBlinkDelay = 5*maxBlinkDelay; // Initial blink sets to the ceiling, so we can decide whether we're double blinking or not later
      }
    } else {
      // Still didn't blink, consider squinting
      if (random(0,100) < 30) {
        Serial.println("Time to squint");
        toggleSquint();
        // Need a squint pause
        blinkPause();
      }
    }
  }
}

//   switch(blinkState) {
//     case 0:
//       Serial.println("No blink pending");
//       break;
//     case 1: // Not blinking, blink desired
//       Serial.println("Starting a fresh blink");
//       blinkState = 2; // Closing eye
//       RightEyeSetOpen(0, blinkSpeed);
//       LeftEyeSetOpen(0, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 2: // Blinking, eye closed
//       Serial.println("Blink is now closing eyes");
//       blinkState = 3; // Re-opening eye
//       RightEyeSetOpen(eyelidOpenPercent, blinkSpeed);
//       LeftEyeSetOpen(eyelidOpenPercent, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 3: // Finished blinking
//       Serial.println("Blink concluded");
//       blinkState = 0; // Not blinking
//       break;

//     case 10: // Left eye wink
//       Serial.println("Left eye wink requested");
      
//       blinkState = 11; // Left winking
//       LeftEyeSetOpen(0, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 11: // Left eye wink
//       Serial.println("Left eye wink reopening");
      
//       blinkState = 15; // Finished winking
//       LeftEyeSetOpen(eyelidOpenPercent, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 12: // Right eye wink
//       Serial.println("Right eye wink requested");
      
//       blinkState = 11; // Right winking
//       RightEyeSetOpen(0, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 13: // Right eye wink
//       Serial.println("Right eye wink reopening");
      
//       blinkState = 15; // Finished winking
//       RightEyeSetOpen(eyelidOpenPercent, blinkSpeed);
//       enableServoEasingInterrupt();
//       break;
//     case 15: // Finished blinking
//       Serial.println("Wink concluded");
//       blinkState = 0; // Not blinking
//       break;
//   }
// }
