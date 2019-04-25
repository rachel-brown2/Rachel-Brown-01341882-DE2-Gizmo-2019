//Code based from 'MPR121 test' and 'PIR sensor tester' both arduino example's https://learn.adafruit.com/pir-passive-infrared-proximity-motion-sensor/using-a-pir-w-arduino http://gizmo.designengineering.london/en/latest/arduino/practice-combined.html 
// Include all necessary external moduels
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Servo.h>
#include <OLED_I2C.h>
#include <DS3231.h>

#define _BV
#define _BV(bit) (1 << (bit))

// TOUCH SENSOR START
// You can have up to 4 on one i2c bus
Adafruit_MPR121 cap = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
//intate the values of the inputs
int Valueled = 0;
int temp = 999;
// TOUCH SENSOR END

// SERVO START
Servo myservo;
Servo myservo1;
Servo myservo2;
int pos = 0;
// SERVO END

// MOTION SENSOR START
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 5000;

boolean lockLow = true;
boolean takeLowTime;

int pirPin = 3;    //the digital pin connected to the PIR sensor's output
//int ledPin = 12;
// MOTION SENSOR END

// OLED START
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];
// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
OLED myOLED(SDA, SCL);
Time  t;
int i;
//OLED END


void setup() {


  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);

  // SERVO START
  myservo.attach(9);
  myservo1.attach(5);
  myservo2.attach(6);
  myservo.write(0); // tells servo to go to position in the brackets
  myservo1.write(0);
  myservo2.write(0);
  // SERVO END

  while (!Serial) { // needed to keep the arduino from starting too fast
    delay(10);
  }
//MOTION SENSOR START
  //give the sensor some time to calibrate
  Serial.print("calibrating sensor "); // serial prints its calibrating 
  for (int i = 0; i < calibrationTime; i++) {
    Serial.print("."); // it prints a . every second 
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);
//MOTION SENSOR END

//TIME DISPLAY CODE START
  // Initialize the rtc object
  rtc.begin();
  rtc.setDOW(SUNDAY);     // Day-of-Week is set to sunday
  rtc.setTime(11, 00, 0);     // Time is set to 11:00:00 
  rtc.setDate(22, 3 , 2019);   // Date to March 22nd, 2019

  if (!myOLED.begin(SSD1306_128X64))
    while (1);  // In case the library failed to allocate enough RAM for the display buffer...

  myOLED.setFont(SmallFont);
//TIME DISPLAY CODE END

// TOUCH SENSOR CODE START
// Display's in serial that the arduino is looking for the touch sensor
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  //Serial dipsays whether the MPR121 touch sensor moduel has been found or not
//TOUCH SENSOR CODE END



}
void loop() {

  // ======= TOUCH SENSOR =======
  // This section detects if the flower petal has been touched. If it has, it will move the servo and display a message on the OLED. 
  
  currtouched = cap.touched();

  if ((currtouched & _BV(1)) && !(lasttouched & _BV(1)) ) {    // if touched
    Serial.println(" touched");                                // prints that it has been touched in serial 
    for (pos = 0; pos < 180; pos += 1) {                       // moves both servos from 0 degrees to 180 degrees, in steps of 1 degree
      myservo.write(pos);                                      // servo to move to position in variable 'pos' 
      myservo1.write(pos);
      delay(15);                                               // waits 15ms for the servo to reach the position, so it doesn't move too quickly
    }
    for (pos = 180; pos > 0; pos -= 1) {                       // moves both servos from 180 degrees to 0 degrees, in steps of 1 degree
      myservo.write(pos);                                      // servo to move to position in variable 'pos'
      myservo1.write(pos);
      delay(15);                                               // waits 15ms for the servo to reach the position, so it doesn't move too quickly
    }

  
    myOLED.clrScr();                                           // After the flowers have moved a finishing message will display on the OLED saying Heya my name is Bloss
    myOLED.setFont(SmallFont);
    myOLED.print("Heya my name is Bloss", CENTER, 24);
    myOLED.update();
    delay (5000);
    myOLED.clrScr();
  
  }
  
  if (!(currtouched & _BV(1)) && (lasttouched & _BV(1)) ) {    // If untouched, reset the index
    Serial.println(" released");
  }
  // resetting the MPR121's state
  lasttouched = currtouched;
  // Delay so it isn't overwhelming
  delay(100);
  
  // ======= TIME DISPLAY CODE ======= 
  // This updates the time from the RTC, ready to be displayed on the OLED

  t = rtc.getTime();                                           // ds3231 sends the time to the OLED every second
  Serial.print(rtc.getDOWStr());                               //print the day of the week in serial
  Serial.print(" ");
  Serial.print(rtc.getDateStr());                              //print the date in serial 
  Serial.print(" -- ");
  Serial.print(rtc.getTimeStr());                              //print the time in serial    

  myOLED.setFont(MediumNumbers);                               // Format the OLED, and display the time on it
  myOLED.print(rtc.getDateStr(), RIGHT, 0);
  myOLED.setFont(BigNumbers);
  myOLED.printNumI(t.sec, RIGHT, 40, 2, '0');
  myOLED.printNumI(t.hour, LEFT, 40, 2, '0');
  myOLED.printNumI(t.min, CENTER, 40, 2, '0');
  myOLED.update();

  delay(1000);

  // ======= MOTION DETECTION CODE ======= 
  // Check the state of the PIR Sensor reading. If motion is detected, OLED screen will wake up; if not detected then OLED will set to sleep
  
  if (digitalRead(pirPin) == HIGH) {                            // If motion is detected
    myOLED.sleepMode(SLEEP_OFF);                                // OLED screen is set to SLEEP_OFF - turning on the display
    if (lockLow) {                                              //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;
      Serial.println("---");
      Serial.print("motion detected at ");
      Serial.print(millis() / 1000);
      Serial.println(" sec");
      delay(50);
    }
    takeLowTime = true;
  }

  if (digitalRead(pirPin) == LOW) {                             // If motion is detected
    
    if (takeLowTime) {
      lowIn = millis();                                         // save the time of the transition from high to LOW
      takeLowTime = false;                                      // make sure this is only done at the start of a LOW phase
    }    
    
    if (!lockLow && millis() - lowIn > pause) {                 //if the sensor is low for more than the given pause, we assume that no more motion is going to happen, hence turning
      myOLED.sleepMode(SLEEP_ON);                               // OLED screen is set to SLEEP
      lockLow = true;
      Serial.print("motion ended at ");      //output
      Serial.print((millis() - pause) / 1000);
      Serial.println(" sec");
      delay(50);
    }
  }
}
