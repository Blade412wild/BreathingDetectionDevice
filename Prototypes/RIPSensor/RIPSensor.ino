
// states
enum class RIPSensorState { Idle,
                            RipSensorConnecting,
                            Calibration,
                            Active };
enum class CalibrationState { Idle,
                              ChestMin,
                              ChestMax,
                              Finished };
enum class LedConnectionState { Idle,
                                Connecting,
                                Connected,
                                DisConnected };

enum class BreathingState { Exhaling = -1,
                            HoldingBreath = 0,
                            Inhaling = 1 };


const int num_LEDs = 5;                              // number of LEDs for each sensor
const int chest_LED_pins[] = { 9, 10, 11, 12, 13 };  // chest sensor LED pins in the order you want them to light up
const int belly_LED_pins[] = { 8, 7, 6, 5, 4 };      // belly sensor LED pins in the order you want them to light up

// RIPSensor Data
//RIPSensorState RIPSensorCurrentState = RipSensorState::Idle;
//const String speedVarName = "1";
const String BreathingStateName = "1";
const String inhaleExhaleSpeedName = "2";
const String chestPositionVarName = "3";

const int RIPIdleStateToken = 0;
const int RIPConnectingStateToken = 1;
const int RIPCalibrationStateToken = 2;
const int RIPActiveStateToken = 3;

const int chest_sensor_pin = A0;
const int belly_sensor_pin = A5;

int chestMinRaw;
int chestMaxRaw;
int chest_sensor_readingRaw;
float chestFinalValue;
float chestPreviousValue;
int chest_sensor_readingAverage;

int belly_min_adjust_reading;
int belly_max_adjust_reading;
int belly_sensor_reading;


// RIPSensor ConnectionState vars
bool isConnected = false;

// ConnectionLed Vars
RIPSensorState currentRIPSensorState = RIPSensorState::Idle;
const int ledPin = 3;
bool ledStatus = false;
int ledSetupBlinkInterval = 250;
int ledCalibrationBlinkInterval = 500;
int ledConnectedBlinkInterval = 1000;
int lastTimeBlinked = 0;
int currentledBlinkInterval = 0;

// Calibration vars
CalibrationState currentCalibrationState = CalibrationState::Idle;
int NextCallibrationState;
const int CalibrationDoneToken = 0;
const int abortCalibrationToken = 1;

const int nextStepToken = 2;
const int previousStepToken = 3;
const int restartCalibrationToken = 4;

bool calibrationIsDone = false;
bool abortCalibration = false;

bool firstEntered = true;

// sendingData Setup
int connectionTokenSendInterval = 50;
int lastTimeConnectionTokenSent = 0;
const String connectionToken = "-1";

// sendingData SensorData
int sensorDataInterval = 50;  // sending sensorDataInterval
int lastTimeSensorDataSent = 0;
String SendingData = "";
String valueSplitter = ":";
String varSplitter = "|";

// breathingState parameters
BreathingState currentBreathingState;
const float exhaleSpeedThreshold = -1.0;
const float inhaleSpeedThreshold = 1.2;
const int collectedSpeedsLenght = 5;
float inhaleExhaleSpeed = 0;
float collectedSpeeds[collectedSpeedsLenght - 1];

String message;

int counter = 0;
int readingInterval = 6;
int speedCounter = 0;


// time
unsigned long currentTime = 0;
unsigned long startTime = 0;


void setup() {  // code that only runs once
  Serial.begin(9600);
  SetupBreathingLeds();
  startTime = millis();
  currentledBlinkInterval = ledSetupBlinkInterval;
  RIPSensorConnectingState();
}

void loop() {  // code that loops forever
  UpdateProjectTime();
  HandleBlinking();
  HandleRIPSensorState();
}

void HandleRIPSensorState() {
  String incomingData = CheckIncomingData();
  HandleRIPSensorInComingData(incomingData);

  switch (currentRIPSensorState) {
    case RIPSensorState::Idle: RIPSensorIdleState(); break;
    case RIPSensorState::RipSensorConnecting: RIPSensorConnectingState();
    case RIPSensorState::Calibration: RIPSensorCalibrationState(); break;
    case RIPSensorState::Active: RIPSensorActiveState(); break;
  }
}

void HandleRIPSensorInComingData(String incomingData) {

  switch (incomingData.toInt()) {
    //case RIPIdleStateToken: currentRIPSensorState = RIPSensorState::Idle; break;
    case RIPConnectingStateToken: currentRIPSensorState = RIPSensorState::RipSensorConnecting; break;
    case RIPCalibrationStateToken: currentRIPSensorState = RIPSensorState::Calibration; break;
    case RIPActiveStateToken: currentRIPSensorState = RIPSensorState::Active; break;
  }
}

