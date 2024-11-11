/*
Maxx Ibarra
Code for ME 72 Team Vroombas

This code has the basics for controlling our motor with our motor controller.
For testing, I used a 12 VDC power supply pluged into a 120 VAC outlet and 
and Arduino Uno Microcontroller. 

Very similar to a blink sketch for an LED, but with motor on/off cycles. 

On at one speed in one direction for duration
Off for duration
On at same speed in opposite direction for duration
Off for duration
repeat ad nauseum 

Schematics included as picture in directory
*/

#define enA 3
#define in1 4
#define in2 5

int motorSpeedA = 100;

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void loop() {
  
  // motorSpeed values range between 0 and 255 where 0 is MIN RPM and 255 is MAX RPM
  motorSpeedA = 40;

  // in1 HIGH and in2 LOW for rotating clockwise when looking into motor shaft
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  
  while (motorSpeedA < 255) {
    analogWrite(enA, motorSpeedA); 
    delay(50);
    motorSpeedA ++;
  }

  if (motorSpeedA == 255) {
    motorSpeedA = 0;
    delay(1000);
    motorSpeedA = 40;
  }
}
