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
#define CH5 2

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
bool ch5Value;

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

  pinMode(enA, OUTPUT);
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
}


void loop() {
  
  // Get values for each channel
  ch2Value = readChannel(CH2, -100, 100, 0);
  ch3Value = readChannel(CH3, -100, 100, -100);
  
  if ((ch3Value > 10) && (ch2Value > 10)){
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

  // Print to Serial Monitor
  Serial.print(" | Ch2: ");
  Serial.print(ch2Value);
  Serial.print(" | Ch3: ");
  Serial.print(ch3Value);
  Serial.print(" | Ch5: ");
  Serial.println(ch5Value);
  
  delay(50);
}


// if (((ch3Value < 10) && (ch3Value > -10)) && ((ch2Value < 10) && (ch2Value > -10)))