void RIPSensorIdleState() {
  HandleBlinking();
}

void RIPSensorConnectingState() {
  ConnectionMode();
}

void RIPSensorCalibrationState() {
  //Serial.println("calibrate");
  CalibrateDevice();
}

void RIPSensorActiveState() {

  int time = currentTime - lastTimeSensorDataSent;
  if (time > sensorDataInterval) {
    int rawChestData = ReadChestData();
    chestFinalValue = ConvertFloatValueToNewScale(rawChestData, chestMinRaw, chestMaxRaw, 0.0f, 100.0f);
    if (chestFinalValue <= 0) {
      chestFinalValue = 0;
    } else if (chestFinalValue >= 100) {
      chestFinalValue = 100;
    }

    float displacement = chestFinalValue - chestPreviousValue;
    float passedTime = (sensorDataInterval * time) / 1000.0f;
    float speed = displacement / passedTime;

    if (speedCounter < collectedSpeedsLenght) {
      collectedSpeeds[speedCounter] = speed;
    }

    if (speedCounter >= collectedSpeedsLenght - 1) {
      inhaleExhaleSpeed = GetAvarageFromArray(collectedSpeeds, collectedSpeedsLenght);
      currentBreathingState = GetBreathingState(inhaleExhaleSpeed);
      // Serial.print(" | F : ");
      // Serial.print(chestFinalValue);
      // Serial.print(" | S : ");
      // Serial.print(inhaleExhaleSpeed);
      // Serial.print(" | State : ");
      // Serial.println(static_cast<int>(currentBreathingState));
       HandleSendingRIPSensorData();

      speedCounter = 0;
      chestPreviousValue = chestFinalValue;
    } else {
      speedCounter++;
    }

    lastTimeSensorDataSent = currentTime;
  }
  //Serial.println("-");
}

void AddVarToMessage() {
}

void SendMessage() {
}

void EmptyArray(float array[], int lenght) {
  for (int i = 0; i = lenght; i++) {
    array[i] = 0;
  }
}

float GetAvarageFromArray(float array[], int lenght) {

  float allSpeeds = 0;

  for (int i = 0; i < lenght; i++) {
    allSpeeds += array[i];
  }
  return allSpeeds / lenght;
}

void SetupBreathingLeds() {
  // Set LED pins as outputs. Use a for loop since there are a lot of pins.
  for (int pin = 0; pin < num_LEDs; pin++) {
    pinMode(chest_LED_pins[pin], OUTPUT);
    pinMode(belly_LED_pins[pin], OUTPUT);
  }
}

int ReadChestData() {
  int rawChestData = analogRead(chest_sensor_pin);
  return rawChestData;
}

void HandleBreathingLedFeedback() {
}

void CalibrateDevice() {
  // on enter //TODO make a statemachine a scriptBased
  if (firstEntered) {
    firstEntered = false;
    currentCalibrationState = CalibrationState::ChestMin;
    currentledBlinkInterval = ledCalibrationBlinkInterval;
    calibrationIsDone = false;
    abortCalibration = false;
  }

  // update state
  while (!calibrationIsDone) {
    UpdateProjectTime();
    HandleBlinking();

    String incomingData = CheckIncomingData();
    HandleCalibrationIncomingData(incomingData);
    UpdateCalibrationStateMachine();
  }
}

void HandleCalibrationIncomingData(String incomingData) {

  switch (incomingData.toInt()) {
    //case CalibrationDoneToken: calibrationIsDone = true; break;
    case abortCalibrationToken: abortCalibration = true; break;
    case nextStepToken: NextStep(); break;
    case previousStepToken: PreviousStep(); break;
    case restartCalibrationToken: currentCalibrationState = CalibrationState::ChestMin; break;
  }
}

void NextStep() {
  switch (currentCalibrationState) {
    case CalibrationState::Idle: currentCalibrationState = CalibrationState::ChestMin; break;
    case CalibrationState::ChestMin: currentCalibrationState = CalibrationState::ChestMax; break;
    case CalibrationState::ChestMax: currentCalibrationState = CalibrationState::Finished; break;
  }
}

void PreviousStep() {
  switch (currentCalibrationState) {
    case CalibrationState::ChestMin: currentCalibrationState = CalibrationState::Idle; break;
    case CalibrationState::ChestMax: currentCalibrationState = CalibrationState::ChestMin; break;
    case CalibrationState::Finished: currentCalibrationState = CalibrationState::ChestMax; break;
  }
}


