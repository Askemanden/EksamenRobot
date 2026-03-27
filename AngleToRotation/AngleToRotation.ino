#include <Arduino.h>

const double DEGREES_TO_TURN = 90.0;
const double GEAR_RATIO = 102.083;

const uint8_t IN1 = 9;
const uint8_t IN2 = 10;
const uint8_t encoder1 = 3;
const uint8_t encoder2 = 2;

const long COUNTS_PER_REV = 16 * GEAR_RATIO;
const int  threshold      = 5;

const double Kp = 0.5;
const double Ki = 0.0;
const double Kd = 100.0;

const int MIN_SPEED = 40;
const int MAX_SPEED = 180;

volatile long current_position = 0;

void encoderISR() {
  if (digitalRead(encoder2) == HIGH) {
    current_position++;
  } else {
    current_position--;
  }
}

long   target_position = 0;
bool   finished = false;
double integral = 0;
long   last_error = 0;

void turn(bool cw, int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  if (cw) {
    analogWrite(IN1, speed);
    analogWrite(IN2, 0);
  } else {
    analogWrite(IN1, 0);
    analogWrite(IN2, speed);
  }
}

void stop_motor() {
  analogWrite(IN1, 255);
  analogWrite(IN2, 255);
  delay(30);
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  integral = 0;
  last_error = 0;
}

long degrees_to_counts(double degrees) {
  return (long)(degrees * COUNTS_PER_REV / 360.0);
}

void rotateDegrees(double degrees) {
  target_position = current_position + degrees_to_counts(degrees);
  finished = false;
  integral = 0;
  last_error = 0;
}

void dc_motor_update() {
  if (finished) return;

  long error = target_position - current_position;

  if (abs(error) <= threshold) {
    stop_motor();
    finished = true;
    return;
  }

  integral += error;
  integral  = constrain(integral, -1000, 1000);
  double derivative = error - last_error;
  last_error = error;

  double output = Kp * error + Ki * integral + Kd * derivative;

  int speed = (int)abs(output);
  turn(output > 0, speed);
}

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(encoder1, INPUT_PULLUP);
  pinMode(encoder2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoder1), encoderISR, RISING);

  stop_motor();
  delay(500);
  rotateDegrees(DEGREES_TO_TURN);
}

void loop() {
  dc_motor_update();
}