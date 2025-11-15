/******************************************************************************
  Example_01_BasicReadings.ino

  Read values of air velocity from the FS3000 sensor, print them to terminal.
  Prints raw data, m/s and mph.
  Note, the response time on the sensor is 125ms.

  SparkFun FS3000 Arduino Library
  Pete Lewis @ SparkFun Electronics
  Original Creation Date: August 5th, 2021
  https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library

  Development environment specifics:

  IDE: Arduino 1.8.15
  Hardware Platform: SparkFun RedBoard Qwiic
  SparkFun Air Velocity Sensor Breakout - FS3000 (Qwiic) Version: 1.0

  Artemis RedBoard @ 400KHz (Core v2.1.0) 
  (note, v2.1.1 has a known issue with clock stretching at 100KHz)  

  Do you like this library? Help support SparkFun. Buy a board!

    SparkFun Air Velocity Sensor Breakout - FS3000-1005 (Qwiic)
    https://www.sparkfun.com/products/18377

    SparkFun Air Velocity Sensor Breakout - FS3000-1015 (Qwiic)
    https://www.sparkfun.com/products/18768

  Hardware Connections:
  Use a Qwiic cable to connect from the RedBoard Qwiic to the FS3000 breakout (QWIIC).
  You can also choose to wire up the connections using the header pins like so:

  ARDUINO --> FS3000
  SDA (A4) --> SDA
  SCL (A5) --> SCL
  3.3V --> 3.3V
  GND --> GND

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

//V1.0

#include <Wire.h>
#include <SparkFun_FS3000_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_FS3000


FS3000 fs;

const int ledPin = 11;

bool isConnected = false;
char emptyChar = ' ';

// led data
bool ledStatus = false;
int ledSetupBlinkInterval = 250;
int ledConnectedBlinkInterval = 1000;
int lastTimeBlinked = 0;
int currentledBlinkInterval = 0;

// sendingData Setup
int connectionTokenSendInterval = 50;
int lastTimeConnectionTokenSent = 0;
const String connectionToken = "-1";

// sendingData SensorData
int sensorDataInterval = 125;  //note, reponse time on the sensor is 125ms
int lastTimeSensorDataSent = 0;
String SendingData = "";
String valueSplitter = ":";
String variableNameSplitter = "|";
const String AirVelocityName = "3";
const String inExhaleSpeedName = "4";
const String ExHalingThroughNoseName = "2";
const String BreathingStateName = "1";

//sensor
float airVelocity;


// time
unsigned long currentTime = 0;
unsigned long startTime = 0;




int maxSetupProgress = 5000;

void setup() {
  Serial.begin(9600);
  SetupSensor();

  startTime = millis();
  currentledBlinkInterval = ledSetupBlinkInterval;
  ConnectionMode();
}

void loop() {
  UpdateProjectTime();
  HandleBlinking();
  HandleSendingSensorData();
}

void SetupSensor() {
  Wire.begin();

  if (fs.begin() == false)  //Begin communication over I2C
  {
    //Serial.println("The sensor did not respond. Please check wiring.");
    while (1)
      ;  //Freeze
  }

  fs.setRange(AIRFLOW_RANGE_15_MPS);
  //Serial.println("Sensor is connected properly.");
}

void ConnectionMode() {
  int setupProgressDuration = 0;
  while (!isConnected) {
    UpdateProjectTime();
    String incommingMessage = CheckIncommingData();

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

String CheckIncommingData() {
  if (Serial.available()) {
    String incomingData = Serial.readStringUntil('\n');
    return incomingData;
  } else {
    return "";
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

void HandleSendingSensorData() {
  int time = currentTime - lastTimeSensorDataSent;
  if (time > sensorDataInterval) {
    airVelocity = fs.readMetersPerSecond();
    Serial.print(AirVelocityName + valueSplitter);
    Serial.println(airVelocity);
    //Serial.println("Tick");
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

void UpdateProjectTime() {
  currentTime = millis() - startTime;
}
