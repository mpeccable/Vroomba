 #include <Bluepad32.h>
#include <ESP32Servo.h>

static const int servo1Pin = 26;  // First servo GPIO
static const int servo2Pin = 27;  // Second servo GPIO
Servo servo3;
Servo servo4;
GamepadPtr myGamepad = nullptr;

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

    // Start at neutral position
    servo3.write(0);
    servo4.write(0);
    delay(500);  // Give servos time to reach the neutral position
}

void loop() {
    BP32.update();  // Update Bluepad32 library to check for events

    if (myGamepad) {
        // Check if either R1 or L1 is pressed
        if (myGamepad->r1() || myGamepad->l1()) {

            // Move servos forward (0° → 180°) when R1 is pressed
            if (myGamepad->r1()) {
                Serial.println("R1 pressed, moving servos forward");
                // Move the servos faster by incrementing by 30 degrees at a time
                for (int pos = 90; pos <= 180; pos += 30) {
                    servo3.write(pos);
                    servo4.write(pos);
                    Serial.print("Servo Position: ");
                    Serial.println(pos);
                    delay(1);  // Reduced delay for faster movement
                }
            }
            // Move servos backward (180° → 0°) when L1 is pressed
            else if (myGamepad->l1()) {
                Serial.println("L1 pressed, moving servos backward");
                // Move the servos faster by decrementing by 30 degrees at a time
                for (int pos = 90; pos >= 0; pos -= 30) {
                    servo3.write(pos);
                    servo4.write(pos);
                    Serial.print("Servo Position: ");
                    Serial.println(pos);
                    delay(1);  // Reduced delay for faster movement
                }
            }
        } else {
            // If neither bumper is pressed, servos stay in their current position
            Serial.println("Neither bumper pressed, keeping servos still");
            servo3.write(0);
            servo4.write(0);
        }
    }

    delay(10);  // Small delay to prevent excessive polling
}