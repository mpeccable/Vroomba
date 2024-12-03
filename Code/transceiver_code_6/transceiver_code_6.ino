/*
  Arduino FS-I6X Demo
  fsi6x-arduino-uno.ino
  Read output ports from FS-IA6B receiver module
  Display values on Serial Monitor
  
  Channel functions by Ricardo Paiva - https://gist.github.com/werneckpaiva/
  
  DroneBot Workshop 2021
  https://dronebotworkshop.com
*/

// Used this code to check the connection between the transceiver and transmitter and
// to see which channel corresponds to what command

// Define Input Connections
#define CH2 10 // RIGHT
#define CH3 9 //LEFT MOTOR FORWARD/BACKWARD
#define CH5 2 // KILL SWITCH
#define CH6 11 // MODE SWITCH
#define CH4 12 // TURNING??

// FOR THE LEFT MOTOR
#define enA 3
#define in1A 4
#define in2A 5

// FOR THE RIGHT MOTOR
#define enB 6
#define in1B 7
#define in2B 8

// Integers to represent values from sticks and pots
int ch2Value;
int ch3Value;
int ch4Value;
bool ch5Value;
bool ch6Value;

int motorSpeedA = 0;
int motorSpeedB = 0;

// Read the number of a specified channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

// Read the switch channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue){
  int intDefaultValue = (defaultValue)? 100: 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

void setup(){
  // Set up serial monitor
  Serial.begin(115200);
  
  // Set all pins as inputs
  
  pinMode(CH2, INPUT);
  pinMode(CH3, INPUT);
  pinMode(CH5, INPUT);
  pinMode(CH4, INPUT);
  pinMode(CH6, INPUT);

  pinMode(enA, OUTPUT);
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
}

