/*
 * ============================================================
 * VibePiano Glove - Gesture-Based Air Piano
 * ============================================================
 * Microcontroller : ESP32
 * Sensors         : 5 Flex Sensors + MPU6050
 * Feedback        : 5 Vibration Motors
 * IDE             : Arduino IDE
 *
 * Finger mapping:
 * Thumb  -> C
 * Index  -> D
 * Middle -> E
 * Ring   -> F
 * Pinky  -> G
 *
 * Features:
 * - 10-second straight/bent calibration for every finger
 * - Automatic press/release thresholds
 * - Flex-sensor filtering
 * - MPU6050 neutral/up/down calibration
 * - Octave 3/4/5 selection from wrist pitch
 * - Serial NOTE_ON / NOTE_OFF messages
 * - Optional proportional vibration feedback
 *
 * Required libraries:
 * - Adafruit MPU6050
 * - Adafruit Unified Sensor
 *
 * NOTE:
 * The original PDF/code supplied for this project contains copy/
 * extraction corruption and does not include all function bodies.
 * This file preserves the documented project behavior and
 * reconstructs the missing helper/performance functions so the
 * sketch is complete and compilable.
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

const int NUM_FINGERS = 5;

const int flexPins[NUM_FINGERS] = {35, 36, 34, 32, 33};
const int motorPins[NUM_FINGERS] = {25, 26, 27, 14, 13};

const char* fingerNames[NUM_FINGERS] = {
  "THUMB", "INDEX", "MIDDLE", "RING", "PINKY"
};

const char* baseNotes[NUM_FINGERS] = {
  "C", "D", "E", "F", "G"
};

int straightVal[NUM_FINGERS] = {0, 0, 0, 0, 0};
int bentVal[NUM_FINGERS] = {0, 0, 0, 0, 0};
int thresholdVal[NUM_FINGERS] = {0, 0, 0, 0, 0};
int releaseVal[NUM_FINGERS] = {0, 0, 0, 0, 0};
int bendRangeVal[NUM_FINGERS] = {0, 0, 0, 0, 0};

bool noteOn[NUM_FINGERS] = {false, false, false, false, false};

const unsigned long captureDuration = 10000UL;
const unsigned long mpuReadInterval = 40UL;

const float pressPercent = 0.15f;
const float releasePercent = 0.08f;

const int minimumRange = 60;
const int adcFilterWeightNew = 35;

unsigned long stateStartTime = 0;
unsigned long lastStatusPrint = 0;
unsigned long lastMpuRead = 0;

int currentFinger = 0;
bool motorsEnabled = false;

float filteredFlex[NUM_FINGERS] = {0, 0, 0, 0, 0};

Adafruit_MPU6050 mpu;

float wristPitch = 0.0f;
float wristRoll = 0.0f;

float neutralPitch = 0.0f;
float upPitch = 0.0f;
float downPitch = 0.0f;

int currentOctave = 4;

enum SystemState {
  SHOW_INTRO,
  CALIBRATE_STRAIGHT,
  CALIBRATE_BENT,
  NEXT_FINGER,
  CALIBRATE_MPU_NEUTRAL,
  CALIBRATE_MPU_UP,
  CALIBRATE_MPU_DOWN,
  CALIBRATION_DONE,
  PERFORMANCE_MODE
};

SystemState systemState = SHOW_INTRO;

// ---------- Function declarations ----------
void printIntro();
void handleIntro();
void handleStraightCalibration();
void handleBentCalibration();
void handleNextFinger();
void handleMPUNeutralCalibration();
void handleMPUUpCalibration();
void handleMPUDownCalibration();
void handleCalibrationDone();
void handlePerformanceMode();

void printCountdown(int value, const char* mode);
void readMPU6050();
void updateOctaveFromLearnedTilt();
String buildNoteName(int octave, int fingerIndex);
void printCalibrationTable();

int readFilteredFlex(int finger);
void computeFingerThresholds(int finger);

void setMotor(int finger, int intensity);
void turnAllMotorsOff();

bool isFingerPressed(int finger, int value);
bool isFingerReleased(int finger, int value);
int getBendPercent(int finger, int value);

void sendNoteOn(int finger);
void sendNoteOff(int finger);


// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(1500);

  for (int i = 0; i < NUM_FINGERS; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);

    filteredFlex[i] = analogRead(flexPins[i]);
  }

  // MPU6050 I2C: SDA = GPIO21, SCL = GPIO22
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("MPU6050 NOT DETECTED. CHECK WIRING.");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  printIntro();

  stateStartTime = millis();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  switch (systemState) {

    case SHOW_INTRO:
      handleIntro();
      break;

    case CALIBRATE_STRAIGHT:
      handleStraightCalibration();
      break;

    case CALIBRATE_BENT:
      handleBentCalibration();
      break;

    case NEXT_FINGER:
      handleNextFinger();
      break;

    case CALIBRATE_MPU_NEUTRAL:
      handleMPUNeutralCalibration();
      break;

    case CALIBRATE_MPU_UP:
      handleMPUUpCalibration();
      break;

    case CALIBRATE_MPU_DOWN:
      handleMPUDownCalibration();
      break;

    case CALIBRATION_DONE:
      handleCalibrationDone();
      break;

    case PERFORMANCE_MODE:
      handlePerformanceMode();
      break;
  }
}


// ============================================================
// INTRO
// ============================================================

void printIntro() {

  Serial.println();
  Serial.println("==================================================");
  Serial.println("VIBEPAINO GLOVE - FINAL SENSITIVE VERSION");
  Serial.println("==================================================");
  Serial.println("STEP 1: FLEX CALIBRATION");
  Serial.println("STEP 2: MPU CALIBRATION");
  Serial.println("NEUTRAL = OCTAVE 4");
  Serial.println("UP TILT = OCTAVE 5");
  Serial.println("DOWN TILT = OCTAVE 3");
  Serial.println("==================================================");
}


void handleIntro() {

  Serial.println("STARTING FINGER CALIBRATION...");
  Serial.println("FIRST FINGER: THUMB");

  delay(1500);

  currentFinger = 0;
  stateStartTime = millis();
  lastStatusPrint = 0;

  systemState = CALIBRATE_STRAIGHT;
}


// ============================================================
// FLEX SENSOR CALIBRATION - STRAIGHT
// ============================================================

void handleStraightCalibration() {

  static long sum = 0;
  static int count = 0;
  static bool initialized = false;

  if (!initialized) {
    sum = 0;
    count = 0;
    initialized = true;

    Serial.print("KEEP ");
    Serial.print(fingerNames[currentFinger]);
    Serial.println(" STRAIGHT FOR 10 SECONDS...");
  }

  int flexVal = analogRead(flexPins[currentFinger]);

  sum += flexVal;
  count++;

  printCountdown(flexVal, "STRAIGHT");

  if (millis() - stateStartTime >= captureDuration) {

    straightVal[currentFinger] = sum / count;

    Serial.print("STRAIGHT SAVED FOR ");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(": ");
    Serial.println(straightVal[currentFinger]);

    initialized = false;
    stateStartTime = millis();
    lastStatusPrint = 0;

    systemState = CALIBRATE_BENT;
  }
}


// ============================================================
// FLEX SENSOR CALIBRATION - BENT
// ============================================================

void handleBentCalibration() {

  static long sum = 0;
  static int count = 0;
  static bool initialized = false;

  if (!initialized) {
    sum = 0;
    count = 0;
    initialized = true;

    Serial.print("BEND ");
    Serial.print(fingerNames[currentFinger]);
    Serial.println(" FULLY FOR 10 SECONDS...");
  }

  int flexVal = analogRead(flexPins[currentFinger]);

  sum += flexVal;
  count++;

  printCountdown(flexVal, "BENT");

  if (millis() - stateStartTime >= captureDuration) {

    bentVal[currentFinger] = sum / count;

    computeFingerThresholds(currentFinger);

    Serial.print("BENT SAVED FOR ");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(": ");
    Serial.println(bentVal[currentFinger]);

    Serial.print("RANGE FOR ");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(": ");
    Serial.println(bendRangeVal[currentFinger]);

    Serial.print("PRESS THRESHOLD FOR ");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(": ");
    Serial.println(thresholdVal[currentFinger]);

    Serial.print("RELEASE THRESHOLD FOR ");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(": ");
    Serial.println(releaseVal[currentFinger]);

    initialized = false;
    stateStartTime = millis();

    systemState = NEXT_FINGER;
  }
}


// ============================================================
// MOVE TO NEXT FINGER
// ============================================================

void handleNextFinger() {

  currentFinger++;

  if (currentFinger < NUM_FINGERS) {

    Serial.println("--------------------------------------");
    Serial.print("NEXT FINGER: ");
    Serial.println(fingerNames[currentFinger]);
    Serial.println("--------------------------------------");

    delay(1200);

    stateStartTime = millis();
    lastStatusPrint = 0;

    systemState = CALIBRATE_STRAIGHT;

  } else {

    Serial.println("ALL FLEX SENSORS CALIBRATED.");
    Serial.println("STARTING MPU CALIBRATION...");

    delay(1500);

    stateStartTime = millis();
    lastStatusPrint = 0;

    systemState = CALIBRATE_MPU_NEUTRAL;
  }
}


// ============================================================
// MPU6050 - NEUTRAL CALIBRATION
// ============================================================

void handleMPUNeutralCalibration() {

  static float sum = 0;
  static int count = 0;
  static bool initialized = false;

  if (!initialized) {

    sum = 0;
    count = 0;
    initialized = true;

    Serial.println("KEEP HAND IN NEUTRAL POSITION FOR 10 SECONDS...");
  }

  readMPU6050();

  sum += wristPitch;
  count++;

  if (millis() - lastStatusPrint >= 1000) {

    lastStatusPrint = millis();

    Serial.print("NEUTRAL PITCH = ");
    Serial.println(wristPitch, 1);
  }

  if (millis() - stateStartTime >= captureDuration) {

    neutralPitch = sum / count;

    Serial.print("NEUTRAL SAVED: ");
    Serial.println(neutralPitch, 2);

    initialized = false;
    stateStartTime = millis();
    lastStatusPrint = 0;

    systemState = CALIBRATE_MPU_UP;
  }
}


// ============================================================
// MPU6050 - UP CALIBRATION
// ============================================================

void handleMPUUpCalibration() {

  static float sum = 0;
  static int count = 0;
  static bool initialized = false;

  if (!initialized) {

    sum = 0;
    count = 0;
    initialized = true;

    Serial.println("TILT HAND UP FOR 10 SECONDS...");
  }

  readMPU6050();

  sum += wristPitch;
  count++;

  if (millis() - lastStatusPrint >= 1000) {

    lastStatusPrint = millis();

    Serial.print("UP PITCH = ");
    Serial.println(wristPitch, 1);
  }

  if (millis() - stateStartTime >= captureDuration) {

    upPitch = sum / count;

    Serial.print("UP SAVED: ");
    Serial.println(upPitch, 2);

    initialized = false;
    stateStartTime = millis();
    lastStatusPrint = 0;

    systemState = CALIBRATE_MPU_DOWN;
  }
}


// ============================================================
// MPU6050 - DOWN CALIBRATION
// ============================================================

void handleMPUDownCalibration() {

  static float sum = 0;
  static int count = 0;
  static bool initialized = false;

  if (!initialized) {

    sum = 0;
    count = 0;
    initialized = true;

    Serial.println("TILT HAND DOWN FOR 10 SECONDS...");
  }

  readMPU6050();

  sum += wristPitch;
  count++;

  if (millis() - lastStatusPrint >= 1000) {

    lastStatusPrint = millis();

    Serial.print("DOWN PITCH = ");
    Serial.println(wristPitch, 1);
  }

  if (millis() - stateStartTime >= captureDuration) {

    downPitch = sum / count;

    Serial.print("DOWN SAVED: ");
    Serial.println(downPitch, 2);

    initialized = false;

    systemState = CALIBRATION_DONE;
  }
}


// ============================================================
// CALIBRATION COMPLETE
// ============================================================

void handleCalibrationDone() {

  Serial.println("==================================================");
  Serial.println("CALIBRATION COMPLETED SUCCESSFULLY");

  printCalibrationTable();

  Serial.println("SYSTEM READY");
  Serial.println("==================================================");

  delay(2000);

  systemState = PERFORMANCE_MODE;
}


// ============================================================
// PERFORMANCE MODE
// ============================================================

void handlePerformanceMode() {

  // Update MPU data at approximately 25 Hz.
  if (millis() - lastMpuRead >= mpuReadInterval) {

    lastMpuRead = millis();

    readMPU6050();
    updateOctaveFromLearnedTilt();
  }

  for (int i = 0; i < NUM_FINGERS; i++) {

    int flexValue = readFilteredFlex(i);

    if (!noteOn[i]) {

      if (isFingerPressed(i, flexValue)) {
        sendNoteOn(i);
      }

    } else {

      if (isFingerReleased(i, flexValue)) {
        sendNoteOff(i);
      } else {

        // Keep haptic intensity proportional to finger bend.
        int bendPercent = getBendPercent(i, flexValue);

        if (motorsEnabled) {
          int intensity = map(bendPercent, 0, 100, 40, 255);
          intensity = constrain(intensity, 0, 255);
          setMotor(i, intensity);
        }
      }
    }
  }
}


// ============================================================
// FLEX SENSOR FILTER
// ============================================================

int readFilteredFlex(int finger) {

  int raw = analogRead(flexPins[finger]);

  // Weight of the new sample = 35%.
  // Weight of previous filtered value = 65%.
  filteredFlex[finger] =
      ((100 - adcFilterWeightNew) * filteredFlex[finger]
       + adcFilterWeightNew * raw) / 100.0f;

  return (int)filteredFlex[finger];
}


// ============================================================
// THRESHOLD CALCULATION
// ============================================================

void computeFingerThresholds(int finger) {

  int straight = straightVal[finger];
  int bent = bentVal[finger];

  int range = abs(bent - straight);

  // Prevent unusably small calibration ranges.
  if (range < minimumRange) {
    range = minimumRange;
  }

  bendRangeVal[finger] = range;

  /*
   * The threshold is based on 15% of the calibrated bend range.
   * The release threshold is based on 8%.
   *
   * This works whether the ADC value increases or decreases
   * when the flex sensor bends.
   */
  if (bent > straight) {

    thresholdVal[finger] =
        straight + (int)(range * pressPercent);

    releaseVal[finger] =
        straight + (int)(range * releasePercent);

  } else {

    thresholdVal[finger] =
        straight - (int)(range * pressPercent);

    releaseVal[finger] =
        straight - (int)(range * releasePercent);
  }
}


