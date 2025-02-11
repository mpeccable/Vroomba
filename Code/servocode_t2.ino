#include <Bluepad32.h>
#include <ESP32Servo.h>

static const int servo1Pin = 26;  // First servo GPIO
static const int servo2Pin = 27;  // Second servo GPIO
Servo servo3;
Servo servo4;
GamepadPtr myGamepad = nullptr;

// Store the last known servo position
static int lastPosition = 90;  
static int prevPosition = 90;  // Track previous position to avoid redundant writes

// Callback for when a controller is connected
void onConnect(GamepadPtr gamepad) {
    Serial.println("Controller connected!");
    myGamepad = gamepad;
}

// Callback for when a controller is disconnected
void onDisconnect(GamepadPtr gamepad) {
    Serial.println("Controller disconnected!");
    myGamepad = nullptr;
}

void setup() {
    Serial.begin(115200);

    // Initialize Bluepad32 with the connection and disconnection callbacks
    BP32.setup(onConnect, onDisconnect);

    // Attach both servos
    servo3.attach(servo1Pin);
    servo4.attach(servo2Pin);

    // Start at a neutral position
    servo3.write(lastPosition);
    servo4.write(lastPosition);
    delay(500);  // Give servos time to reach the neutral position
}

void loop() {
    BP32.update();  // Update Bluepad32 library to check for events

    if (myGamepad) {
        if (myGamepad->r1()) {
            Serial.println("R1 pressed, moving servos forward");
            lastPosition = min(lastPosition + 30, 180);  // Increment but cap at 180
        } 
        else if (myGamepad->l1()) {
            Serial.println("L1 pressed, moving servos backward");
            lastPosition = max(lastPosition - 30, 0);  // Decrement but cap at 0
        } 

        // Only update the servos if the position has changed
        if (lastPosition != prevPosition) {
            servo3.write(lastPosition);
            servo4.write(lastPosition);
            Serial.print("Servo Position: ");
            Serial.println(lastPosition);
            prevPosition = lastPosition;  // Update previous position
        }
    }

    delay(10);  // Small delay to prevent excessive polling
}
