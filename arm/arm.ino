double #include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Servo.h>

//  ██████╗ ██████╗ ██╗      ██████╗ ██████╗      ███████╗███████╗███╗   ██╗███████╗ ██████╗ ██████╗  ██████╗
// ██╔════╝██╔═══██╗██║     ██╔═══██╗██╔══██╗     ██╔════╝██╔════╝████╗  ██║██╔════╝██╔═══██╗██╔══██╗██╔═══██╗
// ██║     ██║   ██║██║     ██║   ██║██████╔╝     █████╗  █████╗  ██╔██╗ ██║█████╗  ██║   ██║██████╔╝██║   ██║
// ██║     ██║   ██║██║     ██║   ██║██╔══██╗     ██╔══╝  ██╔══╝  ██║╚██╗██║██╔══╝  ██║   ██║██╔══██╗██║   ██║
// ╚██████╗╚██████╔╝███████╗╚██████╔╝██║  ██║     ██║     ███████╗██║ ╚████║██║     ╚██████╔╝██║  ██║╚██████╔╝
//  ╚═════╝ ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝  ╚═╝     ╚═╝     ╚══════╝╚═╝  ╚═══╝╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚═════╝

/**!
 * \addtogroup ColorSensor
 * @{
 */
namespace ColorSensor
{
  /**
   * @brief Enumeration of detected color categories.
   */
  enum DetectedColor
  {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_UNCERTAIN
  };

  /**
   * @brief TCS34725 sensor object with standard integration time and gain.
   */
  Adafruit_TCS34725 tcs = Adafruit_TCS34725(
      TCS34725_INTEGRATIONTIME_50MS,
      TCS34725_GAIN_4X);

  /**
   * @brief Initializes serial communication and the TCS34725 sensor.
   *
   * This function must be called before any color reading is attempted.
   * If the sensor is not detected, the program halts.
   */
  void initColorSensor()
  {
    delay(200);

    if (!tcs.begin())
    {
      Serial.println("ERROR: TCS34725 not detected");
      while (1)
        ;
    }

    Serial.println("TCS34725 initialized");
  }

  /**
   * @brief Reads raw color values from the TCS34725 sensor.
   *
   * @param r Reference to store the red channel value.
   * @param g Reference to store the green channel value.
   * @param b Reference to store the blue channel value.
   * @param c Reference to store the clear (overall brightness) value.
   */
  void readColorRaw(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &c)
  {
    tcs.getRawData(&r, &g, &b, &c);
  }

  /**
   * @brief Classifies a color using thresholds and ratios to avoid false positives.
   *
   * @param r Raw red value.
   * @param g Raw green value.
   * @param b Raw blue value.
   * @param c Clear channel (brightness).
   * @return DetectedColor Enum representing the detected color.
   */
  DetectedColor categorizeColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c)
  {

    if (c < 200)
    {
      return COLOR_BLACK;
    }
    if (c > 4000)
    {
      return COLOR_WHITE;
    }

    float rn = (float)r;
    float gn = (float)g;
    float bn = (float)b;

    const float minDiff = 0.03;
    const float minRatio = 1.025;

    if (rn > gn * minRatio && rn > bn * minRatio && (rn - gn) > minDiff && (rn - bn) > minDiff)
    {
      return COLOR_RED;
    }

    if (gn > rn * minRatio && gn > bn * minRatio && (gn - rn) > minDiff && (gn - bn) > minDiff)
    {
      return COLOR_GREEN;
    }

    if (bn > rn * minRatio && bn > gn * minRatio && (bn - rn) > minDiff && (bn - gn) > minDiff)
    {
      return COLOR_BLUE;
    }

    return COLOR_UNCERTAIN;
  }

  /**
   * @brief Converts a DetectedColor enum value to a human-readable string.
   *
   * @param color The enum value to convert.
   * @return const char* String representation of the color.
   */
  const char *colorToString(DetectedColor color)
  {
    switch (color)
    {
    case COLOR_RED:
      return "RED";
    case COLOR_GREEN:
      return "GREEN";
    case COLOR_BLUE:
      return "BLUE";
    case COLOR_BLACK:
      return "BLACK";
    case COLOR_WHITE:
      return "WHITE";
    default:
      return "UNCERTAIN";
    }
  }

  /**
   * @brief Reads the sensor, categorizes the color, and prints the result.
   */
  void detectAndPrintColor()
  {
    uint16_t r, g, b, c;
    readColorRaw(r, g, b, c);

    DetectedColor color = categorizeColor(r, g, b, c);

    Serial.print("R: ");
    Serial.print(r);
    Serial.print("  G: ");
    Serial.print(g);
    Serial.print("  B: ");
    Serial.print(b);
    Serial.print("  C: ");
    Serial.print(c);
    Serial.print("  -> Detected: ");
    Serial.println(colorToString(color));
  }
}
/*! @} */

