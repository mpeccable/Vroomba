#include <Bluepad32.h>
#include <Stepper.h>
#include <RoboClaw.h>

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

// RoboClaw Variables and Setup
HardwareSerial mySerial(2); // UART2
RoboClaw roboclaw(&mySerial, 10000);

// Variable names for motor speed
int forward_max = -512;
int forward_deadspace = 40;
int reverse_deadspace = 40;
int reverse_max = 511;
int min_motor_speed = 30;
int max_motor_speed = 70;
int left_max = -512;
int left_deadspace = 40;
int right_max = 511;
int right_deadspace = 40;
bool killSwitch = false;
bool lastButtonState = false;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];  // Store connected gamepads

// Callback for when a gamepad connects
void onConnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == nullptr) {
            myGamepads[i] = gp;
            Serial.println("Gamepad connected.");
            return;
        }
    }
}

// Callback for when a gamepad disconnects
void onDisconnectedGamepad(GamepadPtr gp) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == gp) {
            myGamepads[i] = nullptr;
            Serial.println("Gamepad disconnected.");
            return;
        }
    }
}

// Stepper Motor Task (Runs in Parallel)
void moveStepperTask(void* parameter) {
    Serial.println("Stepper motor running...");
    myStepper.step(stepsPerRevolution);
    Serial.println("Stepper motor stopped.");
    vTaskDelete(NULL);
}

// Setup function
void setup() {
    Serial.begin(115200);
    mySerial.begin(38400);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

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
    BP32.forgetBluetoothKeys();

    Serial.println("Setup complete. Waiting for gamepad...");
}

// Loop function
void loop() {
    BP32.update();  // Check for gamepad updates

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            bool buttonPressed = (myGamepad->buttons() == 0x0002);

            // Kill Switch (toggle ON/OFF)
            if (buttonPressed && !lastButtonState) {
                killSwitch = !killSwitch;
                Serial.printf("Kill Switch: %s\n", killSwitch ? "ON" : "OFF");

                if (killSwitch) {
                    roboclaw.ForwardM1(0X80, 0);
                    roboclaw.ForwardM2(0X80, 0);
                    roboclaw.BackwardM1(0X80, 0);
                    roboclaw.BackwardM2(0X80, 0);
                }
            }
            lastButtonState = buttonPressed;

            if (!killSwitch) {
                int forwardReverse = 0;
                int leftRight = 0;

                // Forward and Reverse Drive
                if (myGamepad->axisY() <= -forward_deadspace) {
                    forwardReverse = map(abs(myGamepad->axisY()), forward_deadspace, abs(forward_max), min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", forwardReverse);
                } else if (myGamepad->axisY() >= reverse_deadspace) {
                    forwardReverse = -map(abs(myGamepad->axisY()), reverse_deadspace, reverse_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", forwardReverse);
                }

                // Left and Right Steering
                if (myGamepad->axisRX() <= -left_deadspace) {
                    leftRight = -map(abs(myGamepad->axisRX()), left_deadspace, abs(left_max), min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", leftRight);
                } else if (myGamepad->axisRX() >= right_deadspace) {
                    leftRight = map(abs(myGamepad->axisRX()), right_deadspace, right_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", leftRight);
                }

                // Combine Forward/Reverse and Left/Right
                int leftMotorSpeed = constrain(forwardReverse + leftRight, -max_motor_speed, max_motor_speed);
                int rightMotorSpeed = constrain(forwardReverse - leftRight, -max_motor_speed, max_motor_speed);

                // RoboClaw Motor Speeds
                if (leftMotorSpeed > 0) {
                    roboclaw.ForwardM1(0X80, leftMotorSpeed);
                    Serial.printf("Motor Speed: %d\n", leftMotorSpeed);
                } else {
                    roboclaw.BackwardM1(0X80, abs(leftMotorSpeed));
                    Serial.printf("Motor Speed: %d\n", leftMotorSpeed);
                }

                if (rightMotorSpeed > 0) {
                    roboclaw.ForwardM2(0X80, rightMotorSpeed);
                    Serial.printf("Motor Speed: %d\n", rightMotorSpeed);
                } else {
                    roboclaw.BackwardM2(0X80, abs(rightMotorSpeed));
                    Serial.printf("Motor Speed: %d\n", rightMotorSpeed);
                }

                // Stop motors if no input received
                if (forwardReverse == 0 && leftRight == 0) {
                    roboclaw.ForwardM1(0X80, 0);
                    roboclaw.ForwardM2(0X80, 0);
                    roboclaw.BackwardM1(0X80, 0);
                    roboclaw.BackwardM2(0X80, 0);
                }
            }

            // Right Bumper (R1/RB) - DC Motor Control
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
    delay(40);
}
