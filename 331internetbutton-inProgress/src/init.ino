#include "InternetButton.h"

InternetButton b = InternetButton();
int ledPos = 1;

void setup() {
    b.begin();
    b.setBrightness(95);

}

void loop(){
    b.ledOn(ledPos, 0, 0, 0);
    //b.playNote("G3",8);
    delay(330);
  //  b.playNote("G3",8);
    b.ledOn(ledPos, 0, 30, 30);
    delay(330);
  //  b.playNote("G3",8);
    b.ledOff(ledPos);
    delay(1000);
  //  b.playNote("G3",8);
}