// ============================================================
// FINGER PRESS / RELEASE DETECTION
// ============================================================

bool isFingerPressed(int finger, int value) {

  if (bentVal[finger] > straightVal[finger]) {
    return value >= thresholdVal[finger];
  }

  return value <= thresholdVal[finger];
}


bool isFingerReleased(int finger, int value) {

  if (bentVal[finger] > straightVal[finger]) {
    return value <= releaseVal[finger];
  }

  return value >= releaseVal[finger];
}


// ============================================================
// BEND PERCENTAGE
// ============================================================

int getBendPercent(int finger, int value) {

  int straight = straightVal[finger];
  int bent = bentVal[finger];

  int range = abs(bent - straight);

  if (range < 1) {
    return 0;
  }

  int bend;

  if (bent > straight) {
    bend = value - straight;
  } else {
    bend = straight - value;
  }

  int percent = (bend * 100) / range;

  return constrain(percent, 0, 100);
}


// ============================================================
// MPU6050 READING
// ============================================================

void readMPU6050() {

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  mpu.getEvent(&accel, &gyro, &temp);

  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;

  /*
   * Pitch calculated from accelerometer gravity vector.
   * The result is expressed in degrees.
   */
  wristPitch =
      atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;

  wristRoll =
      atan2(ay, az) * 180.0f / PI;
}


