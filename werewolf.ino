#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// Enable the use of the expander board BEFORE the hpp
#define USE_PCA9685_SERVO_EXPANDER
#include <ServoEasing.hpp>
#include "constants.h"

bool servoInMotion[16];
Adafruit_NeoPixel pixels(1, 3, NEO_GRB + NEO_KHZ800);

void setRandomSeeding() {
    uint16_t seed = 0;
    seed = random(0,1000) + (getASeed()*100);
    randomSeed(seed);
    delay(1);

    // Attempting to get some randomness by doing this a few different times/ways
    seed = ((random(0,1000) + getASeed()) * 10) / 3;  
    randomSeed(seed);
    delay(1);

    seed += (random(0,1000) + getASeed());
    randomSeed(seed);
    Serial.print("Random seed: ");
    Serial.println(seed);
}

void setup() {
    Serial.begin(9600);
    while (!Serial); // Wait for Serial to become available. Is optimized away for some cores.
    setRandomSeeding();
    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(0, 150, 0));
    pixels.show();
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_SERVO_EASING));

    initializeEyes();
    initializeEyelids();
    // Stare straight ahead, then slowly open eyes
    lookAt(90,90);
    delay(500);
    RightEyeSetOpen(EYELID_NORMAL, 40);
    LeftEyeSetOpen(EYELID_NORMAL, 40);
    enableServoEasingInterrupt();
    while(!updateAllServos());
    squint();
}

int getASeed() {
  return (
    analogRead(0)+
    analogRead(1)+
    analogRead(2)+
    analogRead(3)+
    analogRead(4)+
    analogRead(5)+
    analogRead(6)+
    analogRead(7)+
    analogRead(8)+
    analogRead(9)
  );
}

void loop() {
  // toggleSquint();
  //   while(!updateAllServos());
  //   delay(1000);
  // setEyelidPosition(0);
  // RightEyeSetOpen(0, 40);
  // LeftEyeSetOpen(0, 40);
  // while(!updateAllServos());
  // delay(2000);
  // setEyelidPosition(50);
  // RightEyeSetOpen(50, 40);
  // LeftEyeSetOpen(50, 40);
  // while(!updateAllServos());
  // delay(10000);
  // setEyelidPosition(EYELID_NORMAL);
  // RightEyeSetOpen(0, 40);
  // LeftEyeSetOpen(0, 40);
  // while(!updateAllServos());
  // delay(10000);
  eyelidLoop();
  eyeMoveLoop();
  delay(10);
}
// #include <Wire.h>
// #include <Adafruit_PWMServoDriver.h>
// #include <ServoEasing.h>

// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// #define SERVOMIN  140 // this is the 'minimum' pulse length count (out of 4096)
// #define SERVOMAX  520 // this is the 'maximum' pulse length count (out of 4096)

// // our servo # counter
// //uint8_t servonum = 0;

// int xval;
// int yval;

// // TODO: Better names?
// int lexpulse;
// int rexpulse;

// int leypulse;
// int reypulse;

// int uplidpulse;
// int lolidpulse;
// int altuplidpulse;
// int altlolidpulse;

// // TODO: What?
// int trimval;

// //const int analogInPin = A0;
// int sensorValue = 0;
// int outputValue = 0;
// int switchval = 0;

// // Eye servo ids
// uint8_t LEye = 0;
// uint8_t REye = 1;
// // Eyelid servo ids
// uint8_t BLEyelid = 2;
// int BLEyelidResting = 400;
// uint8_t TLEyelid = 3;
// int TLEyelidResting = 240;

// uint8_t BREyelid = 6;
// int BREyelidResting = 240;
// uint8_t TREyelid = 7;
// int TREyelidResting = 400;

// void setup() {
//   Serial.begin(9600);
//   Serial.println("8 channel Servo test!");
// //  pinMode(analogInPin, INPUT);
//   pinMode(2, INPUT_PULLUP);

//   pwm.begin();
//   pwm.setPWMFreq(60); // Analog servos run at ~60Hz updates

//   delay(10);
// }

// void getDesiredPosition() {
//   xval = analogRead(A1);
//     lexpulse = map(xval, 0,1023, 220, 440);
//     rexpulse = lexpulse;  
    
