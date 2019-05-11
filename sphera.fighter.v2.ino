// muhack robotics workshop 2019 - Written by Claudio Tregambe aka c3g
// 

// use bluetooth serial adapter to receive commands from PC and robot control using keyboard
// Use these values to move (or change them if you like)
//        w   forward
//        a   left
//        d   reverse
//        x   right
//
// Speed control is also implemented 
//        -   speed down
//        +   speed up
//        /   max speed
//
// Every value not listed = stop

#include <NewPing.h>
#include <SoftwareSerial.h>

// L298 motor control variables
#define M1_A   12 // Arduino pin D12
#define M1_PWM 11 // Arduino pin D11
#define M1_B   10 // Arduino pin D10

#define M2_A   4  // Arduino pin D4
#define M2_PWM 3  // Arduino pin D3
#define M2_B   2  // Arduino pin D2

#define LED 13  // LED pin attached to Arduino D13

// Control pins for HC-SR04 attached to Arduino A1 (Echo) and A2 (Trigger)
#define TRIGGER_PIN  A2  // Arduino pin A2
#define ECHO_PIN     A1  // Arduino pin A1
#define MAX_DISTANCE 10  // sets maximum useable sensor measuring distance in cm

// Left and Right photoresistors pin
#define PHRES_RIGHT A0  // Arduino pin A0
#define PHRES_LEFT  A3  // Arduino pin A3

// NewPing setup of pins and maximum distance.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Some global variables of main use
int distance_value = 0; // the current distance of a ping signal
int incomingByte = 0;   // store serial data
int speed_val = 255;    // speed value
int ena_autonomous = 0; // enable or disable PING data evaluation
int ena_debug = 1;      // enable or disable DEBUG to the serial monitor

// Photoresistors setup for light and values
int phResLeft_default = 0;  // store the standard light value on the left (stored during startup)
int phResRight_default = 0; // store the standard light value on the right (stored during startup)
int phResLeft = 0;          // it contain the current light on the left to compare with the left light default
int phResRight = 0;         // it contain the current light on the right to compare with the right light default


////////// -------------------- SETUP -------------------- //////////
void setup(){

  TCCR2B = TCCR2B & 0b11111000 | 0x01; // change PWM frequency for pins 3 and 11 to 32kHz so there will be no motor whining

  // Start serial monitor at 9600 bps
  Serial.begin(9600);

  // declare outputs
  pinMode(LED, OUTPUT);

  pinMode(M1_A, OUTPUT);
  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_B, OUTPUT);

  pinMode(M2_A, OUTPUT);
  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_B, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PHRES_RIGHT, OUTPUT);
  pinMode(PHRES_LEFT, OUTPUT);

  phResRight_default = analogRead(PHRES_RIGHT);
  phResLeft_default = analogRead(PHRES_LEFT);
  

  // turn motors Off by default
  goStop();

  delay(500);

}

////////// -------------------- LOOP -------------------- //////////

void loop(){
  switch (ena_autonomous) {
  case 0:
    driveME();
    break;
  default:
    driveAuto();
    break;
  }
}

void driveME() {
  
  // check for serial data
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    // if available, blink LED and print serial data received.
    digitalWrite(LED, HIGH);
    // delay 10 milliseconds to allow serial update time
    delay(10);
  }
  switch (incomingByte) { // check incoming byte from serial

  case 119: // if byte is equal to "119" or "w", go forward
    goForward();
    break;
  case 97: // if byte is equal to "97" or "a", go left
    goLeft();
    break;
  case 100: // if byte is equal to "100" or "d", go right
    goRight();
    break;
  case 120: // if byte is equal to "120" or "x", go reverse
    goReverse();
    break;

  case 43: // if byte is equal to "43" or "+" - raise speed
    speed_val = speed_val + 5;
    if (speed_val > 250){
      speed_val = 255;
    }    
    break;
  case 45: // if byte is equal to "45" or "-" - lower speed
    speed_val = speed_val - 5;
    if (speed_val < 0){
      speed_val = 0;    // MIN
    }
    break;
  case 47: // if byte is equal to "47" or "/" - max speed
    speed_val = 255;
    break;

  default: // otherwise, stop both motors
    M1_stop();
    M2_stop();
    break;
}

  digitalWrite(LED, LOW);

  if (ena_debug == 1){
    //Serial.print("Serial received : ");
    //Serial.print(incomingByte);
    //Serial.print("    Speed received : ");
    //Serial.println(speed_val);
    phResLeft = analogRead(PHRES_LEFT);
    Serial.print(" phResLeft = ");
    Serial.print(phResLeft);
    phResRight = analogRead(PHRES_RIGHT);
    Serial.print(" phResRight = ");
    Serial.println(phResRight);
  }

}

void driveAuto() {
  distance_value = readPing();
  if (distance_value > MAX_DISTANCE) {
    goForward();
  }
  if (distance_value < MAX_DISTANCE) {
    goReverse();
    goLeft();
  }
  
}

// -------------------- END OF LOOP -------------------- //

/////////// HC-SR04 functions ////////////////

int readPing() { // read the ultrasonic sensor distance
  delay(70);
  unsigned int uS = sonar.ping();
  int cm = uS/US_ROUNDTRIP_CM;
  return cm;
}

/////////// motor functions ////////////////

void goStop (){ 
  M1_stop();
  M2_stop();
}

void goForward(){
  M1_forward(speed_val);
  M2_forward(speed_val);
}

void goReverse(){
  M1_reverse(speed_val);
  M2_reverse(speed_val);
}

void goLeft(){
    M1_reverse(speed_val);
    M2_forward(speed_val);
}

void goRight(){
    M1_forward(speed_val);
    M2_reverse(speed_val);
}

void M1_reverse(int x){
digitalWrite(M1_B, LOW);
digitalWrite(M1_A, HIGH);
analogWrite(M1_PWM, x);
}

void M1_forward(int x){
digitalWrite(M1_A, LOW);
digitalWrite(M1_B, HIGH);
analogWrite(M1_PWM, x);
}

void M1_stop(){
digitalWrite(M1_B, LOW);
digitalWrite(M1_A, LOW);
digitalWrite(M1_PWM, LOW);
}

void M2_forward(int y){
digitalWrite(M2_B, LOW);
digitalWrite(M2_A, HIGH);
analogWrite(M2_PWM, y);
}

void M2_reverse(int y){
digitalWrite(M2_A, LOW);
digitalWrite(M2_B, HIGH);
analogWrite(M2_PWM, y);
}

void M2_stop(){
digitalWrite(M2_B, LOW);
digitalWrite(M2_A, LOW);
digitalWrite(M2_PWM, LOW);
}


