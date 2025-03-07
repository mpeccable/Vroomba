#include <Bluepad32.h>
#include <RoboClaw.h>
#include <uni.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];
HardwareSerial mySerial(2);
RoboClaw roboclaw(&mySerial, 10000);
static const char *controller_addr_string = "AC:8E:BD:12:C5:6A"; //controller address for the chipwich shooter here 

//setting the pins for the H-bridge Motor 
#define IN1 12
#define IN2 13
#define ENABLEA_PIN 14 
#define ENPIN 27 //ESP32 reset pin 

int deadspace = 90;
int_max = 511;
int min_motor_speed = 65, max_motor_speed = 120;
bool killSwitch = false, lastButtonState = false;

void onConnectedGamepad(GamepadPtr gp) {
    if (!myGamepads[0]) { // 
      myGamepads[0] = gp;
    }
    else { 
      gp->disconnect();
    }
}
void onDisconnectedGamepad(GamepadPtr gp) { 
    if (myGamepads[0] == gp) {
      myGamepads[0] = nullptr;
    }
    //stop all movement 
    roboclaw.ForwardM1(0X80, 0);
    roboclaw.ForwardM2(0X80, 0);
    roboclaw.BackwardM1(0X80, 0);
    roboclaw.BackwardM2(0X80, 0);
    //stops shooter movement 
    analogWrite(IN1, 0); analogWrite(IN2, 0);
    digitalWrite(ENABLEA_PIN, LOW);
    delay(150); pinMode(ENPIN, OUTPUT);
    digitalWrite(ENPIN, LOW); delay(10);
    digitalWrite(ENPIN, HIGH); // reset the esp32 to try and reconnect 
}

void setup() {
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT); pinMode(ENPIN, OUTPUT);
    bd_addr_t controller_addr;
    sscanf_bd_addr(controller_addr_string, controller_addr);
    uni_bt_allowlist_add_addr(controller_addr);
    uni_bt_allowlist_set_enabled(true);
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
}

void loop() {
    GamepadPtr myGamepad = myGamepads[0];
    if (!myGamepad) return;
    
    bool buttonPressed = (myGamepad->buttons() == 0x0002);
    if (buttonPressed && !lastButtonState) killSwitch = !killSwitch;
    lastButtonState = buttonPressed;
    
    if (killSwitch || !myGamepad->isConnected()) {
        roboclaw.ForwardM1(0X80, 0);
        roboclaw.ForwardM2(0X80, 0);
        roboclaw.BackwardM1(0X80, 0);
        roboclaw.BackwardM2(0X80, 0);
        analogWrite(IN1, 0); analogWrite(IN2, 0);
        digitalWrite(ENABLEA_PIN, LOW);
    }
    
    int throttle = myGamepad->throttle(), brake = myGamepad->brake();
    int acceleration = (throttle >= brake) ?
        map(throttle, 0, 1023, min_motor_speed, max_motor_speed) :
        map(brake, 0, 1023, min_motor_speed, 12);
    
    int forwardReverse = 0, leftRight = 0;
    if (abs(myGamepad->axisRX()) >= deadspace)
        forwardReverse = (myGamepad->axisRX() > 0 ? -1 : 1) *
            map(abs(myGamepad->axisRX()), deadspace, int_max, min_motor_speed, acceleration);
    
    if (abs(myGamepad->axisY()) >= deadspace)
        leftRight = (myGamepad->axisY() > 0 ? 1 : -1) *
            map(abs(myGamepad->axisY()), deadspace, int_max, min_motor_speed, acceleration);
    
    int leftMotorSpeed = constrain(forwardReverse + leftRight, -acceleration, acceleration);
    int rightMotorSpeed = constrain(forwardReverse - leftRight, -acceleration, acceleration);
    
    leftMotorSpeed > 0 {
      roboclaw.ForwardM1(0X80, leftMotorSpeed) 
      roboclaw.BackwardM1(0X80, abs(leftMotorSpeed));
    }
    rightMotorSpeed > 0{
      roboclaw.ForwardM2(0X80, rightMotorSpeed) :
    }
                           roboclaw.BackwardM2(0X80, abs(rightMotorSpeed));
    
    if (!forwardReverse && !leftRight) {
        roboclaw.ForwardM1(0X80, 0); roboclaw.ForwardM2(0X80, 0);
        roboclaw.BackwardM1(0X80, 0); roboclaw.BackwardM2(0X80, 0);
    }
    
    int btn = myGamepad->buttons();
    if (btn == 0x0004) { digitalWrite(ENABLEA_PIN, HIGH); analogWrite(IN1, 0); analogWrite(IN2, 140); }
    else if (btn == 0x0001) { digitalWrite(ENABLEA_PIN, HIGH); analogWrite(IN2, 0); analogWrite(IN1, 100); }
    else if (btn == 0x0010) { digitalWrite(ENABLEA_PIN, HIGH); analogWrite(IN2, 0); analogWrite(IN1, 220); }
    else { digitalWrite(ENABLEA_PIN, LOW); analogWrite(IN1, 0); analogWrite(IN2, 0); }
    
    delay(5);
}
