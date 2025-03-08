#include <RoboClaw.h>

// H-bridge motor controller baud rate - 1152000 baud

HardwareSerial mySerial(2);
RoboClaw roboclaw(&mySerial, 10000);

//setting the pins for the H-bridge Motor 
#define IN1 13 
#define IN2 12
#define ENABLEA_PIN 14
#define testpin 27
int testVal = 0;

void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT);
    pinMode(testpin, INPUT);
    
    mySerial.begin(38400);
    Serial.begin(115200);
}

void controlShooter(int in1, int in2, bool enable) {
    digitalWrite(ENABLEA_PIN, enable);
    analogWrite(IN1, in1);
    analogWrite(IN2, in2);
}

void loop() {
    //SHOOTER MECHANISM
      testVal = analogRead(testpin);
      Serial.println(testVal);
      delay(500);

      //int sensorValue = analogRead(testpin);
      //Serial.println(sensorValue);
//    // Intake - similar to how pressing "A" was with the bluetooth controller
      if (testVal = 100){
        controlShooter(0, 140, true); //prev - controlShooter(#, 0, true);
      }
//
//    // Outake - similar to how pressing "X" was with the bluetooth controller
//        else if(testVal = 100 ){
//        controlShooter(0, 20, true);
//      }
//    
//    // Shoot - similar to how pressing "LB" was with the bluetooth controller 
//      else if(testVal < ){
//       controlShooter(220, 0, true);
//      }
    
    // Set pins & ENA = 0 - no intake, outake, or shooting occurs 
//      else if (testVal = 50) {
//        controlShooter(0, 0, false);
//      }
    delay(20);
}
