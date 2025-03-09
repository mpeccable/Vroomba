#include <RoboClaw.h>

//setting the pins for the H-bridge Motor 
#define IN1 23 
#define IN2 22
#define ENABLEA_PIN 14
#define CH1 27
#define CH6 26
#define CH5 33

// Channel values correspond to sticks and switches on flysky controller
int ch1Value = 0;                 //right stick up/down
int ch5Value = 0;                 //SWd up/down
int ch6Value = 0;                 //SWa up/down

void setup() {
  pinMode(IN1, OUTPUT);           //motor controller direction 1 power
  pinMode(IN2, OUTPUT);           //motor controller direction 2 power
  pinMode(ENABLEA_PIN, OUTPUT);   //motor controller on switch
  pinMode(CH1, INPUT);            //transmitted right stick y axis
  pinMode(CH6, INPUT);            //transmitter switch a
  pinMode(CH5, INPUT);            //transmitter switch d
}

void loop() {
  //SHOOTER MECHANISM
  //Gets a value between 913 (min right stick y) and 2099 (max right stick y)
  ch1Value = pulseIn(CH1, HIGH);

  //Kill switches, Ch6 for whole drive train & shooter Ch5 just for shooter
  //Values either 999/1000 SWd up versus 1999/2000 SWd down
  ch6Value = pulseIn(CH6, HIGH);
  //Values either 999/1000 SWa up versus 1999/2000 SWa down
  ch5Value = pulseIn(CH5, HIGH);

  // Set pins & ENA = 0 - no intake, outake, or shooting occurs 
  if (ch5Value > 1500 | ch6Value < 1500) {
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(ENABLEA_PIN, LOW);
  }

  else {
    if (ch1Value < 1300) {
      //spit out puck along the ground
      digitalWrite(ENABLEA_PIN, HIGH);
      analogWrite(IN1, 0);
      analogWrite(IN2, 200);
    }
    else if (ch1Value >= 1300 & ch1Value < 1700) {
      //grab onto puck but dont shoot
      digitalWrite(ENABLEA_PIN, HIGH);
      analogWrite(IN1, 120);
      analogWrite(IN2, 0);
    }
    else if (ch1Value >= 1700) {
      //shoot puck over top
      digitalWrite(ENABLEA_PIN, HIGH);
      analogWrite(IN1, 220);
      analogWrite(IN2, 0);
    }
  }
  delay(5);
}