// ██╗██╗  ██╗
// ██║██║ ██╔╝
// ██║█████╔╝
// ██║██╔═██╗
// ██║██║  ██╗
// ╚═╝╚═╝  ╚═╝

/*!
 * \addtogroup IK
 * @{
 */
namespace IK
{
  /**
   * @brief $L_1$
   */
  const double L1 = 10;

  /**
   * @brief $L_2$
   */
  const double L2 = 7;

  /**
   * @brief $L_3$
   */
  const double L3 = 4;

  const double STEP_DIST = 1;

  const double GRAB_HEIGHT = 3;

  /**
   * @struct Vector2
   * @brief A 2d vector in the uv plane of the arm.
   * @var Vector2::u U-coordinate (x-equivalent) of the vector.
   * @var Vector2::v v-coordinate (y-equivalent) of the vector.
   */
  struct Vector2
  {
    double u;
    double v;
  };

  /**
   * @struct Arm
   * @brief Contains angles of the arm. All angles are in radians.
   * @var Arm::theta1 Angle at the bottom of the arm. $\theta_1$.
   * @var Arm::theta2 Elbow angle of the arm. $\theta_2$.
   * @var Arm::theta3 Wrist angle of the arm. Should always result in 90 deg negative angle with x-axist at the point of rotation. $\theta_3$.
   * @var Arm::base_angle Arm angle with the x-axis. Rotates the UV plane around Z. $\theta_O$
   * @var Arm::final_targetUV Last target of the Arm from which to calculate the next target in the path of the hand.
   * @var Arm::targetUV Last target of the Arm from which to calculate the next target in the path of the hand.
   */
  struct Arm
  {
    Vector2 final_targetUV;
    Vector2 targetUV;
    double theta1;
    double theta2;
    double theta3;
    double base_angle;
  };

  Vector2 vectorAdd(Vector2 self, Vector2 other)
  {
    return {.u = self.u + other.u, .v = self.v + other.v};
  }

  /**
   * @brief Calculates base_angle and writes it to self and calculates final_targetUV
   * @param self The arm for which to calculate data.
   * @param sensor_position_distance Distance of sensor from origin. $d_s$
   * @param sensor_reading_distance The reading of the sensor in centimeters. $L_s$
   * @param sensor_angle_rad Angle in the sensors servo. $\theta_s$
   */
  int calculateFinalTarget(Arm *self, double sensor_position_distance, double sensor_reading_distance, double sensor_angle_rad)
  {
    /*$d_s*/
    double target_distance = sqrt(sq(sensor_position_distance) + sq(sensor_reading_distance) - 2 * sensor_position_distance * sensor_reading_distance * cos(sensor_angle_rad));
    double sin_theta_O = sin(sensor_angle_rad) * sensor_reading_distance / target_distance;
    double cos_theta_O = (sq(target_distance) + sq(sensor_position_distance) - sq(sensor_reading_distance)) / (2 * target_distance * sensor_position_distance);
    self->base_angle = atan2(sin_theta_O, cos_theta_O);
    self->final_targetUV = {.u = target_distance, .v = 0};
    return 0;
  }

  /**
   * @brief Finds the next UV coordinate in the path of self and writes to self on targetUV.
   * @param self The arm to which to write
   */
  int calculateNextTargetUV(Arm *self)
  {
    Vector2 final_targetUV = self->final_targetUV;

    Vector2 previous_target = self->targetUV;

    double safe_v = final_targetUV.v + GRAB_HEIGHT;

    if (abs(previous_target.u - final_targetUV.u) < 0.01)
    {
      self->targetUV = vectorAdd(previous_target, {.u = 0, .v = -STEP_DIST});
    }

    else if (previous_target.v < safe_v)
    {
      self->targetUV = vectorAdd(previous_target, {.u = 0, .v = STEP_DIST});
    }

    else if (previous_target.u < final_targetUV.u)
    {
      self->targetUV = vectorAdd(previous_target, {.u = STEP_DIST, .v = 0});
    }
    else
    {
      self->targetUV = vectorAdd(previous_target, {.u = -STEP_DIST, .v = 0});
    }

    if (abs(self->targetUV.u - final_targetUV.u) < STEP_DIST)
    {
      self->targetUV.u = final_targetUV.u;
    }
    if (abs(self->targetUV.v - final_targetUV.v) < STEP_DIST)
    {
      self->targetUV.v = final_targetUV.v;
    }

    return 0;
  }

