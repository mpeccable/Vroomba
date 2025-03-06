#include <Bluepad32.h>
#include <RoboClaw.h>
#include <uni.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// Create HardwareSerial Object for Serial2
HardwareSerial mySerial(2); // UART2

RoboClaw roboclaw(&mySerial, 10000);

// Controller address here 
static const char * controller_addr_string = "AC:8E:BD:12:C5:6A";

// H-Bridge Pins 
#define IN1 13
#define IN2 12
#define ENABLEA_PIN 14 

int motorSpeed = 0; // initially set the motor speed to 0

int forward_max = -512;
int forward_deadspace = 90;

int reverse_deadspace = 90;
int reverse_max = 511;

int min_motor_speed = 65; // minimum speed for the robot
int max_motor_speed = 120; // maximum speed for the robot 
int acceleration = min_motor_speed;

int left_max = -512;
int left_deadspace = 90;
int right_max = 511;
int right_deadspace = 90;

bool killSwitch = false;
bool lastButtonState = false;

// This callback gets called any time a new gamepad is connected
void onConnectedGamepad(GamepadPtr gp) {
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("CALLBACK: Gamepad tried to connect with address: %02X:%02X:%02X:%02X:%02X:%02X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Check if this gamepad matches the allowlisted address
    if (myGamepads[0] == nullptr) {
        Serial.println("CALLBACK: Gamepad connected to this ESP32.");
        myGamepads[0] = gp;
    } else {
        Serial.println("Another gamepad tried to connect, but this ESP32 only allows one.");
        gp->disconnect(); // Reject additional controllers
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT);
  
    Serial.begin(115200);
    mySerial.begin(38400);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

    Serial.println("Checking allowlist...");
    bd_addr_t controller_addr;
    sscanf_bd_addr(controller_addr_string, controller_addr);
    uni_bt_allowlist_add_addr(controller_addr);

    Serial.printf("Added to allowlist: %s\n", controller_addr_string);
    uni_bt_allowlist_set_enabled(true);
    
    Serial.printf("Allowlist enabled: %d\n", uni_bt_allowlist_is_enabled());
    
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    //BP32.forgetBluetoothKeys();
}

void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepads[0] == gp) {
        Serial.println("CALLBACK: Gamepad disconnected from this ESP32.");
        myGamepads[0] = nullptr;
    }
}


// Arduino loop function. Runs in CPU 1
void loop() {
    BP32.update();

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            bool buttonPressed = (myGamepad->buttons() == 0x0002); // this is the B button

            if (buttonPressed && !lastButtonState) {  // Toggle the kill switch
                killSwitch = !killSwitch;
                Serial.printf("Kill Switch: %s\n", killSwitch ? "ON" : "OFF");

                if (killSwitch) {
                  // Stops drivetrain movement 
                    roboclaw.ForwardM1(0X80, 0);
                    roboclaw.ForwardM2(0X80, 0);
                    roboclaw.BackwardM1(0X80, 0);
                    roboclaw.BackwardM2(0X80, 0);
                  // Stops shooter movement 
                    analogWrite(IN1, 0);
                    analogWrite(IN2, 0);
                    digitalWrite(ENABLEA_PIN, LOW)
                }
            }
            lastButtonState = buttonPressed;

            if (!killSwitch) {
                int forwardReverse = 0;
                int leftRight = 0;
                
                int throttleValue = myGamepad->throttle();
                Serial.printf("Throttle Value: %d\n", throttleValue);

                int brakeValue = myGamepad->brake();
                Serial.printf("Brake Value: %d\n", brakeValue);

                if (throttleValue > 0) { 
                    acceleration = map(throttleValue, 0, 1023, min_motor_speed, max_motor_speed);
                    Serial.printf("Mapped Acceleration: %d\n", acceleration);
                } else if (brakeValue > 0) {
                    acceleration = map(brakeValue, 0, 1023, min_motor_speed, 10);
                    Serial.printf("Mapped Deceleration: %d\n", acceleration);
                }
                else {
                    acceleration = min_motor_speed; // Default (which is minimum motor speed) if no throttle
                }
        
                // Forward and Reverse Drive (use RX instead of Y)
                if (myGamepad->axisRX() <= -forward_deadspace) {
                    forwardReverse = map(abs(myGamepad->axisRX()), forward_deadspace, abs(forward_max), min_motor_speed, acceleration);
                } 
                else if (myGamepad->axisRX() >= reverse_deadspace) {
                    forwardReverse = -map(abs(myGamepad->axisRX()), reverse_deadspace, reverse_max, min_motor_speed, acceleration);
                }
                
                // Left and Right Steering (use Y instead of RX)
                if (myGamepad->axisY() <= -left_deadspace) {
                    leftRight = -map(abs(myGamepad->axisY()), left_deadspace, abs(left_max), min_motor_speed, acceleration);
                } 
                else if (myGamepad->axisY() >= right_deadspace) {
                    leftRight = map(abs(myGamepad->axisY()), right_deadspace, right_max, min_motor_speed, acceleration);
                }
                
                // Shooter Mechanism
                
                  // Have it constantly spinning slowly for intake when right bumper is pressed down 
                  if(myGamepad->buttons() == 0x0004){ //X button 
                    digitalWrite(ENABLEA_PIN, HIGH);
                    analogWrite(IN1, 0); 
                    analogWrite(IN2, 60);
                  }
                  
                  // If the left bumper is pressed --> outake occurs at a fast speed  
                  else if(myGamepad->buttons() == 0x0001)  {  //A button 
                    digitalWrite(ENABLEA_PIN, HIGH);
                    analogWrite(IN2, 0);
                    analogWrite(IN1, 60);
                  }

                  else {
                    digitalWrite(ENABLEA_PIN, LOW);
                    analogWrite(IN1, 0);
                    analogWrite(IN2, 0);
                  }

                // Combine Forward/Reverse and Left/Right
                int leftMotorSpeed = constrain(forwardReverse + leftRight, -acceleration, acceleration);
                int rightMotorSpeed = constrain(forwardReverse - leftRight, -acceleration, acceleration);

                Serial.printf("Left Motor: %d, Right Motor: %d\n", leftMotorSpeed, rightMotorSpeed);


                // Motor Speeds (given above combination of forward/reverse and left/right)
                if (leftMotorSpeed > 0) {
                    roboclaw.ForwardM1(0X80, leftMotorSpeed);
                } else {
                    roboclaw.BackwardM1(0X80, abs(leftMotorSpeed));
                }

                if (rightMotorSpeed > 0) {
                    roboclaw.ForwardM2(0X80, rightMotorSpeed);
                } else {
                    roboclaw.BackwardM2(0X80, abs(rightMotorSpeed));
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