// ============================================================
// OCTAVE CONTROL
// ============================================================

void updateOctaveFromLearnedTilt() {

  /*
   * Determine boundaries halfway between the learned
   * neutral/up/down positions.
   */

  float upBoundary = (neutralPitch + upPitch) / 2.0f;
  float downBoundary = (neutralPitch + downPitch) / 2.0f;

  int newOctave = 4;

  // Handles either sign convention for the MPU pitch.
  if (upPitch > neutralPitch) {

    if (wristPitch > upBoundary) {
      newOctave = 5;
    } else if (wristPitch < downBoundary) {
      newOctave = 3;
    }

  } else {

    if (wristPitch < upBoundary) {
      newOctave = 5;
    } else if (wristPitch > downBoundary) {
      newOctave = 3;
    }
  }

  if (newOctave != currentOctave) {

    currentOctave = newOctave;

    Serial.print("SYSTEM_STATUS,OCTAVE=");
    Serial.print(currentOctave);
    Serial.print(",PITCH=");
    Serial.println(wristPitch, 1);
  }
}


// ============================================================
// NOTE NAME GENERATION
// ============================================================

String buildNoteName(int octave, int fingerIndex) {

  if (fingerIndex < 0 || fingerIndex >= NUM_FINGERS) {
    return "";
  }

  String note = baseNotes[fingerIndex];

  note += String(octave);

  return note;
}


