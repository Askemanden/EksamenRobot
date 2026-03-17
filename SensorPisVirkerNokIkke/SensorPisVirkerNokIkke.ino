#include <Servo.h>

struct UltrasonicSensor {
  int trigPin;
  int echoPin;
};

struct ServoScanner {
  Servo servo;
  int pin;
  int minAngle;
  int maxAngle;
  int stepSize;
};

struct ScanResult {
  int firstAngle;
  int lastAngle;
  int trueAngle;
  bool detected;
};



UltrasonicSensor sonar = {
  .trigPin = 9,
  .echoPin = 8
};

ServoScanner scanner = {
  .servo = Servo(),
  .pin = 6,
  .minAngle = 0,
  .maxAngle = 180,
  .stepSize = 2
};

float detectionThresholdcm = 200;



float measureDistance(const UltrasonicSensor &s) {
  digitalWrite(s.trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(s.trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(s.trigPin, LOW);

  long duration = pulseIn(s.echoPin, HIGH, 30000);
  if (duration == 0) return -1;

  return duration / 58.0;
}



ScanResult performScan(const UltrasonicSensor &s, ServoScanner &sc) {
  ScanResult result = {
    .firstAngle = -1,
    .lastAngle = -1,
    .trueAngle = -1,
    .detected = false
  };

  for (int angle = sc.minAngle; angle <= sc.maxAngle; angle += sc.stepSize) {
    sc.servo.write(angle);
    delay(40);

    float dist = measureDistance(s);

    if (dist > 0 && dist < detectionThresholdcm) {
      if (result.firstAngle == -1) result.firstAngle = angle;
      result.lastAngle = angle;
      result.detected = true;
    }
  }

  if (result.detected) {
    result.trueAngle = (result.firstAngle + result.lastAngle) / 2;
  }

  return result;
}


void printScanResult(const ScanResult &r) {
  if (!r.detected) {
    Serial.println("No object detected.");
    return;
  }

  Serial.print("Object detected between ");
  Serial.print(r.firstAngle);
  Serial.print("° and ");
  Serial.print(r.lastAngle);
  Serial.print("°   ");

  Serial.print("Estimated true angle: ");
  Serial.print(r.trueAngle);
  Serial.println("°");
}



void setup() {
  Serial.begin(9600);

  pinMode(sonar.trigPin, OUTPUT);
  pinMode(sonar.echoPin, INPUT);

  scanner.servo.attach(scanner.pin);

  Serial.println("ready");
}


void loop() {
  ScanResult r = performScan(sonar, scanner);
  printScanResult(r);
  delay(300);
}
