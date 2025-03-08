#include <RoboClaw.h>

// H-bridge motor controller baud rate - 1152000 baud

//setting the pins for the H-bridge Motor 
#define IN1 13 
#define IN2 12
#define ENABLEA_PIN 14
#define CH1 27
#define CH6 26
#define CH5 33
int ch1Value = 0;
int ch5Value;
int ch6Value;

void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT);
    pinMode(CH1, INPUT);
    pinMode(CH6, INPUT);
    pinMode(CH5, INPUT);
    
    Serial.begin(115200);
}

void controlShooter(int in1, int in2, bool enable) {
    digitalWrite(ENABLEA_PIN, enable);
    analogWrite(IN1, in1);
    analogWrite(IN2, in2);
}

void loop() {
    //SHOOTER MECHANISM
      ch1Value = pulseIn(CH1, HIGH);
      // Serial.println(ch1Value);

      ch6Value = pulseIn(CH6, HIGH);

      ch5Value = pulseIn(CH5, HIGH);

      Serial.println(ch5Value);

      delay(50);

      //int sensorValue = analogRead(testpin);
      //Serial.println(sensorValue);

    // Set pins & ENA = 0 - no intake, outake, or shooting occurs 
      if ((ch5Value == 999) || (ch5Value == 1000)) {
        controlShooter(0, 0, false);
      }
      
//    // Intake - similar to how pressing "A" was with the bluetooth controller
      else if (ch1Value < 1500){
        controlShooter(100, 0, true); //prev - controlShooter(#, 0, true);
      }
//
//    // Outake - similar to how pressing "X" was with the bluetooth controller
       else if(ch1Value > 1530 ){
       controlShooter(0, 100, true);
     }
//    
//    // Shoot - similar to how pressing "LB" was with the bluetooth controller 
     else if((ch6Value == 999) || (ch6Value == 1000)){
      controlShooter(180, 0, true);
     }
    
     else{
       controlShooter(0, 0, false);
     }
}
