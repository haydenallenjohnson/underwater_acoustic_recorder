/*  Software for feather to turn Tascam DR40-X on and off with switch
 *  Uploaded 2023-07-18 12:10
 */

// include libraries for sleep mode
#include <RTCZero.h>
#include <ArduinoLowPower.h>

// define pins
#define LED_PIN 13
#define SET_PIN 15
#define UNSET_PIN 16
#define BUTTON_PIN 17
#define TASCAM_POWER_PIN 5

// define bytes corresponding to fake button presses
#define STOP 8 
#define PLAY 9
#define RECORD 11
#define ENTER 24

#define START_MASK  0x80
#define REPEAT_MASK 0xC0
#define END_MASK    0x00

bool tascamOn;
bool buttonOn;

void setup() {  
  // set relay pins to output
  pinMode(SET_PIN, OUTPUT);
  pinMode(UNSET_PIN, OUTPUT);
  
  // put relay in unset mode
  digitalWrite(UNSET_PIN, HIGH);
  delay(10);
  digitalWrite(UNSET_PIN, LOW);

  // turn off LED to save power
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // set button pin to input pullup
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // set tascam power pin to input
  pinMode(TASCAM_POWER_PIN, INPUT);

  // set up wakeup interupt when switch is set to on
  LowPower.attachInterruptWakeup(BUTTON_PIN, wakeUp, FALLING);

  // enter sleep mode
  LowPower.deepSleep();
}

void loop() {
  // input pullup defaults to high and is pulled low by button press
  // read button status; "on" if pulled low
  buttonOn = !digitalRead(BUTTON_PIN);
  
  // read tascam power status from 3.3V jack output to remote
  tascamOn = digitalRead(TASCAM_POWER_PIN);
  
  if (tascamOn == false && buttonOn == false) { 
    LowPower.deepSleep();
  } else if (tascamOn == false && buttonOn == true) {
    startRecording();
    delay(5000);
  } else if (tascamOn == true && buttonOn == false) {
    stopRecording();
    digitalWrite(LED_PIN, LOW);
    LowPower.deepSleep();
  } else if (tascamOn == true && buttonOn == true) {
    delay(1000);
  }
}

void startRecording() {
  // turn on tascam
  toggleTascamPower();

  // begin serial communication
  Serial1.begin(9600, SERIAL_8E1);
  delay(5000);  // serial comms take a few seconds to start working

  // press enter to accept USB power source
  Serial1.write(ENTER | START_MASK);
  delay(100);
  Serial1.write(ENTER | END_MASK);
  delay(5000);
  
  // press enter to accept phantom power on
  Serial1.write(ENTER | START_MASK);
  delay(100);
  Serial1.write(ENTER | END_MASK);
  delay(5000);
  
  // press record twice (once to enter record mode, then to start)
  Serial1.write(RECORD | START_MASK);
  delay(100);
  Serial1.write(RECORD | END_MASK);
  delay(5000);
  Serial1.write(RECORD | START_MASK);
  delay(100);
  Serial1.write(RECORD | END_MASK);
}

void stopRecording() {
  // press the stop button
  Serial1.write(STOP | START_MASK);
  delay(100);
  Serial1.write(STOP | END_MASK);

  // pause for dramatic effect
  delay(5000);

  // disable serial comms (so power doesn't leak through tascam)
  Serial1.end();
  delay(2000);

  // turn off tascam
  toggleTascamPower();
}

void toggleTascamPower() {
  digitalWrite(SET_PIN, HIGH);
  delay(10);
  digitalWrite(SET_PIN, LOW);
  delay(3000);
  digitalWrite(UNSET_PIN, HIGH);
  delay(10);
  digitalWrite(UNSET_PIN, LOW);
}

void wakeUp() {
  // rise and shine
  digitalWrite(LED_PIN, HIGH);
}
