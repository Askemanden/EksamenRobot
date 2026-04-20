Askemanden
4skemanden
Invisible

nigganigga [LOTM],  — 9:07 AM
#include <Arduino.h>

const double DEGREES_TO_TURN = 90.0;
const double GEAR_RATIO = 30.0;

const uint8_t IN1 = 9;

ProlaMobilen.ino
3 KB
her
din neger
nigganigga [LOTM],  — 10:10 AM
hej aske
Askemanden — 10:10 AM
hvad?
nigganigga [LOTM],  — 10:10 AM
nigga
louis er så ubrugelig
jeg flytter mig tilbage til jer
Askemanden — 11:00 AM
okay
nigganigga [LOTM],  — 1:42 PM
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
  int resetAngle; // uh uh ah ah
};

UltrasonicSensor sonar = {
  .trigPin = 9,
  .echoPin = 10
};

ServoScanner scanner = {
  .servo = Servo(),
  .pin = 11,
  .minAngle = 0,
  .maxAngle = 90,
  .stepSize = 1
};

float detectionThresholdcm = 25;
float distanceOffsetcm = 0.6;
const int EDGE_EXTENSION = 30; // Ekstra grader uden for intervallet

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
  ScanResult result = {
    .edgeBefore = -1,
    .edgeAfter = -1,
    .trueAngle = -1,
    .objectWidth = -1,
    .objectDistance = -1,
    .detected = false
  };

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

    Serial.print("Angle: ");
... (135 lines left)

Servobutitworks.ino
7 KB
﻿
salute
nigga
nigga
bigladfike
🐒/🥷
 
 
 
 
 
🔥 #1 LOTM/COI glazer 🔥
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
  int resetAngle; // uh uh ah ah
};

UltrasonicSensor sonar = {
  .trigPin = 9,
  .echoPin = 10
};

ServoScanner scanner = {
  .servo = Servo(),
  .pin = 11,
  .minAngle = 0,
  .maxAngle = 90,
  .stepSize = 1
};

float detectionThresholdcm = 25;
float distanceOffsetcm = 0.6;
const int EDGE_EXTENSION = 30; // Ekstra grader uden for intervallet

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
  ScanResult result = {
    .edgeBefore = -1,
    .edgeAfter = -1,
    .trueAngle = -1,
    .objectWidth = -1,
    .objectDistance = -1,
    .detected = false
  };

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

    Serial.print("Angle: ");
    Serial.print(angle);
    if (inExtension) Serial.print(" [EXT]");
    Serial.print(" | Dist: ");
    if (dist < 0) Serial.println("timeout");
    else { Serial.print(dist); Serial.println(" cm"); }

    if (!wasDetected && isDetected) {
      result.edgeBefore = prevAngle;
      result.detected = true;
      Serial.print("Object entered, edge before: ");
      Serial.print(prevAngle);
      Serial.println("°");
    }

    if (isDetected) {
      distanceSum += dist;
      distanceCount++;
    }

    if (wasDetected && !isDetected) {
      result.edgeAfter = angle;
      Serial.print("Object lost, edge after: ");
      Serial.print(angle);
      Serial.println("°");
      if (inExtension) {
        inExtension = false;
        Serial.println("Extension complete — edge found, resetting.");
      }
      break;
    }

    if (isDetected) {
      bool atNormalEnd = (step > 0) ? (angle >= endAngle) : (angle <= endAngle);
      if (atNormalEnd && !inExtension) {
        inExtension = true;
        dynamicEnd = endAngle + (step * EDGE_EXTENSION);
        dynamicEnd = constrain(dynamicEnd, absoluteMin, absoluteMax);
        Serial.print("Object at boundary — extending scan to: ");
        Serial.print(dynamicEnd);
        Serial.println("°");
      }
    }

    wasDetected = isDetected;
    prevAngle = angle;
  }

  if (result.detected && result.edgeAfter == -1) {
    result.edgeAfter = dynamicEnd;
    Serial.print("Object still detected at scan end, using: ");
    Serial.print(dynamicEnd);
    Serial.println("°");
  }

  if (result.detected) {
    result.trueAngle = (result.edgeBefore + result.edgeAfter) / 2;
    if (distanceCount > 0) result.objectDistance = distanceSum / distanceCount;

    // Beregn reset-position: modsatte side af intervallet fra edgeBefore
    // Hvis edgeBefore er tæt på minAngle, reset til maxAngle og omvendt
    int midInterval = (sc.minAngle + sc.maxAngle) / 2;
    if (result.edgeBefore < midInterval) {
      result.resetAngle = sc.maxAngle; // Objekt fundet tæt på min → reset til max
    } else {
      result.resetAngle = sc.minAngle; // Objekt fundet tæt på max → reset til min
    }
  }

  return result;
}

void printScanResult(const ScanResult &r) {
  if (!r.detected) {
    Serial.println("No object detected.");
    return;
  }
  Serial.print("Edge before: "); Serial.print(r.edgeBefore); Serial.print("°");
  Serial.print("  |  Edge after: "); Serial.print(r.edgeAfter); Serial.print("°");
  Serial.print("  |  True angle: "); Serial.print(r.trueAngle); Serial.print("°");
  Serial.print("  |  Distance: ");
  if (r.objectDistance > 0) { Serial.print(r.objectDistance); Serial.print(" cm"); }
  else Serial.print("N/A");
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  pinMode(sonar.trigPin, OUTPUT);
  pinMode(sonar.echoPin, INPUT);
  scanner.servo.attach(scanner.pin);
  Serial.println("ready");
}

bool hasTarget = false;
int targetAngle = 45;

void loop() {
  Serial.println("--- Scanning forward ---");
  ScanResult r = performScan(sonar, scanner, scanner.minAngle, scanner.maxAngle);
  printScanResult(r);

  if (r.detected) {
    targetAngle = r.trueAngle;
    hasTarget = true;
    Serial.print("Pointing to: "); Serial.print(targetAngle); Serial.println("°");
    scanner.servo.write(targetAngle);
    delay(500);

    Serial.print("Resetting to: "); Serial.print(r.resetAngle); Serial.println("°");
    scanner.servo.write(r.resetAngle);
    delay(400);
  } else {
    hasTarget = false;
  }

  Serial.println("--- Scanning backward ---");
  ScanResult r2 = performScan(sonar, scanner, scanner.maxAngle, scanner.minAngle);
  printScanResult(r2);

  if (r2.detected) {
    targetAngle = r2.trueAngle;
    hasTarget = true;
    Serial.print("Pointing to: "); Serial.print(targetAngle); Serial.println("°");
    scanner.servo.write(targetAngle);
    delay(500);

    Serial.print("Resetting to: "); Serial.print(r2.resetAngle); Serial.println("°");
    scanner.servo.write(r2.resetAngle);
    delay(400);
  } else {
    hasTarget = false;
  }

  delay(300);
}