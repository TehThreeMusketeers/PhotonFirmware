// You know the routine by now, just going to comment on the new things!

#include "InternetButton.h"
#include <Particle.h>

// InternetButton Library
InternetButton ioButton = InternetButton();

// buttons conditions
uint8_t button1 = 0;
uint8_t button2 = 0;
uint8_t button3 = 0;
uint8_t button4 = 0;
uint8_t buttonAll = 0;

// Fancy LED animation
float ledPos = 1;
float ledDesired = 3;
uint8_t ledPosAchieved = 0;

bool isAlarmRinging = false;

void setup() {
    // register the cloud function
    Particle.function("soundAlarm", soundAlarm);
    Particle.function("stopAlarm", stopAlarm);

    // starts button and plays tune to show everything is right
    ioButton.begin();

    // Get brightness
    uint8_t bright = ioButton.getBrightness();

    // Set brightness
    ioButton.setBrightness(bright); // 0 - 255 (default = 255)

    // Get BPM
    int bpm = ioButton.getBPM(); // default is 250

    // Set BPM
    ioButton.setBPM(bpm);

    // Set number of LEDs if desired (11 by default)
    ioButton.setNumLeds(11);

    //Plays starting tune
    ioButton.playSong("C4,8,E4,8,G4,8,C5,8,G5,4,G5,8,C5,8,G4,8,E4,8,C5,4");
}

void loop() {
    // keep ringing until turned off
    if (isAlarmRinging) {
      soundAlarm("");
    }else{
      stopAlarm("");
    }

    // User is about to leave so arm
    if(ioButton.allButtonsOn()){
        if (!buttonAll) {
          buttonAll = 1;
          ioButton.rainbow(10);
          arm(10);
        }
    }
    else {buttonAll = 0;}

    // User is home but whishes to arm the system
    if(ioButton.buttonOn(2)){
      if (!button2) {
        button2 = 1;
        arm(1);
        ledControl(3);
      }
    }
    else {button2 = 0;}

    // User wants to disarm the system
    if(ioButton.buttonOn(4)){
      if (!button4) {
        button4 = 1;
        disarm();
        ledControl(9);
      }
    }
    else {button4 = 0;}

    // This set of conditionals animates the moving LEDs
    if(abs(ledPos - ledDesired) > .001){
        ledPos = ledPos - (ledPos - ledDesired)/300;
        ioButton.allLedsOff();

        // What's this hidden in here? Oh my, a function that takes
        // the position as a float! Smooth positions from 0-12!
        ioButton.smoothLedOn(ledPos, 0, 60, 60);
    }
    else if(ledPosAchieved == 1){
        ioButton.allLedsOff();
        ledPosAchieved = 2;
    }
    else if(ledPosAchieved != 2){
        ledPosAchieved = 1;
    }
}

// soudns alarm
int soundAlarm(String nonse) {
  ioButton.allLedsOn(150,0,0);
  ioButton.playSong("E5,3,G5,3,E6,3,C6,3,D6,3,G6,3");
  Particle.publish("alarmRinging",NULL, 60, PUBLIC);
  ioButton.allLedsOff();
  isAlarmRinging = true;
  return 1;
}

int stopAlarm(String nonse) {
  isAlarmRinging = false;
  return 1;
}

// plays arm tune
void arm(int duration) {
  String song = "";
  if (duration > 1) {
    song = "C4,3,E4,3,G4,3,C5,3,G5,3";
  } else{
    song = "C4,8,E4,8,G4,8,C5,8,G5,4";
  }
  for (size_t i = 0; i < duration; i++) {
    ioButton.playSong(song);
  }
}

// plays disarm tune
void disarm() {
  ioButton.playSong("G5,8,C5,8,G4,8,E4,8,C5,4");
}

// fancy LED animation
void ledControl(int ledn){
    ioButton.ledOn(ledn,60,60,60);
    delay(200);
    ledDesired = ledn;
    if(ledPos < ledDesired){ledDesired++;}
    else{ledDesired--;}
    ledPosAchieved = 0;
}