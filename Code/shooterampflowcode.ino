#include <Bluepad32.h>

// Motor A
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14; 

// PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];  // Store connected gamepads

void onConnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == nullptr) {
            myGamepads[i] = gp;
            Serial.println("Gamepad connected.");
            return;
        }
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == gp) {
            myGamepads[i] = nullptr;
            Serial.println("Gamepad disconnected.");
            return;
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Set motor pins as outputs
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enable1Pin, OUTPUT);

    // Configure LEDC PWM
    ledcSetup(pwmChannel, freq, resolution);
    ledcAttachPin(enable1Pin, pwmChannel);

    // Initialize Bluepad32
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.forgetBluetoothKeys();  // Optional to clear paired devices
    Serial.println("Setup complete. Waiting for gamepad...");
}

void loop() {
    BP32.update();  // Check for gamepad updates

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            Serial.println("Gamepad active.");

            if (myGamepad->r1()) {  // Right bumper (RB/R1)
                Serial.println("Right bumper pressed: Motor forward");
                digitalWrite(motor1Pin1, LOW);
                digitalWrite(motor1Pin2, HIGH);
                ledcWrite(pwmChannel, dutyCycle);  // Motor speed forward
            } 
            else if (myGamepad->l1()) {  // Left bumper (LB/L1)
                Serial.println("Left bumper pressed: Motor backward");
                digitalWrite(motor1Pin1, HIGH);
                digitalWrite(motor1Pin2, LOW);
                ledcWrite(pwmChannel, dutyCycle);  // Motor speed backward
            } 
            else {
                Serial.println("No bumper pressed: Motor stopped");
                digitalWrite(motor1Pin1, LOW);
                digitalWrite(motor1Pin2, LOW);
                ledcWrite(pwmChannel, 0);  // Stop motor
            }
        }
    }

    vTaskDelay(1);  // Prevent watchdog reset
}