  /**
   * @brief Calculates the angles ($\theta_1 \, \theta_2 \, \theta_3$) of self and write to self.
   * @param self The arm to which to write.
   */
  int calculateArmAngles(Arm *self)
  {
    Vector2 final_targetUV = self->final_targetUV;
    Vector2 target = self->targetUV;
    self->theta2 = -acos((sq(target.u) + sq(target.v) - sq(L1) - sq(L2)) / (2 * L1 * L2));
    double k1 = L1 + L2 * cos(self->theta2);
    double k2 = L2 * sin(self->theta2);
    self->theta1 = atan2(target.v, target.u) - atan2(k2, k1);
    self->theta3 = 2 * PI - self->theta1 - self->theta2 - PI / 2;
    return 0;
  }

}
/*! @} */

// ███╗   ███╗ ██████╗ ████████╗ ██████╗ ██████╗
// ████╗ ████║██╔═══██╗╚══██╔══╝██╔═══██╗██╔══██╗
// ██╔████╔██║██║   ██║   ██║   ██║   ██║██████╔╝
// ██║╚██╔╝██║██║   ██║   ██║   ██║   ██║██╔══██╗
// ██║ ╚═╝ ██║╚██████╔╝   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝

/*!
 * \addtogroup Motor
 * @{
 */
namespace Motor
{

  void turnAll(Arm arm);
}
/*! @} */


  //  ███████╗ ██████╗ █████╗ ███╗   ██╗███╗   ██╗███████╗██████╗ 
  //  ██╔════╝██╔════╝██╔══██╗████╗  ██║████╗  ██║██╔════╝██╔══██╗
  //  ███████╗██║     ███████║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝
  //  ╚════██║██║     ██╔══██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗
  //  ███████║╚██████╗██║  ██║██║ ╚████║██║ ╚████║███████╗██║  ██║
  //  ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝

  //       ███████╗ ██████╗ █████╗ ███╗   ██╗███╗   ██╗███████╗██████╗ 
  //       ██╔════╝██╔════╝██╔══██╗████╗  ██║████╗  ██║██╔════╝██╔══██╗
  //       ███████╗██║     ███████║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝
  //       ╚════██║██║     ██╔══██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗
  //       ███████║╚██████╗██║  ██║██║ ╚████║██║ ╚████║███████╗██║  ██║
  //       ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝

  //  ███████╗ ██████╗ █████╗ ███╗   ██╗███╗   ██╗███████╗██████╗ 
  //  ██╔════╝██╔════╝██╔══██╗████╗  ██║████╗  ██║██╔════╝██╔══██╗
  //  ███████╗██║     ███████║██╔██╗ ██║██╔██╗ ██║█████╗  ██████╔╝
  //  ╚════██║██║     ██╔══██║██║╚██╗██║██║╚██╗██║██╔══╝  ██╔══██╗
  //  ███████║╚██████╗██║  ██║██║ ╚████║██║ ╚████║███████╗██║  ██║
  //  ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝╚═╝  ╚═╝

/*!
* \addtogroup Scanner
* @{
*/
namespace Scanner
{

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

UltrasonicSensor sonar = {.trigPin=9, .echoPin=10};

ServoScanner scanner = {
  .servo = Servo(),
  .pin = 11,
  .minAngle = 0,
  .maxAngle = 90,
  .stepSize = 1
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

int searchForTarget(UltrasonicSensor &s, ServoScanner &sc) {
  while (true) {
    int angle = scanForTargetAngle(s, sc);

    if (angle != -1) {
      Serial.print("Detected angle: ");
      Serial.print(angle);
      Serial.println("°");

      // point at target
      sc.servo.write(angle);
      delay(800);

      // return to center
      int center = (sc.minAngle + sc.maxAngle) / 2;
      sc.servo.write(center);
      delay(500);

      return angle;
    } else {
      Serial.println("No object detected");
      delay(300);
    }
  }
}
}
/*! @} */


enum STATE
{
  SEARCHING,
  FOUND,
  PICKING_UP,
  IDENTIFYING_COLOR,
  MOVING_OBJECT,
  PUTTING_DOWN,
  RESETTING
};

STATE state = RESETTING;
IK::Arm arm = {};

void setup()
{
  // put your setup code here, to run once:
  ColorSensor::initColorSensor();
  Scanner::initScanner();
  arm.final_targetUV = {0,0};
  arm.targetUV = {0,0};
  arm.theta1 = 90;
  arm.theta2 = 90;
  arm.theta3 = 90;
  arm.base_angle = 0;

}

void loop()
{
  // put your main code here, to run repeatedly:
  switch (state) {
    case SEARCHING:
      Motor::turnAll(arm)
    case FOUND:
      Motor::turnAll(arm)
    case PICKING_UP:
      Motor::turnAll(arm)
    case IDENTIFYING_COLOR:
      Motor::turnAll(arm)
    case MOVING_OBJECT:
      Motor::turnAll(arm)
    case PUTTING_DOWN:
      Motor::turnAll(arm)
    case RESETTING:
      Motor::turnAll(arm)
  }

}
