const int trans_pin = 5; //Trig
const int recv_pin = 7; //Echo

void setup() {

Serial.begin(9600);
pinMode(trans_pin,OUTPUT); //transmit is ouput
pinMode(recv_pin,INPUT); //receive is input
}

double getDistanceCM() {
  long duration;

  // Ensure clean LOW pulse
  digitalWrite(trans_pin, LOW);
  delayMicroseconds(5);

  // Send 10µs HIGH pulse
  digitalWrite(trans_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trans_pin, LOW);

  // Measure echo time
  duration = pulseIn(recv_pin, HIGH);

  // Convert to centimeters
  double distance_cm = duration / 58.0;

  return distance_cm;
}

