// Watch The Clock, a final project for Yale cpsc334
// by Lukas Burger 12/15/2019

#include <Stepper.h>

// set current time at program upload
// This needs to be changed for every installation
#define start_hour 2
#define start_mins 31

const int stepsPerRevolution = 32;  // specific for my motor

// initialize the stepper library on pins 15 through 4:
Stepper myStepper(stepsPerRevolution, 15,0,2,4);

// pin declarations
int ultra_power = 14;
int trigPin = 13;
int echoPin = 12;
int button_pin = 16;

// calibration variables
int steps_from_midnight = 0;
int hour_step_size = 0;
int step_size = 15;

int hour_pos = 0;
long dist;

void setup() {
  
  // set the speed and serial speed
  myStepper.setSpeed(640);
  Serial.begin(9600);

  //  set pinmodes
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(ultra_power, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // run initial calibration
  setMidnight();
  setHourSize();
}

// Set stepper position of 12 and reset steps_from_midnight
void setMidnight() {
  int i = 0;
  while(digitalRead(button_pin) == LOW);
  while(digitalRead(button_pin) == HIGH) {
      Serial.println("calibrating midnight " + String(i));
      i++;
      myStepper.step(step_size);
      delay(50);
  }
  steps_from_midnight = 0;
}

// Set steps per hour by doing full revolution and dividing by 12
void setHourSize() {
  int i = 0;
  while(digitalRead(button_pin) == LOW);
  while(digitalRead(button_pin) == HIGH) {
      Serial.println("calibrating hour size " + String(i));
      i++;
      myStepper.step(step_size);
      delay(50);
  }
  while(digitalRead(button_pin) == LOW);
  hour_step_size = i / 12;
  hour_pos = 12;
}

// calculate stepper distance between hours
int hourDiff(int h1, int h2, int dir) {
  if (dir == 1) {

    if (h1 > h2) {
      return h1 - h2;
    } else {
      return (12 - h2) + h1;
    }
    
  } else if (dir == -1) {

    if (h1 < h2) {
      return h2 - h1;
    } else {
      return (12 - h1) + h2;
    }
  }
}

// move clock hand to specific hour in a given direction
void goToHour(int target_pos, int dir) {
  int diff = hourDiff(target_pos, hour_pos, dir);

  if (dir == 1) {

    if (target_pos > hour_pos) {
      diff = target_pos - hour_pos;
    } else {
      diff = (12 - hour_pos) + target_pos;
    }
    
  } else if (dir == -1) {

    if (target_pos < hour_pos) {
      diff = hour_pos - target_pos;
    } else {
      diff = (12 - target_pos) + hour_pos;
    }
    
  }

  myStepper.step(dir * diff * hour_step_size * step_size);
  hour_pos = target_pos;
  delay(10);
}

// tick forward then background across 12 (like a metronome)
void tickPattern(int count) {
  int h = (count % 5) + 1;
  goToHour(h, 1);
  delay(500);
  goToHour(12 - h, -1);
  delay(500);
}

// cycle through four corners of the clock
void quartersPattern(int count) {
    int h = ((count % 4)*3) + 3;
    goToHour(h, 1);
    delay(1000);
}

// move two hours forward and then one hour back
void ratchetPattern() {
    int first_pos = hour_pos;
    goToHour(first_pos + 2, 1);
    delay(10);
    goToHour(first_pos + 1, -1);
    delay(10);
}

// spin backwards
void backSpin() {
    goToHour(12, -1);
    delay(10);
    goToHour(11, -1);
    delay(10);
}

// go to random hours
void setRandom() {
  goToHour(random(1, 13), random(-1, 2));
  delay(10);
}

// set clock time to real calculated time
void setClockTime() {

  int nextHour = calculateHour();
  if (nextHour != hour_pos) {
    int dir = 1;
    if (hourDiff(nextHour, hour_pos, -1) < hourDiff(nextHour, hour_pos, -1)) {
      dir = -1;
    }
    goToHour(nextHour, dir);
  }  
}

void checkCalibrate() {
  if(digitalRead(button_pin) == LOW) {
    setMidnight();
    setHourSize();
  }
}

// loop variables
int count = 0;
int nextPattern = 0;
int pattern = 0;
void loop() {

  // check to see if button is pressed and calibrate if so
  checkCalibrate();
  // measure distance from ultrasonic sensor
  dist = SonarSensor(trigPin, echoPin);

  //  if distance short (someone standing in front) set clock to real time
  if (dist < 2000) {
    Serial.println("Dist: " + String(dist) + " Using clock time");
    setClockTime();
    nextPattern = count;
    delay(2000);
  } else {
    //  else update the current pattern at random intervals
    if (count >= nextPattern) {
      pattern = random(0, 5);
      nextPattern = count + random(6, 30);
      Serial.println("Dist: " + String(dist) + " Pattern #: " + String(pattern) + " Duration: " + String(nextPattern-count));
    }

    // set pattern to be displayed
    switch (pattern) {
      case 0:
        setRandom();
        break;
      case 1:
        quartersPattern(count);
        break;
      case 2:
        tickPattern(count);
        break;
      case 3:
        backSpin();
        break;
      case 4:
        ratchetPattern();
        break;
    }
  }

  delay(10);

  count++;
}

// function to read from ultrasonic sensor
int SonarSensor(int trigPin,int echoPin) {
  digitalWrite(ultra_power, HIGH);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH)*0.034/2;
}

// calculate the current hour given millis() and start times defined at upload
int calculateHour() {
  long milliseconds = millis();
  long elapsed_mins = (milliseconds / (1000*60) % 60);
  long elapsed_hours = milliseconds / (1000*60*60);

  int current_hour = start_hour + elapsed_hours;
  if (elapsed_mins + start_mins >= 60) {
    current_hour += 1;  
  }
  return current_hour;
}
