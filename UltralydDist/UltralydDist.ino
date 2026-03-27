<<<<<<< HEAD
  const int trans_pin = 5; //Trig
const int recv_pin = 7; //Echo
=======
/**
 * @file UltralydDist.ino
 * @brief Ultrasonic distance measurement using trigger/echo pins.
 */
>>>>>>> c3166c707212d895821d9c4a88f1a387fcdaf003

/**
 * @brief Pin used to trigger the ultrasonic transmitter.
 */
const int trans_pin = 5;

/**
 * @brief Pin used to read the ultrasonic echo signal.
 */
const int recv_pin = 7;

/**
 * @brief Arduino setup function.
 *
 * Initializes serial communication and configures the ultrasonic
 * transmitter and receiver pins.
 */
void setup() {
    Serial.begin(9600);
    pinMode(trans_pin, OUTPUT);
    pinMode(recv_pin, INPUT);
}

/**
 * @brief Measures distance using an ultrasonic sensor.
 *
 * Sends a short trigger pulse and measures the duration of the
 * returning echo pulse. The duration is converted to distance
 * in centimeters using the formula:
 *
 * distance_cm = duration / 58
 *
 * @return Distance to the nearest object in centimeters.
 */
double getDistanceCM() {
    long duration;

    digitalWrite(trans_pin, LOW);
    delayMicroseconds(5);

    digitalWrite(trans_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trans_pin, LOW);

    duration = pulseIn(recv_pin, HIGH);

    double distance_cm = duration * 0.0172;

    return distance_cm;
}
<<<<<<< HEAD

void loop() {
  double distance = getDistanceCM();
  if (distance > 0 && distance < 400) {  // Typical HC-SR04 range
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Out of range");
  }
  
  delay(200);
}
=======
>>>>>>> c3166c707212d895821d9c4a88f1a387fcdaf003
