/*
I2C ArduinoHead: 

MUX board Connections
MUX 0 = RightPort
MUX 1 = LeftPort

digitalPins:
pin 11 : reset I2C MuxBoard LOW = Reset
pin 12 : Led Left
pin 13 : Led Right

*/

#include <Wire.h>
#include <SparkFun_FS3000_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_FS3000

#define MUX_Address 0x70  // TCA9548A Encoders address
#define InhalingPort rightPort
#define ExhalingPort leftPort
#define ExhaleLed ledBlueRight
#define InhaleLed ledBlueLeft


//-------------------------------- BoardSetup

// mux cport onnection
const uint8_t leftPort = 1;
const uint8_t rightPort = 0;

// digital pins
const uint8_t ResetPin = 11;
const uint8_t ledBlueLeft = 12;
const uint8_t ledBlueRight = 13;

uint8_t ledInterval = 100;
uint8_t lastLedChange;

uint8_t inhalePort = 0;
uint8_t exhalePort;


//-------------------------------- Communication

// sendingData Setup
int connectionTokenSendInterval = 50;
int lastTimeConnectionTokenSent = 0;
const String connectionToken = "-1";

// Parsing/Sending rules
String SendingData = "";
String valueSplitter = ":";
String variableNameSplitter = "|";

// variables names
const String breathingStateName = "1";
const String In_ExhaleVelocityName = "2";

int sensorDataInterval = 125;  //note, reponse time on the sensor is 125ms
int lastTimeSensorDataSent = 0;
bool isConnected = false;
char emptyChar = ' ';

//-------------------------------- Sensor

// Sensors
FS3000 inhaleSensor;
FS3000 exhaleSensor;

bool inhaleSensorConnected;
bool exhaleSensorConnected;

float inhaleVelocity;
float previousInhaleVelocity;

float exhaleVelocity;
float previousExhaleVelocity;

//-------------------------------- overig

int breathingState = 1;  // 0 = inhaling, 1 = holdingBreath, 2 = exhaling

// time
unsigned long currentTime = 0;
unsigned long startTime = 0;

int currentCounter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

  startTime = millis();
  VelocitySensorsSetup();
  ConnectionMode();
}

void loop() {
  // put your main code here, to run repeatedly:
  UpdateProjectTime();
  HandleSendingSensorData();
}


void UpdateProjectTime() {
  currentTime = millis() - startTime;
}


void VelocitySensorsSetup() {
  //Serial.println("-------------");
  //Serial.println("Setup Sensors");

  //Serial.println("----- inhaling");
  TryConnectingToSensor(inhaleSensor, InhalingPort, inhaleSensorConnected, InhaleLed);

  //Serial.println("----- exhaling");
  TryConnectingToSensor(exhaleSensor, ExhalingPort, exhaleSensorConnected, ExhaleLed);
}

void TryConnectingToSensor(FS3000 &sensor, uint8_t sensorPort, bool &isConnected, uint8_t ledPin) {

  if (!isConnected) {
    SwitchMuxPortTo(sensorPort);

    if (!sensor.begin()) {
      inhaleSensorConnected = false;
    } else {
      sensor.setRange(AIRFLOW_RANGE_15_MPS);
      isConnected = true;
      TurnOnLed(ledPin);
      //Serial.println("connected");
    }
  }
}

void SwitchMuxPortTo(uint8_t port) {
  tcaselect(port);
}

// Initialize I2C buses using TCA9548A I2C Multiplexer
void tcaselect(uint8_t i2c_bus) {
  if (i2c_bus > 7) return;
  Wire.beginTransmission(MUX_Address);
  Wire.write(1 << i2c_bus);
  Wire.endTransmission();
}

void TurnOnLed(uint8_t ledPin) {
  digitalWrite(ledPin, HIGH);
}

void TurnOffLed(uint8_t ledPin) {
  digitalWrite(ledPin, LOW);
}

void ConnectionMode() {
  int setupProgressDuration = 0;
  while (!isConnected) {
    UpdateProjectTime();
    String incommingMessage = CheckIncommingData();

    if (incommingMessage == "1") {
      // if connected device sends 1 break}
      setupProgressDuration = currentTime;
      isConnected = true;
      break;
    }

    HandleSendingConnectionToken();
  }
}

String CheckIncommingData() {
  if (Serial.available()) {
    String incomingData = Serial.readStringUntil('\n');
    return incomingData;
  } else {
    return "";
  }
}

void HandleSendingSensorData() {
  int time = currentTime - lastTimeSensorDataSent;
  String message = "";
  if (time > sensorDataInterval) {


    if (inhaleSensorConnected) {
      inhaleVelocity = inhaleSensor.readMetersPerSecond();
      if (inhaleVelocity < 1) {
        inhaleVelocity = 0;
      }

      message = In_ExhaleVelocityName + valueSplitter + inhaleVelocity;
      Serial.println(message);
    }

    /*
    if (inhaleSensorConnected && exhaleSensorConnected) {
      message += variableNameSplitter;
    }
    */


    if (exhaleSensorConnected) {
      exhaleVelocity = exhaleSensor.readMetersPerSecond();
      if (exhaleVelocity < 1) {
        exhaleVelocity = 0;
      }
      message = In_ExhaleVelocityName + valueSplitter + exhaleVelocity;
      Serial.println(message);
    }

    /*
    if (inhaleSensorConnected && exhaleSensorConnected) {
      DecideBreathingState();
      Serial.println(message);
    }
*/

    lastTimeSensorDataSent = currentTime;
    return;
  }
  //Serial.println("-");
}

void HandleSendingConnectionToken() {
  int connectionTokenTime = currentTime - lastTimeConnectionTokenSent;
  if (connectionTokenTime >= connectionTokenSendInterval) {
    Serial.println(connectionToken + valueSplitter + "1");  // sendingConnectionToken
    lastTimeConnectionTokenSent = currentTime;
    //Serial.println("blink");
  }
}

void ReadSensors() {

  inhaleVelocity = inhaleSensor.readMetersPerSecond();
  exhaleVelocity = exhaleSensor.readMetersPerSecond();
}

void DecideBreathingState() {
  if (inhaleVelocity == 0 && exhaleVelocity == 0) {
    // is holding breath
    breathingState = 2;
    return;
  }

  if (inhaleVelocity != previousInhaleVelocity && exhaleVelocity == 0) {
    // user is inhaling
    breathingState = 0;
    return;
  }

  if (exhaleVelocity != previousExhaleVelocity && inhaleVelocity == 0) {
    // user is exhaling
    breathingState = 1;
    return;
  }
}