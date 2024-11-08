/* Made using code from HowToMechatronics and 
lastminuteengineers.com*/

// define pin connections for motor
#define ENA1 9 //PWM pin for turning motor on/off and speed control
#define IN1 6 //DIGITAL for motor direction
#define IN2 7 //DIGITAL for motor direction

/*
Input Logic for future ref
IN1-0, IN2-0 --> Motor Off
IN1-1, IN2-0 --> Motor Forward
IN1-0, IN2-1 --> Motor Backward
IN1-1, IN2-1 --> Motor Off
*/


void setup() {
  // put your setup code here, to run once:
  // Set all pins to output mode
  pinMode(ENA1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Set motor to OFF initially
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  directionTest();
  delay(1000);
  speedTest();
  delay(1000);
}

void directionTest(){
  // this code will run the motor forward and backward at 
  // max speed

  // Turn on motor
  analogWrite(ENA1, 255);

  // Forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  delay(2000); // for 2 seconds

  // Backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH)
  delay(2000); // for 2 seconds

  // Turn off motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void speedTest(){
  // Spin forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Accelerate from 0 to max speed/2
  for (int i = 0, i < 256/2, i++){
    analogWrite(ENA1, i);
    delay(20);
  }

  // Decelerate from max speed/2 to 0
  for (int i = 256/2, i <= 0, i--){
    analogWrite(ENA1, i);
    delay(20);
  }

  // Turn off motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
