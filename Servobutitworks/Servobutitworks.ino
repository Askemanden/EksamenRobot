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
  int edgeBefore;
  int edgeAfter;
  int trueAngle;
  float objectWidth;
  float objectDistance;
  bool detected;
  int resetAngle;
};

UltrasonicSensor sonar = {9, 10};

ServoScanner scanner = {
  Servo(),
  11,
  0,
  90,
  1
};

float detectionThresholdcm = 25;
float distanceOffsetcm = 0.6;
const int EDGE_EXTENSION = 90;

float measureDistance(const UltrasonicSensor &s) {
  digitalWrite(s.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(s.trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(s.trigPin, LOW);

  long duration = pulseIn(s.echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return (duration / 58.0) + distanceOffsetcm;
}

float averageDistance(const UltrasonicSensor &s, int samples) {
  float sum = 0;
  int valid = 0;
  for (int i = 0; i < samples; i++) {
    float d = measureDistance(s);
    if (d > 0) { sum += d; valid++; }
    delayMicroseconds(500);
  }
  if (valid == 0) return -1;
  return sum / valid;
}

ScanResult performScan(const UltrasonicSensor &s, ServoScanner &sc, int startAngle, int endAngle) {
  ScanResult result = {-1, -1, -1, -1, -1, false, -1};

  int step = (endAngle >= startAngle) ? sc.stepSize : -sc.stepSize;
  int prevAngle = startAngle;
  bool wasDetected = false;
  float distanceSum = 0;
  int distanceCount = 0;

  int absoluteMin = sc.minAngle - EDGE_EXTENSION;
  int absoluteMax = sc.maxAngle + EDGE_EXTENSION;
  bool inExtension = false;

  int dynamicEnd = endAngle;

  for (int angle = startAngle;
       (step > 0) ? (angle <= dynamicEnd) : (angle >= dynamicEnd);
       angle += step)
  {
    int servoAngle = constrain(angle, absoluteMin, absoluteMax);
    sc.servo.write(servoAngle);
    delay(30);

    float dist = averageDistance(s, 3);
    bool isDetected = (dist > 0 && dist <= detectionThresholdcm);

    if (!wasDetected && isDetected) {
      result.edgeBefore = prevAngle;
      result.detected = true;
    }

    if (isDetected) {
      distanceSum += dist;
      distanceCount++;
    }

    if (wasDetected && !isDetected) {
      result.edgeAfter = angle;
      break;
    }

    if (isDetected) {
      bool atNormalEnd = (step > 0) ? (angle >= endAngle) : (angle <= endAngle);
      if (atNormalEnd && !inExtension) {
        inExtension = true;
        dynamicEnd = endAngle + (step * EDGE_EXTENSION);
        dynamicEnd = constrain(dynamicEnd, absoluteMin, absoluteMax);
      }
    }

    wasDetected = isDetected;
    prevAngle = angle;
  }

  if (result.detected && result.edgeAfter == -1) {
    result.edgeAfter = dynamicEnd;
  }

  if (result.detected) {
    result.trueAngle = (result.edgeBefore + result.edgeAfter) / 2;
    if (distanceCount > 0) result.objectDistance = distanceSum / distanceCount;

    int midInterval = (sc.minAngle + sc.maxAngle) / 2;
    if (result.edgeBefore < midInterval) {
      result.resetAngle = sc.maxAngle;
    } else {
      result.resetAngle = sc.minAngle;
    }
  }

  return result;
}

// MAIN CALLABLE FUNCTION
int scanForTargetAngle(UltrasonicSensor &s, ServoScanner &sc) {
  ScanResult r1 = performScan(s, sc, sc.minAngle, sc.maxAngle);
  if (r1.detected) return r1.trueAngle;

  ScanResult r2 = performScan(s, sc, sc.maxAngle, sc.minAngle);
  if (r2.detected) return r2.trueAngle;

  return -1;
}

void initScanner(){
  pinMode(sonar.trigPin, OUTPUT);
  pinMode(sonar.echoPin, INPUT);
  scanner.servo.attach(scanner.pin);
}

void setup(){
  initScanner();
  Serial.begin(9600);
}

void loop() {
  int angle = scanForTargetAngle(sonar, scanner);

  if (angle != -1) {
    Serial.print("Detected angle: ");
    Serial.print(angle);
    Serial.println("°");

    // point at target
    scanner.servo.write(angle);
    delay(800);

    // return to center
    int center = (scanner.minAngle + scanner.maxAngle) / 2;
    scanner.servo.write(center);
    delay(500);
  } else {
    Serial.println("No object detected");
    delay(300);
  }
}