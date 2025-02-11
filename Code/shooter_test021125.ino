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

#define enA 19
#define in1 5
#define in2 18

int motorSpeedA = 100;


// This callback gets called any time a new gamepad is connected, up to 4 at the same time.
void onConnectedGamepad(GamepadPtr gp) {
    // Check if a gamepad is already connected
    if (myGamepads[0] == nullptr) {
        Serial.println("CALLBACK: Gamepad connected to this ESP32.");
        
        GamepadProperties properties = gp->getProperties();
        Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n", gp->getModelName().c_str(), properties.vendor_id, properties.product_id);

        myGamepads[0] = gp; // Assign only one gamepad
    } else {
        Serial.println("Another gamepad tried to connect, but this ESP32 only allows one.");
        gp->disconnect(); // Reject additional controllers
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepads[0] == gp) {
        Serial.println("CALLBACK: Gamepad disconnected from this ESP32.");
        myGamepads[0] = nullptr;
    }
}


void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void loop() {
  motorSpeedA = 40;

  if(myGamepad->buttons() == 0x0080)  {
    intake = map(
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  
  else if(myGamepad->buttons()== 0x0040) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }

  // in1 HIGH and in2 LOW for rotating clockwise when looking into motor shaft
//  digitalWrite(in1, HIGH);
//  digitalWrite(in2, LOW);

  // Send a pwm signal to en of the channel you want to power with the
  analogWrite(enA, motorSpeedA);  // Send PWM signal to motor A
  }
