#include <Bluepad32.h>
#include <RoboClaw.h>
#include <uni.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];
HardwareSerial mySerial(2);
RoboClaw roboclaw(&mySerial, 10000);

static const char* controller_addr_string = "AC:8E:BD:12:C5:6A";
#define IN1 12
#define IN2 13
#define ENABLEA_PIN 14

const int min_motor_speed = 65, max_motor_speed = 120;
bool killSwitch = false, lastButtonState = false;

void onConnectedGamepad(GamepadPtr gp) {
    if (!myGamepads[0]) {
        myGamepads[0] = gp;
    } else {
        gp->disconnect();
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepads[0] == gp) {
        myGamepads[0] = nullptr;
        Serial.println("Gamepad disconnected. Attempting to reconnect...");
    }
}

void reconnectGamepad() {
    if (!myGamepads[0]) {
        Serial.println("Reconnecting to the known controller...");
        BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    }
}

void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT);
    Serial.begin(115200);
    mySerial.begin(38400);
    bd_addr_t controller_addr;
    sscanf_bd_addr(controller_addr_string, controller_addr);
    uni_bt_allowlist_add_addr(controller_addr);
    uni_bt_allowlist_set_enabled(true);
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
}

void stopMotors() {
    roboclaw.ForwardM1(0X80, 0);
    roboclaw.ForwardM2(0X80, 0);
    roboclaw.BackwardM1(0X80, 0);
    roboclaw.BackwardM2(0X80, 0);
}

void controlShooter(int in1, int in2, bool enable) {
    digitalWrite(ENABLEA_PIN, enable);
    analogWrite(IN1, in1);
    analogWrite(IN2, in2);
}

void loop() {
    BP32.update();
    reconnectGamepad();
    GamepadPtr gp = myGamepads[0];
    if (!gp) return;
    
    bool buttonPressed = gp->buttons() & 0x0002;
    if (buttonPressed && !lastButtonState) {
        killSwitch = !killSwitch;
        if (killSwitch) stopMotors();
    }
    lastButtonState = buttonPressed;
    
    if (killSwitch) return;
    
    int forwardReverse = 0, leftRight = 0;
    int throttle = gp->throttle(), brake = gp->brake();
    int acceleration = (throttle > 0) ? map(throttle, 0, 1023, min_motor_speed, max_motor_speed) :
                        (brake > 0) ? map(brake, 0, 1023, min_motor_speed, 10) : min_motor_speed;
    
    if (gp->axisRX() < -90) forwardReverse = map(-gp->axisRX(), 90, 512, min_motor_speed, acceleration);
    else if (gp->axisRX() > 90) forwardReverse = -map(gp->axisRX(), 90, 511, min_motor_speed, acceleration);
    
    if (gp->axisY() < -90) leftRight = -map(-gp->axisY(), 90, 512, min_motor_speed, acceleration);
    else if (gp->axisY() > 90) leftRight = map(gp->axisY(), 90, 511, min_motor_speed, acceleration);
    
    int leftMotorSpeed = constrain(forwardReverse + leftRight, -acceleration, acceleration);
    int rightMotorSpeed = constrain(forwardReverse - leftRight, -acceleration, acceleration);
    
    if (leftMotorSpeed > 0) roboclaw.ForwardM1(0X80, leftMotorSpeed);
    else roboclaw.BackwardM1(0X80, -leftMotorSpeed);
    
    if (rightMotorSpeed > 0) roboclaw.ForwardM2(0X80, rightMotorSpeed);
    else roboclaw.BackwardM2(0X80, -rightMotorSpeed);
    
    if (!forwardReverse && !leftRight) stopMotors();
    
    if (gp->buttons() & 0x0004) controlShooter(0, 140, true);
    else if (gp->buttons() & 0x0001) controlShooter(100, 0, true);
    else if (gp->buttons() & 0x0010) controlShooter(220, 0, true);
    else controlShooter(0, 0, false);
    
    delay(20);
}
