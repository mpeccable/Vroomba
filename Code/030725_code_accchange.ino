#include <Bluepad32.h>
#include <RoboClaw.h>
#include <uni.h>

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];
HardwareSerial mySerial(2);
RoboClaw roboclaw(&mySerial, 10000);

static const char* controller_addr_string = "AC:8E:BD:12:C5:6A"; //chipwich controller address

//setting the pins for the H-bridge Motor 
#define IN1 12
#define IN2 13
#define ENABLEA_PIN 14
#define ENPIN 27 //ESP32 reset pin 

//setting the variables for motor speed and deadspaces (int), aswell as kill switch mechanism (bool)
int min_speed = 65; // baseline speed
int max_speed = 120; //acceleration 

int forwardReverse = 0;
int leftRight = 0;

int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int acceleration = 0;

bool killSwitch = false;
bool lastButtonState = false;

// Connecting for Gamepad 
void onConnectedGamepad(GamepadPtr gp) {
    if (!myGamepads[0]) {
        myGamepads[0] = gp;
    } else {
        gp->disconnect();
    }
}

// Disconnecting for Gamepad 
void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepads[0] == gp) {
        myGamepads[0] = nullptr;
    }
    reconnectGamepad();
    delay(5);
    
    if (!myGamepads[0]){
      digitalWrite(ENPIN, HIGH); //reset ESP32 to reconnect and try again 
    }
}

// Reconnecting for Gamepad 
void reconnectGamepad() {
    if (!myGamepads[0]) {
        BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    }
}

// Setup for controller bluetooth 
void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENABLEA_PIN, OUTPUT);
    mySerial.begin(38400);
    bd_addr_t controller_addr;
    sscanf_bd_addr(controller_addr_string, controller_addr);
    uni_bt_allowlist_add_addr(controller_addr);
    uni_bt_allowlist_set_enabled(true);
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    GamepadPtr gp = myGamepads[0];
}

// Stops the motors for the drivetrain and shooter
void stopMotors() {
    roboclaw.ForwardM1(0X80, 0);
    roboclaw.ForwardM2(0X80, 0);
    roboclaw.BackwardM1(0X80, 0);
    roboclaw.BackwardM2(0X80, 0);

    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    digitalWrite(ENABLEA_PIN, LOW);
}

void controlShooter(int in1, int in2, bool enable) {
    digitalWrite(ENABLEA_PIN, enable);
    analogWrite(IN1, in1);
    analogWrite(IN2, in2);
}

void loop() {
    BP32.update();
    GamepadPtr gp  = myGamepads[0];
    if (!myGamepads[0]){
      reconnectGamepad();
    }
    
    bool buttonPressed = (gp->buttons() == 0x0002);
    if (buttonPressed && !lastButtonState) {
        killSwitch = !killSwitch;
        if (killSwitch){
          stopMotors();
        }
    }
    lastButtonState = buttonPressed;

    forwardReverse = 0;
    leftRight = 0;
    //DRIVETRAIN - ACCELERATION AND DEACCELERATION
    int throttleValue = gp->throttle();
    int brakeValue = gp->brake();
    
    acceleration = int((throttleValue*(0.05376)) - (brakeValue*(0.04887)) + 65);
    
    // For forward/reverse and left/right movement
    if (gp->axisRX() || gp->axisY() <= -90) {
      forwardReverse = map(-gp->axisRX(), 90, 512, min_speed, acceleration); //
      leftRight = -map(gp->axisRX(), 90, 511, min_speed, acceleration);
    }
    else if (gp->axisRX() || gp->axisY() >= -90){
      forwardReverse = -map(gp->axisY(), 90, 512, min_speed, acceleration);
      leftRight = map(gp->axisY(), 90, 511, min_speed, acceleration);
    }

    // For combining left/right and forward/reverse and sending that to the left and right motors
    leftMotorSpeed = constrain(forwardReverse + leftRight, -acceleration, acceleration);
    rightMotorSpeed = constrain(forwardReverse - leftRight, -acceleration, acceleration);
    
    if (leftMotorSpeed > 0) {
      roboclaw.ForwardM1(0X80, leftMotorSpeed);
    }
    else  {
      roboclaw.BackwardM1(0X80, -leftMotorSpeed);
    }
    
    if (rightMotorSpeed > 0) {
      roboclaw.ForwardM2(0X80, rightMotorSpeed);
    }
    else {
      roboclaw.BackwardM2(0X80, -rightMotorSpeed);
    }
    
    if (forwardReverse == 0 && leftRight == 0) {
      stopMotors();
    }

    //SHOOTER MECHANISM
    
    if (gp->buttons() == 0x0004){
      controlShooter(0, 140, true);
    }
    else if(gp->buttons() == 0x0001){
      controlShooter(100, 0, true);
    }
    else if (gp->buttons() == 0x0010) {
      controlShooter(220, 0, true);
    }
    else controlShooter(0, 0, false);
    
    delay(20);
}
