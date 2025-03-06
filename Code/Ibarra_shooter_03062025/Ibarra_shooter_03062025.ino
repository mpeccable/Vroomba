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
#define IN1 12
#define IN2 13
#define ENABLEA_PIN 14 

//ESP32 reset pin
#define ENPIN = 27

int motorSpeed = 0; // initially set the motor speed to 0

//Let's use a single deadspace variable? If they are all the same why we got duplicates? 
//forward_deadspace, reverse_deadspace, left_deadspace, and right_deadspace can all just
//be int deadspace = 90;
//Also, let's use a common max for forward_max, reverse_max, left_max, right_max?
//Each variable takes up program memory when it is calculated or called. Easiest thing to 
//Do is add some extra minus signs in our equations to reduce computation or recall time. 
// and reduce to int_max = 511; (and just not care about the 512 versus 511 for simplicity)


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
  
  // Check if this gamepad matches the allowlisted address
  if (myGamepads[0] == nullptr) {
    myGamepads[0] = gp;
  } 
  else {
    gp->disconnect(); // Reject additional controllers
  }
}

// Arduino setup function. Runs in CPU 1
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
  // Stops shooter movement 
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  digitalWrite(ENABLEA_PIN, LOW);

  delay(150);
  //reset the esp32 to try and reconnect
  digitalWrite(ENPIN, HIGH);
}


// Arduino loop function. Runs in CPU 1
void loop() {
  BP32.update();
  GamepadPtr myGamepad = myGamepads[0];
  
  //This kill switch code doesn't make a lot of sense to me... It seems like buttonPressed and 
  //lastButtonState are the same thing? 
  bool buttonPressed = (myGamepad->buttons() == 0x0002); // this is the B button
        
  if (buttonPressed && !lastButtonState) {  // Toggle the kill switch
    killSwitch = !killSwitch;
    if (killSwitch | !myGamepad->isConnected()) {
      // Stops drivetrain movement 
      roboclaw.ForwardM1(0X80, 0);
      roboclaw.ForwardM2(0X80, 0);
      roboclaw.BackwardM1(0X80, 0);
      roboclaw.BackwardM2(0X80, 0);
      // Stops shooter movement 
      analogWrite(IN1, 0);
      analogWrite(IN2, 0);
      digitalWrite(ENABLEA_PIN, LOW);
    }
  }

  lastButtonState = buttonPressed;

  // Shooter Mechanism              
  // Have it constantly spinning slowly for intake when X is pressed 
  if(myGamepad->buttons() == 0x0004){ //X button 
    digitalWrite(ENABLEA_PIN, HIGH);
    analogWrite(IN1, 0); 
    analogWrite(IN2, 140);
  }
                  
  // If the A is pressed --> outake occurs at a fast speed  
  else if(myGamepad->buttons() == 0x0001)  {  //A button 
    digitalWrite(ENABLEA_PIN, HIGH);
    analogWrite(IN2, 0);
    analogWrite(IN1, 100);
  }

  // If the left bumper is pressed --> outake occurs at a fast speed  
  else if(myGamepad->buttons() == 0x0010)  {  //LB  button 
    digitalWrite(ENABLEA_PIN, HIGH);
    analogWrite(IN2, 0);
    analogWrite(IN1, 220);
  }

  else {
    digitalWrite(ENABLEA_PIN, LOW);
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
  }


  int throttleValue = myGamepad->throttle();
  int brakeValue = myGamepad->brake();
  
  if (throttleValue >= brakeValue) {
    acceleration = map(throttleValue, 0, 1023, min_motor_speed, max_motor_speed);
  }
  else {
    acceleration = map(brakeValue, 0, 1023, min_motor_speed, 12);
  }

   
  // Forward and Reverse Drive (use RX instead of Y)
  int forwardReverse = 0;               
  if (myGamepad->axisRX() <= -forward_deadspace) {
    forwardReverse = map(abs(myGamepad->axisRX()), forward_deadspace, abs(forward_max), min_motor_speed, acceleration);
  } 
  else if (myGamepad->axisRX() >= reverse_deadspace) {
    forwardReverse = -map(abs(myGamepad->axisRX()), reverse_deadspace, reverse_max, min_motor_speed, acceleration);
  }


  // Left and Right Steering (use Y instead of RX)
  int leftRight = 0;
  if (myGamepad->axisY() <= -left_deadspace) {
    leftRight = -map(abs(myGamepad->axisY()), left_deadspace, abs(left_max), min_motor_speed, acceleration);
  } 
  else if (myGamepad->axisY() >= right_deadspace) {
    leftRight = map(abs(myGamepad->axisY()), right_deadspace, right_max, min_motor_speed, acceleration);
  }

  // Combine Forward/Reverse and Left/Right
  int leftMotorSpeed = constrain(forwardReverse + leftRight, -acceleration, acceleration);
  int rightMotorSpeed = constrain(forwardReverse - leftRight, -acceleration, acceleration);

  //Motor Speeds (given above combination of forward/reverse and left/right)
  if (leftMotorSpeed > 0) {
    roboclaw.ForwardM1(0X80, leftMotorSpeed);
  } 
  else {
    roboclaw.BackwardM1(0X80, abs(leftMotorSpeed));
    }

  if (rightMotorSpeed > 0) {
    roboclaw.ForwardM2(0X80, rightMotorSpeed);
  } 
  else {
    roboclaw.BackwardM2(0X80, abs(rightMotorSpeed));
  }

  // Stop motors if no input received 
  if (forwardReverse == 0 && leftRight == 0) {
    roboclaw.ForwardM1(0X80, 0);
    roboclaw.ForwardM2(0X80, 0);
    roboclaw.BackwardM1(0X80, 0);
    roboclaw.BackwardM2(0X80, 0);
  }
  delay(5);
}