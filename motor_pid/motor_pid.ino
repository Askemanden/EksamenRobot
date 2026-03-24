/**
 * @file motor_pid.ino
 * @brief PID-styret DC-motor med encoder-feedback på Arduino.
 *
 * Roterer en DC-motor et bestemt antal grader ved hjælp af en
 * PID-regulator og en quadrature-encoder til positionsfeedback.
 */

#include <Arduino.h>

/// Ønsket rotationsvinkel i grader
const double DEGREES_TO_TURN = 90.0;

/// Gearratio mellem motor og aksel
const double GEAR_RATIO = 102.083;

/// PWM-pin til motorretning A (H-bro)
const uint8_t IN1 = 9;

/// PWM-pin til motorretning B (H-bro)
const uint8_t IN2 = 10;

/// Encoder kanal A – tilknyttet hardware-interrupt (RISING)
const uint8_t encoder1 = 3;

/// Encoder kanal B – bruges til retningsbestemmelse i ISR
const uint8_t encoder2 = 2;

/// Antal encoder-pulser pr. omdrejning på akslen (16 pulser/omdr. × gearratio)
const long COUNTS_PER_REV = 16 * GEAR_RATIO;

/// Dødzone i encoder-counts – inden for denne margin betragtes positionen som nået
const int threshold = 5;

/// PID proportionalforstærkning – reagerer på nuværende fejl
const double Kp = 0.5;

/// PID integralforstærkning – kompenserer for vedvarende offset (deaktiveret)
const double Ki = 0.0;

/// PID differentialforstærkning – dæmper svingninger ved hurtige fejlændringer
const double Kd = 100.0;

/// Minimum PWM-værdi – under denne starter motoren ikke pålideligt
const int MIN_SPEED = 40;

/// Maximum PWM-værdi – begrænser motorens topfart
const int MAX_SPEED = 180;

/**
 * @brief Aktuel motorposition i encoder-counts.
 * @note volatile er påkrævet da variablen opdateres asynkront af en ISR.
 */
volatile long current_position = 0;

/// Ønsket målposition i encoder-counts
long target_position = 0;

/// true når motoren har nået målpositionen
bool finished = false;

/// Akkumuleret fejl til PID I-led
double integral = 0;

/// Fejl fra forrige iteration til PID D-led
long last_error = 0;

/**
 * @brief Interrupt Service Routine – opdaterer encoder-positionen.
 *
 * Kaldes automatisk ved RISING edge på encoder1 (kanal A).
 * Retningen bestemmes ved at aflæse kanal B:
 * - Kanal B HIGH → med uret  → tæl op
 * - Kanal B LOW  → mod uret  → tæl ned
 */
void encoderISR() {
  if (digitalRead(encoder2) == HIGH) {
    current_position++;
  } else {
    current_position--;
  }
}

/**
 * @brief Sætter motorretning og hastighed via H-broen.
 *
 * @param cw    true = med uret, false = mod uret
 * @param speed PWM-værdi; begrænses automatisk til [MIN_SPEED, MAX_SPEED]
 */
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

/**
 * @brief Bremser motoren og nulstiller PID-tilstand.
 *
 * Udfører elektrisk bremsning ved at sætte begge H-bro-pins til 255 i 30 ms,
 * derefter slukkes begge pins. integral og last_error nulstilles så PID-
 * regulatoren starter frisk ved næste bevægelse.
 */
void stop_motor() {
  analogWrite(IN1, 255);
  analogWrite(IN2, 255);
  delay(30);
  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  integral   = 0;
  last_error = 0;
}

/**
 * @brief Omregner grader til encoder-counts.
 *
 * @param degrees Vinkel i grader
 * @return Tilsvarende antal encoder-counts (long)
 */
long degrees_to_counts(double degrees) {
  return (long)(degrees * COUNTS_PER_REV / 360.0);
}

/**
 * @brief Sætter en ny målposition og starter PID-regulatoren.
 *
 * Målpositionen beregnes relativt til den aktuelle position,
 * så funktionen kan kaldes sekventielt til flere bevægelser.
 *
 * @param degrees Ønsket rotation i grader (positiv = med uret)
 */
void rotateDegrees(double degrees) {
  target_position = current_position + degrees_to_counts(degrees);
  finished   = false;
  integral   = 0;
  last_error = 0;
}

/**
 * @brief PID-regulator – skal kaldes gentagne gange fra loop().
 *
 * Beregner motorkommando ud fra positionen:
 * -# Returnér tidligt hvis målet allerede er nået
 * -# Beregn fejl = target − nuværende position
 * -# Stop hvis fejlen er inden for threshold (dødzone)
 * -# Opdater integral med anti-windup (clamp ±1000)
 * -# Beregn differentialterm
 * -# Beregn PID-output og send til motor via turn()
 */
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

/**
 * @brief Initialiserer hardware og starter rotationen.
 *
 * Konfigurerer pins, tilknytter encoder-interrupt,
 * bremser motoren ved opstart og igangsætter den ønskede rotation.
 */
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

/**
 * @brief Hovedløkke – kalder PID-regulatoren kontinuerligt.
 */
void loop() {
  dc_motor_update();
}
