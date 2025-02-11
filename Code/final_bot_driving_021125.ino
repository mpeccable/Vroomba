#include <Bluepad32.h>
#include <RoboClaw.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// Create HardwareSerial Object for Serial2
HardwareSerial mySerial(2); // UART2

RoboClaw roboclaw(&mySerial, 10000);

// Variable names
int motorSpeed = 0; // initially set the motor speed to 0

int forward_max = -512;
int forward_deadspace = 40;

int reverse_deadspace = 40;
int reverse_max = 511;

int min_motor_speed = 30; // minimum speed for the robot
int max_motor_speed = 70; // maximum speed for the robot 

int left_max = -512;
int left_deadspace = 40;
int right_max = 511;
int right_deadspace = 40;

bool killSwitch = false;
bool lastButtonState = false;

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

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    mySerial.begin(38400);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    BP32.forgetBluetoothKeys();
}

// Arduino loop function. Runs in CPU 1
void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            bool buttonPressed = (myGamepad->buttons() == 0x0002);

            if (buttonPressed && !lastButtonState) {  // Toggle the kill switch
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

                // Forward and Reverse Drive (use RX instead of Y)
                if (myGamepad->axisRX() <= -forward_deadspace) {
                    forwardReverse = map(abs(myGamepad->axisRX()), forward_deadspace, abs(forward_max), min_motor_speed, max_motor_speed);
                } 
                else if (myGamepad->axisRX() >= reverse_deadspace) {
                    forwardReverse = -map(abs(myGamepad->axisRX()), reverse_deadspace, reverse_max, min_motor_speed, max_motor_speed);
                }
                
                // Left and Right Steering (use Y instead of RX)
                if (myGamepad->axisY() <= -left_deadspace) {
                    leftRight = -map(abs(myGamepad->axisY()), left_deadspace, abs(left_max), min_motor_speed, max_motor_speed);
                } 
                else if (myGamepad->axisY() >= right_deadspace) {
                    leftRight = map(abs(myGamepad->axisY()), right_deadspace, right_max, min_motor_speed, max_motor_speed);
                }

//
//                // Forward and Reverse Drive
//                
//                if (myGamepad->axisY() <= -forward_deadspace) {
//                    forwardReverse = map(abs(myGamepad->axisY()), forward_deadspace, abs(forward_max), min_motor_speed, max_motor_speed);
//                    Serial.printf("Motor Speed: %d\n", forwardReverse);
//                } 
//                else if (myGamepad->axisY() >= reverse_deadspace) {
//                    forwardReverse = -map(abs(myGamepad->axisY()), reverse_deadspace, reverse_max, min_motor_speed, max_motor_speed);
//                    Serial.printf("Motor Speed: %d\n", forwardReverse);
//                }
//
//                // Left and Right Steering 
//                if (myGamepad->axisRX() <= -left_deadspace) {
//                    leftRight = -map(abs(myGamepad->axisRX()), left_deadspace, abs(left_max), min_motor_speed, max_motor_speed);
//                    Serial.printf("Motor Speed: %d\n", leftRight);
//                } 
//                else if (myGamepad->axisRX() >= right_deadspace) {
//                    leftRight = map(abs(myGamepad->axisRX()), right_deadspace, right_max, min_motor_speed, max_motor_speed);
//                    Serial.printf("Motor Speed: %d\n", leftRight);
//                }

                // Combine Forward/Reverse and Left/Right
                int leftMotorSpeed = constrain(forwardReverse + leftRight, -max_motor_speed, max_motor_speed);
                int rightMotorSpeed = constrain(forwardReverse - leftRight, -max_motor_speed, max_motor_speed);

                // Motor Speeds (given above combination of forward/reverse and left/right)
                if (leftMotorSpeed > 0) {
                    roboclaw.ForwardM1(0X80, leftMotorSpeed);
                    //Serial.printf("Motor Speed: %d\n", leftMotorSpeed);
                } else {
                    roboclaw.BackwardM1(0X80, abs(leftMotorSpeed));
                    //Serial.printf("Motor Speed: %d\n", leftMotorSpeed);
                }

                if (rightMotorSpeed > 0) {
                    roboclaw.ForwardM2(0X80, rightMotorSpeed);
                    //Serial.printf("Motor Speed: %d\n", rightMotorSpeed);
                } else {
                    roboclaw.BackwardM2(0X80, abs(rightMotorSpeed));
                    //Serial.printf("Motor Speed: %d\n", rightMotorSpeed);
                }

                // Stop motors if no input received 
                if (forwardReverse == 0 && leftRight == 0) {
                    roboclaw.ForwardM1(0X80, 0);
                    roboclaw.ForwardM2(0X80, 0);
                    roboclaw.BackwardM1(0X80, 0);
                    roboclaw.BackwardM2(0X80, 0);
                }
            }
        }
    }
    delay(40);
}