void UpdateCalibrationStateMachine() {
  switch (currentCalibrationState) {
    case CalibrationState::Idle: CalibrationIdleState(); break;
    case CalibrationState::ChestMin: CalibrationChestMinState(); break;
    case CalibrationState::ChestMax: CalibrationChestMaxState(); break;
    case CalibrationState::Finished: CalibrationFinishedStateState(); break;
  }
}

void CalibrationIdleState() {
  //Serial.println("CIdle");
}


void CalibrationChestMinState() {
  chest_sensor_readingRaw = analogRead(chest_sensor_pin);
  chestMinRaw = chest_sensor_readingRaw;
  // Serial.print("Chest min : ");
  // Serial.println(chestMinRaw);
}

void CalibrationChestMaxState() {
  chest_sensor_readingRaw = analogRead(chest_sensor_pin);
  chestMaxRaw = chest_sensor_readingRaw;
  // Serial.print("Chest max : ");
  // Serial.println(chestMaxRaw);
}

void CalibrationFinishedStateState() {
  calibrationIsDone = true;
  firstEntered = true;
  currentCalibrationState = CalibrationState::Idle;
  currentRIPSensorState = RIPSensorState::Active;
  currentledBlinkInterval = ledConnectedBlinkInterval;

  // Serial.println("Calibration Finished");
  // Serial.print(" | Chest min : ");
  // Serial.print(chestMinRaw);
  // Serial.print(" | Chest max : ");
  // Serial.print(chestMaxRaw);
  // Serial.println(" ");
}



String CheckIncomingData() {
  if (Serial.available()) {
    String incomingData = Serial.readStringUntil('\n');
    //Serial.println("");
    //Serial.print("incomingData : ");
    //Serial.println(incomingData);
    return incomingData;
  } else {
    return "";
  }
}

void ConnectionMode() {
  int setupProgressDuration = 0;
  while (!isConnected) {
    UpdateProjectTime();
    String incommingMessage = CheckIncomingData();

    if (incommingMessage == "1") {
      // if connected device sends 1 break}
      currentledBlinkInterval = ledConnectedBlinkInterval;  // changign blink pattern
      setupProgressDuration = currentTime;
      isConnected = true;
      break;
    }

    HandleBlinking();
    HandleSendingConnectionToken();
  }
}
void ChangeLedStatus(bool value) {
  if (value == false) {
    digitalWrite(ledPin, LOW);
    ledStatus = false;
  } else {
    digitalWrite(ledPin, HIGH);
    ledStatus = true;
  }
}

void HandleBlinking() {
  //The blinking
  int blinkTime = currentTime - lastTimeBlinked;
  if (blinkTime >= currentledBlinkInterval) {
    if (ledStatus == true) {
      ChangeLedStatus(false);

    } else {
      ChangeLedStatus(true);
    }
    lastTimeBlinked = currentTime;
  }
}

void HandleSendingConnectionToken() {
  int connectionTokenTime = currentTime - lastTimeConnectionTokenSent;
  if (connectionTokenTime >= connectionTokenSendInterval) {
    Serial.println(connectionToken + valueSplitter + "1");  // sendingConnectionToken
    lastTimeConnectionTokenSent = currentTime;
    //Serial.println("blink");
  }
}

void HandleSendingRIPSensorData() {

  Serial.print(chestPositionVarName + valueSplitter);
  Serial.print(chestFinalValue);

  Serial.print(varSplitter);

  Serial.print(inhaleExhaleSpeedName + valueSplitter);
  Serial.print(inhaleExhaleSpeed);

  Serial.print(varSplitter);

  Serial.print(BreathingStateName + valueSplitter);
  Serial.println(static_cast<int>(currentBreathingState));
  lastTimeSensorDataSent = currentTime;
}

void HandleSendingRIPSensorParameters() {
}

BreathingState GetBreathingState(float speed) {

  if (speed < exhaleSpeedThreshold) {
    return BreathingState::Exhaling;

  } else if (speed > inhaleSpeedThreshold) {
    return BreathingState::Inhaling;

  } else {
    return BreathingState::HoldingBreath;
  }
}

void UpdateProjectTime() {
  currentTime = millis() - startTime;
}

static float ConvertFloatValueToNewScale(float currentValue, float aMin, float aMax, float bMin, float bMax) {
  float newValue;
  newValue = bMin + ((currentValue - aMin) * (bMax - bMin)) / (aMax - aMin);
  return newValue;
}