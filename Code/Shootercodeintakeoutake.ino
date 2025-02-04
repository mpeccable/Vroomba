#include <Bluepad32.h>
#include <Stepper.h>

// DC Motor (H-Bridge) Pins
#define MOTOR1_PIN1 27 
#define MOTOR1_PIN2 26 
#define ENABLE1_PIN 14 

// Stepper Motor (ULN2003) Pins
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 21

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// PWM properties for DC Motor
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

// **Step Motor Task: Runs in Parallel Without Blocking**
void moveStepperTask(void* parameter) {
    Serial.println("Stepper motor running...");
    myStepper.step(stepsPerRevolution);
    Serial.println("Stepper motor stopped.");
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);

    // Setup DC Motor Pins
    pinMode(MOTOR1_PIN1, OUTPUT);
    pinMode(MOTOR1_PIN2, OUTPUT);
    pinMode(ENABLE1_PIN, OUTPUT);

    // Setup PWM for DC Motor
    ledcSetup(pwmChannel, freq, resolution);
    ledcAttachPin(ENABLE1_PIN, pwmChannel);

    // Setup Stepper Motor Speed
    myStepper.setSpeed(20);

    // Setup Bluepad32 for gamepad connection
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    //BP32.forgetBluetoothKeys();  // Optional: Clear paired devices

    Serial.println("Setup complete. Waiting for gamepad...");
}

void loop() {
    BP32.update();  // Check for gamepad updates

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            Serial.println("Gamepad active.");

            //Right Bumper (R1/RB) - DC Motor Control
            if (myGamepad->r1()) {  
                Serial.println("Right bumper pressed: DC Motor ON");
                digitalWrite(MOTOR1_PIN1, LOW);
                digitalWrite(MOTOR1_PIN2, HIGH);
                ledcWrite(pwmChannel, dutyCycle);
            } else {
                Serial.println("Right bumper released: DC Motor OFF");
                digitalWrite(MOTOR1_PIN1, LOW);
                digitalWrite(MOTOR1_PIN2, LOW);
                ledcWrite(pwmChannel, 0);
            }

            // Left Bumper (L1/LB) - Stepper Motor Control
            if (myGamepad->l1()) {  
                Serial.println("Left bumper (L1) pressed: Running stepper motor.");
                xTaskCreatePinnedToCore(moveStepperTask, "StepperTask", 2048, NULL, 1, NULL, 0);
            }
        }
    }

    vTaskDelay(1);  // Prevent watchdog reset
}
