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


// mux cport onnection
const uint8_t leftPort = 1;
const uint8_t rightPort = 0;

// digital pins
const int ResetPin = 11;
const int ledBlueLeft = 12;
const int ledBlueRight = 13;

int currentCounter = 0;

int ledInterval = 100;
int lastLedChange;

FS3000 inhaleSensor;
FS3000 exhaleSensor;

bool inhaleSensorConnected;
bool exhaleSensorConnected;

uint8_t inhalePort = 0;
uint8_t exhalePort;

// time
unsigned long currentTime = 0;
unsigned long startTime = 0;




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

  startTime = millis();
  VelocitySensorsSetup();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  UpdateProjectTime();

  //SimpleLedStrip();
}


void UpdateProjectTime() {
  currentTime = millis() - startTime;
}


void VelocitySensorsSetup() {
  Serial.println("-------------");
  Serial.println("Setup Sensors");

  if (!inhaleSensorConnected) {
    // trying to coneect to sensor inhaling
    Serial.println("Inhaling");
    SwitchMuxPortTo(InhalingPort);
    if (!inhaleSensor.begin()) {
      inhaleSensorConnected = false;
    } else {
      inhaleSensorConnected = true;
      TurnOnLed(InhaleLed);
      Serial.println("connected");
    }
  }

  if (!exhaleSensorConnected) {
    Serial.println("Exhaling");
    SwitchMuxPortTo(ExhalingPort);

    if (!exhaleSensor.begin()) {
      exhaleSensorConnected = false;

    } else {
      exhaleSensorConnected = true;

      TurnOnLed(ExhaleLed);
      Serial.println("connected");
    }
  }

  if (!inhaleSensorConnected || !exhaleSensorConnected) {
  }
}

void TryConnectingToSensor(FS3000 &sensor, uint8_t &sensorPort, bool &isConnected) {
  if (!inhaleSensorConnected) {
    // trying to coneect to sensor inhaling
    Serial.println("Inhaling");
    SwitchMuxPortTo(InhalingPort);
    if (!inhaleSensor.begin()) {
      inhaleSensorConnected = false;
    } else {
      inhaleSensorConnected = true;
      TurnOnLed(ExhaleLed);
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

void TurnOnLed(int ledPin) {
  digitalWrite(ledPin, HIGH);
}

void TurnOffLed(int ledPin) {
  digitalWrite(ledPin, LOW);
}