//   Serial.print("X value:");
//   Serial.println(xval);
//   yval = analogRead(A0);
//   Serial.print("Y value:");
//   Serial.println(yval);
//     leypulse = map(yval, 0,1023, 250, 500);
//     reypulse = map(yval, 0,1023, 400, 280);
// }

// void getEyelidInput() {
//   switchval = digitalRead(2);
// }

// void getEyelidLimit() {
//   trimval = analogRead(A2);
//     Serial.print("Read Trim: ");
//     Serial.println(trimval);
//   //trimval = 260;
//     Serial.print("Initial Trim: ");
//     Serial.println(trimval);
//     trimval=map(trimval, 320, 580, -40, 40);
//     Serial.print("Trim: ");
//     Serial.println(trimval);
//      uplidpulse = map(yval, 0, 1023, 400, 280);
//         uplidpulse -= (trimval-40);
//           uplidpulse = constrain(uplidpulse, 280, 400);
//      altuplidpulse = 680-uplidpulse;

//      lolidpulse = map(yval, 0, 1023, 410, 280);
//        lolidpulse += (trimval/2);
//          lolidpulse = constrain(lolidpulse, 280, 400);      
//      altlolidpulse = 680-lolidpulse;
// }

// void moveEyes() {
//   pwm.setPWM(LEye, 0, lexpulse);
//   pwm.setPWM(REye, 0, leypulse);
// }

// void closeEyes() {
//       Serial.print("Bottom left. From: ");
//       Serial.print(BLEyelidResting);
//       Serial.print(" to: ");
//       Serial.println(lolidpulse);
//       Serial.print("Bottom right. From: ");
//       Serial.print(BREyelidResting);
//       Serial.print(" to: ");
//       Serial.println(altlolidpulse);
//       Serial.print("Top left. From: ");
//       Serial.print(TLEyelidResting);
//       Serial.print(" to: ");
//       Serial.println(uplidpulse);
//       Serial.print("Top right. From: ");
//       Serial.print(TREyelidResting);
//       Serial.print(" to: ");
//       Serial.println(altuplidpulse);

//       pwm.setPWM(TLEyelid, 0, uplidpulse);
//       pwm.setPWM(BLEyelid, 0, lolidpulse);
//       pwm.setPWM(BREyelid, 0, altlolidpulse);
//       pwm.setPWM(TREyelid, 0, altuplidpulse);

// }
// void openEyes() {

//       pwm.setPWM(TLEyelid, 0, TLEyelidResting);
//       pwm.setPWM(BLEyelid, 0, BLEyelidResting);
//       pwm.setPWM(BREyelid, 0, BREyelidResting);
//       pwm.setPWM(TREyelid, 0, TREyelidResting);
// }

// void squint() {
//   openPercent=25;
//   //Instead of figuring out stops/limits, just have a "eyelids to X percent"
// }

// void blink() {
//   getEyelidLimit();
//   getEyelidInput();
//   Serial.println(switchval);
//   if (switchval == LOW) {
//       closeEyes();
//       Serial.println("Value is low, eyes are will close");
//       return;
//   }
//   if (random(0,10) > 5) {
//       closeEyes();
//   }
//   else {
//       Serial.println("Value is high, eyes are open");
//       openEyes();
//   }
// }
// // NOT USED
// // you can use this function if you'd like to set the pulse length in seconds
// // e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
// void setServoPulse(uint8_t n, double pulse) {
//   double pulselength;
  
//   pulselength = 1000000;   // 1,000,000 us per second
//   pulselength /= 60;   // 60 Hz
//   Serial.print(pulselength); Serial.println(" us per period"); 
//   pulselength /= 4096;  // 12 bits of resolution
//   Serial.print(pulselength); Serial.println(" us per bit"); 
//   pulse *= 1000000;  // convert to us
//   pulse /= pulselength;
//   Serial.println(pulse);

// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   //blink();
//   getEyelidLimit();
//   getEyelidInput();
//   closeEyes();
//   getDesiredPosition();
//   xval=random(250, 750);
//   yval=random(250, 750);
//   delay(random(1000,5000));
//   getEyelidLimit();
//   openEyes();
//   delay(random(1000,5000));
//     lexpulse = map(xval, 0,1023, 220, 440);
//     rexpulse = lexpulse;  
//     leypulse = map(yval, 0,1023, 250, 500);
//     reypulse = map(yval, 0,1023, 400, 280);
//   //moveEyes();
//   delay(5);
// }
