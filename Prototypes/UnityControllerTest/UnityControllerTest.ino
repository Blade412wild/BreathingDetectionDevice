
const int ledPin = 11;

bool isConnected = false;
char emptyChar = ' ';

// led data
bool ledStatus = false;
int ledSetupBlinkInterval = 250;
int ledConnectedBlinkInterval = 1000;
int lastTimeBlinked = 0;
int currentledBlinkInterval = 0;

// sendingData
int connectionTokenSendInterval = 50;
int lastTimeConnectionTokenSent = 0;
const char connectionToken = '1';

// time
unsigned long currentTime = 0;
unsigned long startTime = 0;

int maxSetupProgress = 5000;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  startTime = millis();
  currentledBlinkInterval = ledSetupBlinkInterval;
  ConnectionMode();
}

void loop() {
  // put your main code here, to run repeatedly:
  UpdateProjectTime();
  HandleBlinking();
}

void ConnectionMode() {
  int setupProgressDuration = 0;
  while (!isConnected) {
    UpdateProjectTime();
    char cmd = CheckIncommingData();
    if (cmd == '1') {
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

char CheckIncommingData() {
  if (Serial.available()) {
    char incomingData = Serial.read();
    return incomingData;
  } else {
    return emptyChar;
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
    Serial.println(connectionToken);
    lastTimeConnectionTokenSent = currentTime;
    //Serial.println("blink");
  }
}

void UpdateProjectTime() {
  currentTime = millis() - startTime;
}