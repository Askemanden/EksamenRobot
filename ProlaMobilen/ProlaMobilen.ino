#include <Arduino.h>

struct Motor{
  volatile long current_position;
  long target_position;
  long counts_per_rev;
  long last_error;
  double gear_ratio;
  double Kp;
  double Ki;
  double Kd;
  double integral;
  double angle;
  int min_speed;
  int max_speed;
  uint8_t in1;
  uint8_t in2;
  uint8_t encoder;
  bool finished;
}

const double DEGREES_TO_TURN = 90.0;
const double GEAR_RATIO = 30.0;

const uint8_t IN1 = 9;
const uint8_t IN2 = 10;
const uint8_t encoder1 = 3;
const uint8_t encoder2 = 2;

const long COUNTS_PER_REV = 16 * GEAR_RATIO;
const int  threshold      = 5;

const double Kp = 0.5;
const double Ki = 0.2;
const double Kd = 5.0;

const int MIN_SPEED = 40;
const int MAX_SPEED = 180;

volatile long current_position = 0;

/*Wrap in argumentless wrapper with pointer to motor*/
void encoderISR(Motor motor) {
  if (digitalRead(motor.encoder2) == HIGH) {
    current_position++;
  } else {
    current_position--;
  }
}

long   target_position = 0;
bool   finished = false;
double integral = 0;
long   last_error = 0;

void turn(Motor motor, bool cw, int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  if (cw) {
    analogWrite(motor.IN1, speed);
    analogWrite(motor.IN2, 0);
  } else {
    analogWrite(motor.IN1, 0);
    analogWrite(motor.IN2, speed);
  }
}

void stop_motor(Motor motor) {
  analogWrite(motor.IN1, 255);
  analogWrite(motor.IN2, 255);
  delay(30);
  analogWrite(motor.IN1, 0);
  analogWrite(motor.IN2, 0);
  motor.integral = 0;
  motor.last_error = 0;
}

long degrees_to_counts(Motor motor, double degrees) {
  return (long)(degrees * motor.COUNTS_PER_REV / 360.0);
}

void rotateDegrees(Motor motor, double degrees) {
  motor.target_position = motor.current_position + degrees_to_counts(degrees);
  motor.finished = false;
  motor.integral = 0;
  motor.last_error = 0;
}

void dc_motor_update(Motor motor) {
  if (motor.finished) return;

  long error = motor.target_position - motor.current_position;

  if (abs(error) <= threshold) {
    stop_motor();
    motor.finished = true;
    return;
  }

  motor.integral += error;
  motor.integral  = constrain(motor.integral, -1000, 1000);
  double derivative = error - motor.last_error;
  motor.last_error = error;

  double output = motor.Kp * error + motor.Ki * motor.integral + motor.Kd * derivative;

  int speed = (int)abs(output);
  turn(output > 0, speed);
}

void post_init_motor(Motor *motor, void(ISR*)()){
  pinMode(motor.IN1, OUTPUT);
  pinMode(motor.IN2, OUTPUT);
  pinMode(motor.encoder1, INPUT_PULLUP);
  pinMode(motor.encoder2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(motor.encoder1), ISR, RISING);

  stop_motor();
}

void setup() {
  Serial.begin(9600);
  

  delay(500);
  rotateDegrees(DEGREES_TO_TURN);
}

void loop() {
  dc_motor_update();
}