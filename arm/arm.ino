#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Encoder.h>


// LLL        CCCCCCC    DDDDDDD
// LLL      CCC     CCC  DDD    DD
// LLL     CCC           DDD     DD
// LLL     CCC           DDD     DD
// LLL     CCC           DDD     DD
// LLL      CCC     CCC  DDD    DD
// LLLLLLL    CCCCCCC    DDDDDDD

/*
* \addtogroup LCD
* ! @{
*/
namespace LCD{

const int LCD_COLS = 16;
const int SCROLL_DELAY = 300;
const int numRows = 2;

// LCD PINS
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


struct LCDRow {
  String text;
  int    scrollPos  = 0;
  bool   scrolling  = false;
  unsigned long lastTick = 0;
};

LCDRow lcdRows[numRows]; 

void Print(String text, int row) {
  LCDRow &r = lcdRows[row];
  r.text      = text;
  r.scrollPos = 0;
  r.lastTick  = 0;
  r.scrolling = (text.length() > LCD_COLS);

  lcd.setCursor(0, row);
  lcd.print(text.substring(0, LCD_COLS));
  for (int i = text.length(); i < LCD_COLS; i++) lcd.print(' ');
}

void Update() {
  unsigned long now = millis();

  for (int row = 0; row < numRows; row++) {
    LCDRow &r = lcdRows[row];
    if (!r.scrolling) continue;
    if (now - r.lastTick < SCROLL_DELAY) continue;
    r.lastTick += SCROLL_DELAY;

    String padded = r.text + "                  ";
    lcd.setCursor(0, row);
    lcd.print(padded.substring(r.scrollPos, r.scrollPos + LCD_COLS));

    r.scrollPos++;
    if (r.scrollPos > (int)r.text.length()) r.scrollPos = 0;
  }
}
}
/*! @}*/


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
  const double L1 = 20;

  /**
   * @brief $L_2$
   */
  const double L2 = 15;

  /**
   * @brief $L_3$
   */
  const double L3 = 8; 

  const double STEP_DIST = 1;

  const double GRAB_HEIGHT = L3; // Højden håndledet er over målet.

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
   * @param d_s Distance of sensor from origin. $d_s$
   * @param L_s The reading of the sensor in centimeters. $L_s$
   * @param theta_s Angle in the sensors servo. $\theta_s$
   */
  int calculateFinalTarget(Arm *self, double d_s, double L_s, double theta_s)
  {
    theta_s -= PI/4;
    theta_s = PI - theta_s;
    // d_mal = sqrt(d_s^2 + L_s^2 - 2*d_s*L_s*cos(theta_s))
    double d_mal = sqrt(sq(d_s) + sq(L_s) - 2.0 * d_s * L_s * cos(theta_s));

    // numerator = (sin(theta_s) / d_mal) * L_s
    double num = (sin(theta_s) / d_mal) * L_s;

    // denominator = (d_mal^2 + d_s^2 - L_s^2) / (2 * d_mal * d_s)
    double den = (sq(d_mal) + sq(d_s) - sq(L_s)) / (2.0 * d_mal * d_s);

    self->base_angle = atan2(num, den);
    self->final_targetUV = {.u = d_mal, 0};
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

    if (abs(previous_target.u - final_targetUV.u) < 0.05 && abs(previous_target.v - final_targetUV.v) < 0.05){
      return 1;
    }

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
  const int MAX_SPEED = 100;
  const int threshold = 3;

  struct PID
  {
    double kp = 0, ki = 0, kd = 0;
    int integral = 0, last_error = 0;
  };

  struct MotorUnit
  {
    unsigned int FULL_ROTATION = 0, MIN_POWER = 0;

    uint8_t enable = 0;
    uint8_t direction1 = 0, direction2 = 0;
    long current_position = 0, target_position = 0, start_position = 0, previous_position = current_position;
    bool finished = false;
    uint8_t finished_ticks = 0, stop_ticks = 0;

    PID pid_data = {};
  };

  MotorUnit baseMotor = {};
  MotorUnit shoulderMotor = {};
  MotorUnit elbowMotor = {};

  Encoder encBase(20, 21);
  Encoder encShoulder(18, 19);
  Encoder encElbow(2, 3);

  void turn(MotorUnit &m, bool cw, unsigned int speed)
  {
    if (m.enable != 0)
    {
      digitalWrite(m.direction1, cw);
      digitalWrite(m.direction2, !cw);
      analogWrite(m.enable, speed);
      return;
    }

    if (cw)
    {
      analogWrite(m.direction1, speed);
      analogWrite(m.direction2, 0);
    }
    else
    {
      analogWrite(m.direction1, 0);
      analogWrite(m.direction2, speed);
    }
  }

  long radians_to_counts(MotorUnit &m, double rad)
  {
    double degrees = rad * 180.0 / PI;
    return (long)(degrees * m.FULL_ROTATION / 360.0);
  }

  void dc_motor_update(MotorUnit &m)
  {
    long error = m.target_position - m.current_position;
    
    // Spring motoren over, hvis den allerede er færdig
    if (m.finished)
    {
      turn(m, false, 0);
      return;
    }
    // Hvis motoren er inden for threshold længe nok, bliver den færdig.
    if (abs(error) <= threshold)
    {
      turn(m, false, 0);
      if (m.finished_ticks >= 250)
      {
        m.finished = true;
        m.finished_ticks = 0;
        return;
      }
      else
      {
        m.finished_ticks++;
      }
    }
    else
    {
      m.finished_ticks = 0;
    }

    // Tjek om motoren forhindres i at bevæge sig
    if (m.current_position <= m.previous_position + threshold && m.current_position >= m.previous_position - threshold)
    {
      m.stop_ticks++;
    }
    if (m.stop_ticks >= 200)
    {
      m.finished = true;
      return;
    }

    m.pid_data.integral += error;
    m.pid_data.integral = constrain(m.pid_data.integral, -1000, 1000);

    double derivative = (error - m.pid_data.last_error);
    m.pid_data.last_error = error;

    double output =
        m.pid_data.kp * error +
        m.pid_data.ki * m.pid_data.integral +
        m.pid_data.kd * derivative;

    int speed = abs(output);

    turn(m, (output > 0), constrain(speed, (int)m.MIN_POWER, MAX_SPEED));
  }

  void initialize_motor(
      uint8_t dir1,
      uint8_t dir2,
      int FULL_ROTATION,
      int MIN_POWER,
      double kp, double ki, double kd,
      MotorUnit &m,
      uint8_t enable = 0)
  {
    m.direction1 = dir1;
    m.direction2 = dir2;
    m.enable = enable;
    m.FULL_ROTATION = FULL_ROTATION;
    m.MIN_POWER = MIN_POWER;

    m.pid_data.kp = kp;
    m.pid_data.ki = ki;
    m.pid_data.kd = kd;

    pinMode(dir1, OUTPUT);
    pinMode(dir2, OUTPUT);
  }

  // API
  void turnAll(IK::Arm arm)
  {
    baseMotor.target_position     = radians_to_counts(baseMotor, arm.base_angle);
    shoulderMotor.target_position = radians_to_counts(shoulderMotor, arm.theta1);
    elbowMotor.target_position = radians_to_counts(elbowMotor, (-(arm.theta1 + arm.theta2)));
    // Wrist (theta3) omitted as it is without motor
    baseMotor.finished = false;
    baseMotor.finished_ticks = 0;
    shoulderMotor.finished = false;
    shoulderMotor.finished_ticks = 0;
    elbowMotor.finished = false;
    elbowMotor.finished_ticks = 0;
  }

  void updateAll()
  {
    // Read encoders
    baseMotor.current_position     = baseMotor.start_position + encBase.read();
    shoulderMotor.current_position = shoulderMotor.start_position + encShoulder.read();
    elbowMotor.current_position = -elbowMotor.start_position -encElbow.read();

    if (!baseMotor.finished)
    {
      dc_motor_update(baseMotor);
    }
    else if (!shoulderMotor.finished)
    {
      dc_motor_update(shoulderMotor);
    }
    else
    {
      dc_motor_update(elbowMotor);
    }
  }

  void initialize(double theta1,double theta2,double base_angle)
  {
    initialize_motor(8, 7,  (int)(6 * 30 * 64), 50, 0.6, 0.01, 0.5, baseMotor, 9); 
    initialize_motor(5, 6, (int)(2*102.083 * 64), 50, 0.6, 0.01, 0.55,  shoulderMotor);
    initialize_motor(11, 10, (int)(2*30 * 64), 50, 0.6, 0.01, 0.5,  elbowMotor);
    baseMotor.current_position = radians_to_counts(baseMotor,base_angle);
    baseMotor.target_position = radians_to_counts(baseMotor, base_angle);
    baseMotor.start_position = radians_to_counts(baseMotor, base_angle);
    shoulderMotor.current_position = radians_to_counts(shoulderMotor, theta1);    
    shoulderMotor.target_position= radians_to_counts(shoulderMotor, theta1); 
    shoulderMotor.start_position= radians_to_counts(shoulderMotor, theta1);
    elbowMotor.current_position = radians_to_counts(elbowMotor, (theta1 + theta2));    
    elbowMotor.target_position= radians_to_counts(elbowMotor, (theta1 + theta2)); 
    elbowMotor.start_position= radians_to_counts(elbowMotor, (theta1 + theta2));
  }


void move()
{
  while (!shoulderMotor.finished || !elbowMotor.finished || !baseMotor.finished)
  {
    updateAll();
  }
  return;
}

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

UltrasonicSensor sonar = {.trigPin=24, .echoPin=13};

ServoScanner scanner = {
  .servo = Servo(),
  .pin = 12,
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
  delayMicroseconds(4);
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


ScanResult scan(UltrasonicSensor &s, ServoScanner &sc) {
  ScanResult r1 = performScan(s, sc, sc.minAngle, sc.maxAngle);
  if (r1.detected) return r1;

  ScanResult r2 = performScan(s, sc, sc.maxAngle, sc.minAngle);
  if (r2.detected) return r2;

  return {-1,-1,-1,-1,-1,-1,-1};
}

void initScanner(){
  pinMode(sonar.trigPin, OUTPUT);
  pinMode(sonar.echoPin, INPUT);
  scanner.servo.attach(scanner.pin);
}
// MAIN CALLABLE FUNCTION
ScanResult searchForTarget(UltrasonicSensor &s, ServoScanner &sc) {
  while (true) {
    ScanResult result = scan(s, sc);
    int angle = result.trueAngle;

    if (angle != -1) {
      Serial.print("Detected angle: ");
      Serial.print(angle);
      Serial.println("°");

      // point at target
      sc.servo.write(angle);
      delay(200);

      // return to center
      int center = (sc.minAngle + sc.maxAngle) / 2;
      sc.servo.write(center);

      return result;
    } else {
      Serial.println("No object detected");
      delay(100);
    }
  }
}
}
/*! @} */

enum STATE
{
  SEARCHING,
  MOVING_TO_OBJECT,
  PICKING_UP,
  IDENTIFYING_COLOR,
  MOVING_OBJECT,
  PUTTING_DOWN,
  RESETTING
};

// IK::Arm arm = {};

// void setup()
// {
//   // put your setup code here, to run once:
//   ColorSensor::initColorSensor();
//   Scanner::initScanner();
//   Motor::init()
//   arm.final_targetUV = {IK::L2, IK::L1-IK::L3};
//   arm.targetUV = {IK::L2, IK::L1-IK::L3};
//   arm.theta1 = 90;
//   arm.theta2 = 90;
//   arm.theta3 = 90;
//   arm.base_angle = 0;
// }

// STATE state = RESETTING;
// int sensor_angle = 0;
// float sensor_detection_distance = 0;
// const int sensor_position = 5;

// ColorSensor::DetectedColor object_color = ColorSensor::COLOR_UNCERTAIN;
// uint16_t r, g, b, c;



// void loop()
// {
//   LCD::Update();
//   Motor::updateAll();

//   switch (state)
//   {
//     case SEARCHING:
//     {
//       Scanner::ScanResult scan_result = Scanner::searchForTarget(Scanner::sonar, Scanner::scanner);

//       sensor_angle = scan_result.trueAngle;
//       sensor_detection_distance = scan_result.objectDistance;

//       state = MOVING_TO_OBJECT;
//       break;
//     }

//     case MOVING_TO_OBJECT:
//     {
//       LCD::Print(
//         "Target at " + String(sensor_angle) + "deg " +
//         String(sensor_detection_distance, 1) + "cm", 0
//       );
//       LCD::Update();


//       // Compute global target
//       IK::calculateFinalTarget(&arm, sensor_position, sensor_detection_distance, radians(sensor_angle));

//       // Step toward target
//       IK::calculateNextTargetUV(&arm);
//       IK::calculateArmAngles(&arm);

//       Motor::turnAll(arm);

//       // Check if reached final target (within tolerance)
//       if (abs(arm.targetUV.u - arm.final_targetUV.u) < 0.5 &&
//           abs(arm.targetUV.v - arm.final_targetUV.v) < 0.5)
//       {
//         state = PICKING_UP;
//       }

//       break;
//     }

//     case PICKING_UP:
//     {
//       LCD::Print("Picking up...", 0);
//       LCD::Update();

//       // --- GRIPPER CLOSE (SKIPPED HARDWARE CODE) ---
//       // closeGripper();

//       delay(500);

//       state = IDENTIFYING_COLOR;
//       break;
//     }

//     case IDENTIFYING_COLOR:
//     {
//       ColorSensor::readColorRaw(r, g, b, c);
//       object_color = ColorSensor::categorizeColor(r, g, b, c);

//       LCD::Print(
//         "Color: " + String(ColorSensor::colorToString(object_color)), 0
//       );
//       LCD::Update();

//       state = MOVING_OBJECT;
//       break;
//     }

//     case MOVING_OBJECT:
//     {
//       LCD::Print("Moving object", 0);
//       LCD::Update();

//       // --- SET NEW TARGET BASED ON COLOR (SKIPPED LOGIC) ---
//       // e.g. arm.final_targetUV = dropZoneForColor(object_color);

//       IK::calculateNextTargetUV(&arm);
//       IK::calculateArmAngles(&arm);

//       Motor::turnAll(arm);

//       // Check arrival
//       if (abs(arm.targetUV.u - arm.final_targetUV.u) < 0.5 &&
//           abs(arm.targetUV.v - arm.final_targetUV.v) < 0.5)
//       {
//         state = PUTTING_DOWN;
//       }

//       break;
//     }

//     case PUTTING_DOWN:
//     {
//       LCD::Print("Releasing...", 0);
//       LCD::Update();

//       // --- GRIPPER OPEN (SKIPPED HARDWARE CODE) ---
//       // openGripper();

//       delay(500);

//       state = RESETTING;
//       break;
//     }

//     case RESETTING:
//     {
//       LCD::Print("Resetting", 0);

//       // Move back to home position
//       arm.final_targetUV = {IK::L2, IK::L1-IK::L3};

//       IK::calculateNextTargetUV(&arm);
//       IK::calculateArmAngles(&arm);

//       Motor::turnAll(arm);

//       if (abs(arm.targetUV.u) < 0.5 &&
//           abs(arm.targetUV.v) < 0.5)
//       {
//         state = SEARCHING;
//       }

//       break;
//     }
//   }
// }


IK::Arm arm = {};

void setup() {
  Serial.begin(921600);

  arm.final_targetUV = {IK::L2 + IK::L1,0};
  arm.targetUV       = {IK::L2, IK::L1 - IK::L3};

  IK::calculateArmAngles(&arm);

  Motor::initialize(arm.theta1, arm.theta2, arm.base_angle);
  Scanner::initScanner();
  Scanner::ScanResult skid = Scanner::searchForTarget(Scanner::sonar, Scanner::scanner);

  Serial.println(skid.trueAngle);
  Serial.println(skid.objectDistance);

  IK::calculateFinalTarget(&arm, 15, skid.objectDistance, skid.trueAngle*PI/180);

}

void loop() {
  arm.targetUV = arm.final_targetUV;

  // Clamp to final target to prevent overshoot
  if (abs(arm.targetUV.u - arm.final_targetUV.u) < IK::STEP_DIST)
     arm.targetUV.u = arm.final_targetUV.u;
  if (abs(arm.targetUV.v - arm.final_targetUV.v) < IK::STEP_DIST)
     arm.targetUV.v = arm.final_targetUV.v;


  IK::calculateArmAngles(&arm);
  Motor::turnAll(arm);
  ;
  Serial.print("Base angle ");
  Serial.println(arm.base_angle);
  Serial.print("Basemotor position: "); Serial.println(Motor::baseMotor.current_position);
  Serial.print("Base motor target ");
  Serial.println(Motor::baseMotor.target_position);
  Serial.print("UV: (");
  Serial.print(arm.targetUV.u);
  Serial.print(", ");
  Serial.print(arm.targetUV.v);
  Serial.print(")  t1: ");
  Serial.print(String(Motor::shoulderMotor.current_position) + " " + String(Motor::shoulderMotor.target_position));
  Serial.print("  t2: ");
  Serial.println(String(Motor::elbowMotor.current_position) + " " + String(Motor::elbowMotor.target_position));

  Motor::move();
}
