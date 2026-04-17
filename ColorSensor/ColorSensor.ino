#include <Wire.h>
#include <Adafruit_TCS34725.h>

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

void setup() {
    initColorSensor();
}

void loop() {
    detectAndPrintColor();
    delay(500);
}