void loop(){
  // Get values for each channel
  ch6Value = readSwitch(CH6, false);

  if (ch6Value == 0)
  {
    // DRIVES TOGETHER
    // Get values for each channel
    ch2Value = readChannel(CH2, -100, 100, 0);
    ch4Value = readChannel(CH4, -100, 100, 0);
    ch5Value = readSwitch(CH5, false);

    if (ch5Value == 0){
      motorSpeedA = 0;
      analogWrite(enA, motorSpeedA);

      motorSpeedB = 0;
      analogWrite(enB, motorSpeedB);

      delay(20); 
    }
    else if ((ch2Value > 10) && ((ch4Value < 10) && (ch4Value > -10))){
      Serial.print("GOING FORWARD");
      motorSpeedA = map(ch2Value, 10, 101, 70, 180);
      motorSpeedB = motorSpeedA;
      
      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch2Value < -10) && ((ch4Value < 10) && (ch4Value > -10))){
      Serial.print("REVERSING");
      motorSpeedA = map(ch2Value, -10, -101, 70, 180);
      motorSpeedB = motorSpeedA;

      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch2Value < 10) && (ch2Value > -10)){
      motorSpeedA = motorSpeedB = 0;
      analogWrite(enA, motorSpeedA);
      analogWrite(enB, motorSpeedB);
      delay(20); 
    }
    // WANNA TURN RIGHT
    else if ((ch2Value > 10) && (ch4Value > 10)) {
      Serial.print("ENTERING RIGHT TURN");
      motorSpeedB = map(ch2Value, 10, 101, 70, 180);
      // MAKE LEFT MOTOR GO FASTER THAN RIGHT MOTOR
      motorSpeedA = motorSpeedB + 0.4*map(ch4Value, 10, 101, 70, 180);
    
      Serial.print(" | Left Motor Speed: ");
      Serial.print(motorSpeedA);

      Serial.print(" | Right Motor Speed: ");
      Serial.println(motorSpeedB);

      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    // WE REVERSE AND PUSH THE JOYSTICK TO THE LEFT
    // CAUSES A REVERSE TO THE LEFT
    else if ((ch2Value < -10) && (ch4Value < -10)) {
      Serial.print("REVERSING LEFT");
      motorSpeedA = map(ch2Value, -10, -101, 70, 180);
      motorSpeedB = motorSpeedA + 0.4*map(ch4Value, -10, -101, 70, 180);

      Serial.print(" | Left Motor Speed: ");
      Serial.print(motorSpeedA);

      Serial.print(" | Right Motor Speed: ");
      Serial.println(motorSpeedB);

      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    // WANNA TURN LEFT
    else if ((ch2Value > 10) && (ch4Value < -10)) {
      // MAKE RIGHT MOTOR GO FASTER THAN LEFT MOTOR
      motorSpeedA = map(ch2Value, 10, 101, 70, 180);
      motorSpeedB = motorSpeedA + 0.4*map(ch4Value, -10, -101, 70, 140);
      

      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    // WE REVERSE AND PUSH THE JOYSTICK TO THE RIGHT
    // CAUSES A REVERSE TO THE RIGHT
    else if ((ch2Value < -10) && (ch4Value > 10)) {
      Serial.print("REVERSING RIGHT");
      motorSpeedB = map(ch2Value, -10, -101, 70, 180);
      motorSpeedA = motorSpeedB + 0.4*map(ch4Value, 10, 101, 70, 180);
      
      Serial.print(" | Left Motor Speed: ");
      Serial.print(motorSpeedA);

      Serial.print(" | Right Motor Speed: ");
      Serial.println(motorSpeedB);

      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
  }
  else if (ch6Value == 1)
  {
    // Get values for each channel
    ch2Value = readChannel(CH2, -100, 100, 0);
    ch3Value = readChannel(CH3, -100, 100, -100);
    ch5Value = readSwitch(CH5, false);
    
    if (ch5Value == 0){
      motorSpeedA = 0;
      analogWrite(enA, motorSpeedA);

      motorSpeedB = 0;
      analogWrite(enB, motorSpeedB);

      delay(20); 
    }
    else if ((ch3Value > 10) && (ch2Value > 10)){
      motorSpeedA = map(ch3Value, 10, 101, 70, 180);
      motorSpeedB = map(ch2Value, 10, 101, 70, 180);

      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch3Value > 10) && (ch2Value < -10)){
      motorSpeedA = map(ch3Value, 10, 101, 70, 180);
      motorSpeedB = map(ch2Value, -10, -101, 70, 180);

      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch3Value < -10) && (ch2Value > 10)){
      motorSpeedA = map(ch3Value, -10, -101, 70, 180);
      motorSpeedB = map(ch2Value, 10, 101, 70, 180);

      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch3Value > 10) && ((ch2Value > -10) && (ch2Value < 10))){
      motorSpeedA = map(ch3Value, 10, 101, 70, 180);
      digitalWrite(in1A, LOW);
      digitalWrite(in2A, HIGH);
      analogWrite(enA, motorSpeedA);

      motorSpeedB = 0;
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch3Value < -10) && ((ch2Value > -10) && (ch2Value < 10))){
      motorSpeedA = map(ch3Value, -10, -101, 70, 180);
      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      motorSpeedB = 0;
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if ((ch2Value > 10) && ((ch3Value > -10) && (ch3Value < 10))){
      motorSpeedB = map(ch2Value, 10, 101, 70, 180);

      digitalWrite(in1B, HIGH);
      digitalWrite(in2B, LOW);
      analogWrite(enB, motorSpeedB);

      motorSpeedA = 0;
      analogWrite(enA, motorSpeedA);
      delay(20);
    }
    else if ((ch2Value < -10) && ((ch3Value > -10) && (ch3Value < 10))){
      motorSpeedB = map(ch2Value, -10, -101, 70, 180);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);

      motorSpeedA = 0;
      analogWrite(enA, motorSpeedA);
      delay(20);
    }
    else if ((ch3Value < -10) && (ch2Value < -10)){
      motorSpeedA = map(ch3Value, -10, -101, 70, 180);
      motorSpeedB = map(ch2Value, -10, -101, 70, 180);

      digitalWrite(in1A, HIGH);
      digitalWrite(in2A, LOW);
      analogWrite(enA, motorSpeedA);

      digitalWrite(in1B, LOW);
      digitalWrite(in2B, HIGH);
      analogWrite(enB, motorSpeedB);
      delay(20);
    }
    else if (((ch3Value < 10) && (ch3Value > -10)) && ((ch2Value < 10) && (ch2Value > -10))){
      motorSpeedA = 0;
      analogWrite(enA, motorSpeedA);

      motorSpeedB = 0;
      analogWrite(enB, motorSpeedB);

      delay(20); 
    }
  }

  // Print to Serial Monitor
  Serial.print(" | Ch2: ");
  Serial.print(ch2Value);
  Serial.print(" | Ch3: ");
  Serial.print(ch3Value);
  Serial.print(" | Ch4: ");
  Serial.print(ch4Value);
  Serial.print(" | Ch5: ");
  Serial.print(ch5Value);
  Serial.print(" | Ch6: ");
  Serial.println(ch6Value);
  
  delay(50);
}

// void drive_together(){
//   // Get values for each channel
//   ch2Value = readChannel(CH2, -100, 100, 0);
//   ch3Value = readChannel(CH3, -100, 100, -100);
//   ch4Value = readChannel(CH4, -100, 100, -100);
//   ch5Value = readSwitch(CH5, false);

//   if (ch5Value == 0){
//     motorSpeedA = 0;
//     analogWrite(enA, motorSpeedA);

//     motorSpeedB = 0;
//     analogWrite(enB, motorSpeedB);

//     delay(20); 
//   }
//   if (ch3Value > 10){
//     motorSpeedA = map(ch3Value, 10, 101, 70, 180);
//     motorSpeedB = motorSpeedA;
    
