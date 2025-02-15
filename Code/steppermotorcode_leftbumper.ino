#include <Bluepad32.h>
#include <Stepper.h>

const int stepsPerRevolution = 2048;

#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 21

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// Flag to indicate if the stepper motor is running
volatile bool isStepperRunning = false;

void onConnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == nullptr) {
            myGamepads[i] = gp;
            Serial.println("Gamepad connected");
            return;
        }
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == gp) {
            myGamepads[i] = nullptr;
            Serial.println("Gamepad disconnected");
            return;
        }
    }
}

void moveStepperTask(void* parameter) {
    myStepper.step(stepsPerRevolution);
    isStepperRunning = false;
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    myStepper.setSpeed(5);
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.forgetBluetoothKeys();
    Serial.println("Setup complete");
}

void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            if (myGamepad->l1() && !isStepperRunning) {  // Left Bumper (L1/LB)
                Serial.println("Left bumper (L1) pressed: Starting stepper motor task.");
                isStepperRunning = true;
                xTaskCreatePinnedToCore(moveStepperTask, "StepperTask", 2048, NULL, 1, NULL, 0);
            }
        }
    }

    vTaskDelay(1);
}