// ============================================================
// NOTE ON
// ============================================================

void sendNoteOn(int finger) {

  noteOn[finger] = true;

  String noteName = buildNoteName(currentOctave, finger);

  int bendPercent = getBendPercent(
      finger,
      (int)filteredFlex[finger]
  );

  Serial.print("NOTE_ON,");
  Serial.print(noteName);
  Serial.print(",FINGER=");
  Serial.print(fingerNames[finger]);
  Serial.print(",OCTAVE=");
  Serial.print(currentOctave);
  Serial.print(",BEND=");
  Serial.println(bendPercent);

  if (motorsEnabled) {

    int intensity = map(bendPercent, 0, 100, 40, 255);
    intensity = constrain(intensity, 0, 255);

    setMotor(finger, intensity);
  }
}


// ============================================================
// NOTE OFF
// ============================================================

void sendNoteOff(int finger) {

  noteOn[finger] = false;

  String noteName = buildNoteName(currentOctave, finger);

  setMotor(finger, 0);

  Serial.print("NOTE_OFF,");
  Serial.print(noteName);
  Serial.print(",FINGER=");
  Serial.print(fingerNames[finger]);
  Serial.print(",OCTAVE=");
  Serial.println(currentOctave);
}


// ============================================================
// VIBRATION MOTOR CONTROL
// ============================================================