//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if (ch3Value < -10){
//     motorSpeedA = map(ch3Value, -10, -101, 70, 180);
//     motorSpeedB = motorSpeedA;

//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value < 10) && (ch3Value > -10)){
//     motorSpeedA = motorSpeedB = 0;
//     analogWrite(enA, motorSpeedA);
//     analogWrite(enB, motorSpeedB);
//     delay(20); 
//   }
//   else if ((ch3Value > 10) && (ch4 > 10)) {
//     motorSpeedB = map(ch3Value, 10, 101, 70, 140);
//     motorSpeedA = motorSpeedB + map(ch4Value, 10, 101, 70, 180);
  
//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value < -10) && (ch4 > 10)) {
//     motorSpeedB = map(ch3Value, 10, 101, 70, 140);
//     motorSpeedA = motorSpeedB + map(ch4Value, -10, -101, 70, 180);

//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value > 10) && (ch4 < -10)) {
//     motorSpeedA = motorSpeedB + map(ch3Value, -10, -101, 70, 140);
//     motorSpeedB = map(ch4Value, 10, 101, 70, 180);

//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value < 10) && (ch4 < -10)) {
//     motorSpeedA = motorSpeedB + map(ch3Value, -10, -101, 70, 140);
//     motorSpeedB = map(ch4Value, -10, -101, 70, 180);

//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }

// }

// void drive_separate() {
  
//   // Get values for each channel
//   ch2Value = readChannel(CH2, -100, 100, 0);
//   ch3Value = readChannel(CH3, -100, 100, -100);
//   ch5Value = readSwitch(CH5, false);
  
//   if (ch5Value == 0){
//     motorSpeedA = 0;
//     analogWrite(enA, motorSpeedA);

//     motorSpeedB = 0;
//     analogWrite(enB, motorSpeedB);

//     delay(20); 
//   }
//   else if ((ch3Value > 10) && (ch2Value > 10)){
//     motorSpeedA = map(ch3Value, 10, 101, 70, 180);
//     motorSpeedB = map(ch2Value, 10, 101, 70, 180);

//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value > 10) && (ch2Value < -10)){
//     motorSpeedA = map(ch3Value, 10, 101, 70, 180);
//     motorSpeedB = map(ch2Value, -10, -101, 70, 180);

//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value < -10) && (ch2Value > 10)){
//     motorSpeedA = map(ch3Value, -10, -101, 70, 180);
//     motorSpeedB = map(ch2Value, 10, 101, 70, 180);

//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value > 10) && ((ch2Value > -10) && (ch2Value < 10))){
//     motorSpeedA = map(ch3Value, 10, 101, 70, 180);
//     digitalWrite(in1A, LOW);
//     digitalWrite(in2A, HIGH);
//     analogWrite(enA, motorSpeedA);

//     motorSpeedB = 0;
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch3Value < -10) && ((ch2Value > -10) && (ch2Value < 10))){
//     motorSpeedA = map(ch3Value, -10, -101, 70, 180);
//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     motorSpeedB = 0;
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if ((ch2Value > 10) && ((ch3Value > -10) && (ch3Value < 10))){
//     motorSpeedB = map(ch2Value, 10, 101, 70, 180);

//     digitalWrite(in1B, HIGH);
//     digitalWrite(in2B, LOW);
//     analogWrite(enB, motorSpeedB);

//     motorSpeedA = 0;
//     analogWrite(enA, motorSpeedA);
//     delay(20);
//   }
//   else if ((ch2Value < -10) && ((ch3Value > -10) && (ch3Value < 10))){
//     motorSpeedB = map(ch2Value, -10, -101, 70, 180);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);

//     motorSpeedA = 0;
//     analogWrite(enA, motorSpeedA);
//     delay(20);
//   }
//   else if ((ch3Value < -10) && (ch2Value < -10)){
//     motorSpeedA = map(ch3Value, -10, -101, 70, 180);
//     motorSpeedB = map(ch2Value, -10, -101, 70, 180);

//     digitalWrite(in1A, HIGH);
//     digitalWrite(in2A, LOW);
//     analogWrite(enA, motorSpeedA);

//     digitalWrite(in1B, LOW);
//     digitalWrite(in2B, HIGH);
//     analogWrite(enB, motorSpeedB);
//     delay(20);
//   }
//   else if (((ch3Value < 10) && (ch3Value > -10)) && ((ch2Value < 10) && (ch2Value > -10))){
//     motorSpeedA = 0;
//     analogWrite(enA, motorSpeedA);

//     motorSpeedB = 0;
//     analogWrite(enB, motorSpeedB);

//     delay(20); 
//   }
// }


// if (((ch3Value < 10) && (ch3Value > -10)) && ((ch2Value < 10) && (ch2Value > -10)))