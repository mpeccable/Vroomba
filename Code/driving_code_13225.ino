#include <Bluepad32.h>
#include <RoboClaw.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

//Create HardwareSerial Object for Serial2
HardwareSerial mySerial(2); //UART2

RoboClaw roboclaw(&mySerial, 10000);

// Variable names

int motorSpeed = 0; // initially set the motor speed to 0 

int forward_max = -512;
int forward_deadspace = -25;

int reverse_deadspace = 25;
int reverse_max = 511;

int min_motor_speed = 70; //(can set to 70 for now)
int max_motor_speed = 150; //(can set to 150 for now)

int left_max = -512;
int left_deadspace = -25;
int right_max = 511;
int right_deadspace = 25;

bool killSwitch = false;
bool lastButtonState = false;

// This callback gets called any time a new gamepad is connected, up t0 4 at the same time.
void onConnectedGamepad(GamepadPtr gp) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == nullptr) {
            Serial.printf("CALLBACK: Gamepad is connected, index=%d\n", i);

            GamepadProperties properties = gp->getProperties();
            Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n", gp->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myGamepads[i] = gp;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Gamepad connected, but could not found empty slot");
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    bool foundGamepad = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == gp) {
            Serial.printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
            myGamepads[i] = nullptr;
            foundGamepad = true;
            break;
        }
    }

    if (!foundGamepad) {
        Serial.println("CALLBACK: Gamepad disconnected, but not found in myGamepads");
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    mySerial.begin(38400);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // "forgetBluetoothKeys()" called when the user performs a "device factory reset", or similar.
    BP32.forgetBluetoothKeys();
}

// Arduino loop function. Runs in CPU 1
void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            // Debugging: Check button value
            Serial.printf("Button state: %02X\n", myGamepad->buttons());

            bool buttonPressed = (myGamepad->buttons() == 0x0002);

            // Toggle kill switch on button press (only when first pressed)
            if (buttonPressed && !lastButtonState) {  
                killSwitch = !killSwitch;
                Serial.printf("Kill Switch: %s\n", killSwitch ? "ON" : "OFF");

                // Reset motor speed when re-enabling movement
                if (!killSwitch) {
                    motorSpeed = 0;
                }
            }
            lastButtonState = buttonPressed; // Store button state

            if (killSwitch) {
                while (motorSpeed > 0) {
                    motorSpeed -= 5; 
                    if (motorSpeed < 0) motorSpeed = 0; 

                    roboclaw.ForwardM1(0X80, motorSpeed);
                    roboclaw.ForwardM2(0X80, motorSpeed);
                    roboclaw.BackwardM1(0X80, motorSpeed);
                    roboclaw.BackwardM2(0X80, motorSpeed);

                    delay(50); // 
                }

                // Ensure motors are fully stopped
                roboclaw.ForwardM1(0X80, 0);
                roboclaw.ForwardM2(0X80, 0);
                roboclaw.BackwardM1(0X80, 0);
                roboclaw.BackwardM2(0X80, 0);
            } 
            else {
                // Normal motor control
                if (myGamepad->axisY() <= 0) { //less than or equal to zero since the forward is reversed polarities (negative)
                    motorSpeed = map(abs(myGamepad->axisY()), forward_deadspace, forward_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", motorSpeed);
                    roboclaw.ForwardM1(0X80, motorSpeed);
                    roboclaw.ForwardM2(0X80, motorSpeed);
                } 
                else if (myGamepad->axisY() > 0) { //greater than zero since the reverse is reversed polarities (positive)
                    motorSpeed = map(abs(myGamepad->axisY()), reverse_deadspace, reverse_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", motorSpeed);
                    roboclaw.BackwardM1(0X80, motorSpeed);
                    roboclaw.BackwardM2(0X80, motorSpeed);
                }
                
                // Left and Right Steering
                if (myGamepad->axisRX() < left_deadspace) { 
                    int turnSpeed = map(abs(myGamepad->axisRX()), left_deadspace, left_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", turnSpeed);
                    roboclaw.ForwardM1(0X80, turnSpeed);  // Left motor forward
                    roboclaw.BackwardM2(0X80, turnSpeed); // Right motor backward
                } 
                else if (myGamepad->axisRX() > right_deadspace) { 
                    int turnSpeed = map(abs(myGamepad->axisRX()), right_deadspace, right_max, min_motor_speed, max_motor_speed);
                    Serial.printf("Motor Speed: %d\n", turnSpeed);
                    roboclaw.ForwardM2(0X80, turnSpeed);  // Right motor forward
                    roboclaw.BackwardM1(0X80, turnSpeed); // Left motor backward
                }
            }
            delay(40);  
        }
    }
    delay(100);
}
