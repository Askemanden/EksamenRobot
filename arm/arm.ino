#include <Wire.h>
#include <Adafruit_TCS34725.h>

/**!
* \addtogroup ColorSensor
* @{
*/
namespace ColorSensor {
    /**
    * @brief Enumeration of detected color categories.
    */
    enum DetectedColor {
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
        TCS34725_GAIN_4X
    );

    /**
    * @brief Initializes serial communication and the TCS34725 sensor.
    *
    * This function must be called before any color reading is attempted.
    * If the sensor is not detected, the program halts.
    */
    void initColorSensor() {
        Serial.begin(9600);
        delay(200);

        if (!tcs.begin()) {
            Serial.println("ERROR: TCS34725 not detected");
            while (1);
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
    void readColorRaw(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &c) {
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
    DetectedColor categorizeColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {

        if (c < 200) {
            return COLOR_BLACK;
        }
        if (c > 4000) {
            return COLOR_WHITE;
        }

        float rn = (float)r;
        float gn = (float)g;
        float bn = (float)b;

        const float minDiff = 0.03;
        const float minRatio = 1.025;

        if (rn > gn * minRatio && rn > bn * minRatio &&
            (rn - gn) > minDiff && (rn - bn) > minDiff) {
            return COLOR_RED;
        }

        if (gn > rn * minRatio && gn > bn * minRatio &&
            (gn - rn) > minDiff && (gn - bn) > minDiff) {
            return COLOR_GREEN;
        }

        if (bn > rn * minRatio && bn > gn * minRatio &&
            (bn - rn) > minDiff && (bn - gn) > minDiff) {
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
    const char* colorToString(DetectedColor color) {
        switch (color) {
            case COLOR_RED:       return "RED";
            case COLOR_GREEN:     return "GREEN";
            case COLOR_BLUE:      return "BLUE";
            case COLOR_BLACK:     return "BLACK";
            case COLOR_WHITE:     return "WHITE";
            default:              return "UNCERTAIN";
        }
    }

    /**
    * @brief Reads the sensor, categorizes the color, and prints the result.
    */
    void detectAndPrintColor() {
        uint16_t r, g, b, c;
        readColorRaw(r, g, b, c);

        DetectedColor color = categorizeColor(r, g, b, c);

        Serial.print("R: "); Serial.print(r);
        Serial.print("  G: "); Serial.print(g);
        Serial.print("  B: "); Serial.print(b);
        Serial.print("  C: "); Serial.print(c);
        Serial.print("  -> Detected: ");
        Serial.println(colorToString(color));
    }
}
/*! @} */

/*!
* \addtogroup IK
* @{
*/
namespace IK {
    /**
    * @brief $L_1$
    */
    #define L1 (10)

    /**
    * @brief $L_2$
    */
    #define L2 (10)

    /**
    * @brief $L_3$
    */
    #define L3 (10)

    #define STEP_DIST (1)

    #define GRAB_HEIGHT (3)

    /**
    * @struct Vector2
    * @brief A 2d vector in the uv plane of the arm.
    * @var Vector2::u U-coordinate (x-equivalent) of the vector.
    * @var Vector2::v v-coordinate (y-equivalent) of the vector.
    */
    struct Vector2 {
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
    struct Arm {
    Vector2 final_targetUV;
    Vector2 targetUV;
    double theta1;
    double theta2;
    double theta3;
    double base_angle;
    };

    Vector2 vectorAdd(Vector2 self, Vector2 other) {
    return { .u = self.u + other.u, .v = self.v + other.v };
    }

    /**
    * @brief Calculates base_angle and writes it to self.
    * @param self The arm for which to calculate data.
    * @param sensor_position_distance Distance of sensor from origin. $d_s$
    * @param sensor_reading_distance The reading of the sensor in centimeters. $L_s$
    * @param sensor_angle_rad Angle in the sensors servo. $\theta_s$
    */
    int calculateBaseAngle(Arm *self, double sensor_position_distance, double sensor_reading_distance, double sensor_angle_rad) {
    /*$d_s*/
    double target_distance = sqrt(sq(sensor_position_distance) + sq(sensor_reading_distance) - 2 * sensor_position_distance * sensor_reading_distance * cos(sensor_angle_rad));
    double sin_theta_O = sin(sensor_angle_rad) * sensor_reading_distance / target_distance;
    double cos_theta_O = (sq(target_distance) + sq(sensor_position_distance) - sq(sensor_reading_distance)) / (2 * target_distance * sensor_position_distance);
    self->base_angle = atan2(sin_theta_O, cos_theta_O);
    self->final_targetUV = {.u=target_distance - L3, .v=0};
    return 0;
    }

    /**
    * @brief Finds the next UV coordinate in the path of self and writes to self on targetUV.
    * @param self The arm to which to write
    */
    int calculateNextTargetUV(Arm *self) {
    Vector2 final_targetUV = self->final_targetUV;

    Vector2 previous_target = self->targetUV;

    double safe_v = final_targetUV.v + GRAB_HEIGHT;

    if (abs(previous_target.u - final_targetUV.u) < 0.01) {
        self->targetUV = vectorAdd(previous_target, { .u = 0, .v = -STEP_DIST });
    }

    else if (previous_target.v < safe_v) {
        self->targetUV = vectorAdd(previous_target, { .u = 0, .v = STEP_DIST });
    }

    else if (previous_target.u < final_targetUV.u) {
        self->targetUV = vectorAdd(previous_target, { .u = STEP_DIST, .v = 0 });
    } else {
        self->targetUV = vectorAdd(previous_target, { .u = -STEP_DIST, .v = 0 });
    }

    if (abs(self->targetUV.u - final_targetUV.u) < STEP_DIST) {
        self->targetUV.u = final_targetUV.u;
    }
    if (abs(self->targetUV.v - final_targetUV.v) < STEP_DIST) {
        self->targetUV.v = final_targetUV.v;
    }

    return 0;
    }


    /**
    * @brief Calculates the angles ($\theta_1 \, \theta_2 \, \theta_3$) of self and write to self.
    * @param self The arm to which to write.
    */
    int calculateArmAngles(Arm *self) {
    Vector2 final_targetUV = self->final_targetUV;
    Vector2 target = self->targetUV;
    self->theta2 = acos((sq(target.u) + sq(target.v) - sq(L1) - sq(L2)) / (2 * L1 * L2));
    double k1 = L1 + L2 * cos(self->theta2);
    double k2 = L2 * sin(self->theta2);
    self->theta1 = atan2(target.v, target.u) - atan2(k2, k1);
    self->theta3 = 2 * PI - self->theta1 - self->theta2 - PI / 2;
    return 0;
    }
}
/*! @} */

/*!
* \addtogroup Motor
* @{
*/
namespace Motor {
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
}
/*! @} */



void setup() {
  // put your setup code here, to run once:
    
}

void loop() {
  // put your main code here, to run repeatedly:

}