void setMotor(int finger, int intensity) {

  intensity = constrain(intensity, 0, 255);

  /*
   * Arduino-ESP32 provides analogWrite() for PWM output.
   * This avoids dependence on the older ledcSetup()/ledcAttachPin()
   * API used by some older ESP32 Arduino cores.
   */
  analogWrite(motorPins[finger], intensity);
}


void turnAllMotorsOff() {

  for (int i = 0; i < NUM_FINGERS; i++) {
    analogWrite(motorPins[i], 0);
    noteOn[i] = false;
  }
}


// ============================================================
// CALIBRATION TABLE
// ============================================================

void printCalibrationTable() {

  Serial.println();
  Serial.println("==================================================");
  Serial.println("FINGER CALIBRATION TABLE");
  Serial.println("==================================================");

  Serial.println("Finger\tStraight\tBent\tRange\tPress\tRelease");

  for (int i = 0; i < NUM_FINGERS; i++) {

    Serial.print(fingerNames[i]);
    Serial.print("\t");

    Serial.print(straightVal[i]);
    Serial.print("\t\t");

    Serial.print(bentVal[i]);
    Serial.print("\t");

    Serial.print(bendRangeVal[i]);
    Serial.print("\t");

    Serial.print(thresholdVal[i]);
    Serial.print("\t");

    Serial.println(releaseVal[i]);
  }

  Serial.println();
  Serial.print("MPU Neutral Pitch: ");
  Serial.println(neutralPitch, 2);

  Serial.print("MPU Up Pitch:      ");
  Serial.println(upPitch, 2);

  Serial.print("MPU Down Pitch:    ");
  Serial.println(downPitch, 2);

  Serial.println("==================================================");
}


// ============================================================
// CALIBRATION COUNTDOWN
// ============================================================

void printCountdown(int value, const char* mode) {

  unsigned long elapsed = millis() - stateStartTime;

  unsigned long remaining =
      (captureDuration > elapsed)
      ? (captureDuration - elapsed)
      : 0;

  int secondsRemaining =
      (int)((remaining + 999) / 1000);

  if (millis() - lastStatusPrint >= 1000) {

    lastStatusPrint = millis();

    Serial.print("FINGER=");
    Serial.print(fingerNames[currentFinger]);
    Serial.print(",MODE=");
    Serial.print(mode);
    Serial.print(",VALUE=");
    Serial.print(value);
    Serial.print(",REMAINING=");
    Serial.print(secondsRemaining);
    Serial.println("s");
  }
}
