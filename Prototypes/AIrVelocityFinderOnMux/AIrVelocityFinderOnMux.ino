#include <Wire.h>
#include <SparkFun_FS3000_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_FS3000

#define MUX_Address 0x70  // TCA9548A Encoders address

FS3000 inhaleSensor;
FS3000 exhaleSensor;

uint8_t inhalePort;
uint8_t exhalePort;

const int muxLimit = 8;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  digitalWrite(13, LOW);

  Wire.begin();

  for (int i = 0; i < 8; i++) {
  tcaselect(i);
  delay(10);

  Serial.print("Port ");
  Serial.print(i);

  if (inhaleSensor.begin()) {
    Serial.println(" -> FOUND");
  } else {
    Serial.println(" -> none");
  }
}


  //inhaleSensorSetup();
  //exhaleSensorSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void inhaleSensorSetup() {
  Serial.println("-------------");
  Serial.println("Setup Inhale");

  for (int i = 0; i < muxLimit; i++) {
    tcaselect(i);
    delay(10);

    Serial.print("testing Port ");
    Serial.print(i);

    if (inhaleSensor.begin()) {
      inhalePort = i;
      Serial.println(" : connected");
      return;
    } else {
      Serial.println(" : couldn't connect");
    }
  }
}

void exhaleSensorSetup() {
  Serial.println("-------------");
  Serial.println("Setup exhale");

  for (int i = 0; i < muxLimit; i++) {

    if (i == inhalePort) continue;
    tcaselect(i);

    Serial.print("testing Port ");
    Serial.print(i);

    if (exhaleSensor.begin()) {
      exhalePort = i;
      Serial.println(" : connected");
      return;
    } else {
      Serial.println(" : couldn't connect");
      continue;
    }
  }
}



// Initialize I2C buses using TCA9548A I2C Multiplexer
void tcaselect(uint8_t i2c_bus) {
  if (i2c_bus > 7) return;
  Wire.beginTransmission(MUX_Address);
  Wire.write(1 << i2c_bus);
  Wire.endTransmission();
}