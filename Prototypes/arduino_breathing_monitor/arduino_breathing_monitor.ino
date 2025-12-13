/*

Arduino chest + belly breathing monitor
Ben Finio
Science Buddies
Full project instructions, including parts list and circuit diagram, available at:
INSERT URL

*/

// Define pins for LEDs - store these in arrays since there are a lot of them. They are constant and won't change.
// If you follow the breadboard diagram on the Science Buddies website, you do not need to change these. 
const int num_LEDs = 5;  // number of LEDs for each sensor 
const int chest_LED_pins[] = {9,10,11,12,13}; // chest sensor LED pins in the order you want them to light up
const int belly_LED_pins[] = {8,7,6,5,4};     // belly sensor LED pins in the order you want them to light up

// Define pins for sensors and potentiometers. If you follow the breadboard diagram on the
// Science Buddies website, you do not need to change these. 
const int chest_sensor_pin = A0; 
const int chest_min_adjust_pin = A1;  
const int chest_max_adjust_pin = A2;    
const int belly_min_adjust_pin = A3;  
const int belly_max_adjust_pin = A4;  
const int belly_sensor_pin = A5;      

// Define variables for sensor and potentiometer readings. 
int chest_min_adjust_reading;   
int chest_max_adjust_reading;   
int chest_sensor_reading; 
int chest_sensor_readingAverage; 
      
int belly_min_adjust_reading;   
int belly_max_adjust_reading;   
int belly_sensor_reading;       

// Declare variables for the min and max analog-to-digital converter (ADC) values that can be adjusted
// using the potentiometers. This lets you calibrate the sensor and make adjustments.
int chest_ADC_min;
int chest_ADC_max; 
int belly_ADC_min;
int belly_ADC_max; 

// Declare variables to calculate the thresholds at which to turn on
// each LED for each sensor. 
int chest_delta;
int belly_delta;
int chest_thresholds[5];
int belly_thresholds[5];

// variables for use with serial plotter
int i = 0;
const int N = 50;
int plot1[N];
int plot2[N];
int plot3[N];
int plot4[N];

int counter = 0;
int readingInterval =6;

void setup() {  // code that only runs once
  // Set LED pins as outputs. Use a for loop since there are a lot of pins.
  for(int pin=0; pin<num_LEDs; pin++){
    pinMode(chest_LED_pins[pin],OUTPUT);
    pinMode(belly_LED_pins[pin],OUTPUT);  
  }

  // Initialize serial communication for debugging. Comment this out if it is not needed.
  // (Serial communication is slow and can slow your program down.)
  Serial.begin(9600);
}

void loop() {  // code that loops forever
  // Read all analog pins.
  chest_min_adjust_reading = analogRead(chest_min_adjust_pin);  
  chest_max_adjust_reading = analogRead(chest_max_adjust_pin);  
  chest_sensor_reading = analogRead(chest_sensor_pin);      
  chest_sensor_readingAverage += chest_sensor_reading;
  // belly_min_adjust_reading = analogRead(belly_min_adjust_pin);   
  // belly_max_adjust_reading = analogRead(belly_max_adjust_pin);   
  // belly_sensor_reading = analogRead(belly_sensor_pin);     

   /*Adjust minimum and maximum range for each sensor based
   on the potentiometer readings. By default, the readings
   remain unchanged as a value between 0-1023. This gives you
   very coarse adjustments. To get finer adjustments, change
   the final two numbers in each map function. For example,
   if you know your chest sensor reading tends to go between
   about 300 (relaxed) and 500 (stretched), you could change the first two
   lines of code to:
       chest_ADC_min = map(chest_min_adjust_reading,0,1023,200,400);
       chest_ADC_max = map(chest_max_adjust_reading,0,1023,400,600);
    This will allow you to make finer adjustments in the range of
    200-400 for the min and 400-600 for the max.
   */

   chest_ADC_min = map(chest_min_adjust_reading,0,1023,100,700);
   chest_ADC_max = map(chest_max_adjust_reading,0,1023,100,700);
   //belly_ADC_min = map(belly_min_adjust_reading,0,1023,100,700);
   //belly_ADC_max = map(belly_max_adjust_reading,0,1023,100,700);

   // Note: you could also add error checking here, to make
   // sure the minimum never exceeds the maximum. As written now,
   // the code will allow that. 

  /* 
   Calculate thresholds for turning LEDs on - evenly spaced
   between min and max ADC values for each sensor, based on
   the number of LEDs.
   For example, if your minimum ADC value is 400 and
   the maximum value is 1000, then:
   the first LED should light up when the ADC value exceeds 500
   the second LED should light up when the ADC value exceeds 600
   the third LED should light up when the ADC value exceeds 700
   the fourth LED should light up when the ADC value exceeds 800
   the fifth LED should light up when the ADC value exceeds 900
  */
  
  chest_delta = (chest_ADC_max - chest_ADC_min)/(num_LEDs+1);
  chest_thresholds[0] = chest_ADC_min+chest_delta;
  for(int i=1; i<num_LEDs; i++){
    chest_thresholds[i] = chest_thresholds[i-1] + chest_delta;    
  }

  // belly_delta = (belly_ADC_max - belly_ADC_min)/(num_LEDs+1);
  // belly_thresholds[0] = belly_ADC_min + belly_delta;
  // for(int i=1; i<num_LEDs; i++){
  //   belly_thresholds[i] = belly_thresholds[i-1] + belly_delta;    
  // }
  
  // // Compare sensor readings to the thresholds and turn on
  // // the appropriate LEDs for each sensor.
  // for(int i=0; i<num_LEDs; i++){
  //   if(chest_sensor_reading > chest_thresholds[i]){
  //     digitalWrite(chest_LED_pins[i],HIGH);      
  //   }
  //   else{
  //     digitalWrite(chest_LED_pins[i],LOW);
  //   }
  //   if(belly_sensor_reading > belly_thresholds[i]){
  //     digitalWrite(belly_LED_pins[i],HIGH);      
  //   }
  //   else{
  //     digitalWrite(belly_LED_pins[i],LOW);
  //   }
    
  // }

  // Print variables to serial monitor for debugging.
  // Comment this section out if not in use. 
  


  if(counter < readingInterval){
    counter++;
    return;
  }
  chest_sensor_readingAverage = chest_sensor_readingAverage / counter;
  

  Serial.print("Chest Sensor: ");
  Serial.print(chest_sensor_reading);
  Serial.print(" | chest_sensor_readingAverage ");
  Serial.println(chest_sensor_readingAverage);
  // Serial.print(" | Chest ADC min: ");
  // Serial.print(chest_ADC_min);
  // Serial.print(" | Chest ADC max: ");
  // Serial.println(chest_ADC_max);
  //Serial.print(" Belly Sensor: ");
  //Serial.print(belly_sensor_reading);
  //Serial.print(" | Belly ADC min: ");
  //Serial.print(belly_ADC_min);
  //Serial.print(" | Belly ADC max: ");
  //Serial.println(belly_ADC_max);
  
  counter = 0;
  chest_sensor_readingAverage = 0;

  }
