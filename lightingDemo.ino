//Dental Heater Control Code
//Michael Crinnion - Crin Makes Stuff - 2021

//Don't change anything in this section!//
#include <FastLED.h>                    //
#define ledNum 16                       //
#define ledControl 6                    //
#define button 2                        //
CRGB leds[ledNum];                      //
int mode = 0;                           //
bool once = 0;                          //
volatile int buttonState = 0;           //
volatile int changeModeFlag = 0;        //
unsigned long longPressStart = 0;       //
long nextColour;                        //
bool longPressFlag = 0;                 //


///////////////////////////
//   Adjustable Timings  //
///////////////////////////

int wheelSpeed = 50; //Decrease this value to increase spin speed, time in milliseconds between LED forward movements.
int wheelContrast = 15;
int pulseSpeed = 40; //Decrease this value to increase fade up and down speed, time in milliseconds between brightness changes
int pulseDepth = 10; //Increase to dip closer to black during pulse cycles
int blendSpeed = 400; //Speed colours blend into one another when switching modes, higher numbers are faster
unsigned long standbyDelay = 3; //Number of seconds delay on long press before shutdown.

////////////////////////////
//   Adjustable Colours   //
////////////////////////////

//Use hex colour values. Any standard HTML hex colour can be subsitutued here.
//Eg. in HTML a colour value such as #10808b, 10808b is the hex value and should be added following an 0x such as 0x10808b
//Use https://htmlcolorcodes.com/ or any other HEX enabled colour picker
//Colours won't exactly match how they appear on a monitor

int masterBrightness = 255; //Value between 0 and 255 for master brightness of LEDS, scales all animations.
long fadeToCol = 0x000000; //The colour which wait modes transition to and from as they fade in and out
long hexColStandby = 0x4c00ff; //Standby colour
long hexCol40 = 0x00ffd0; //The colour that indicates 40°C, used for both the heat and wait modes
long hexCol50 = 0xff3c00; //The colour that indicates 50°C, used for both the heat and wait modes
long hexCol60 = 0xff0044; //The colour that indicates 60°C, used for both the heat and wait modes

long colour [4] = {hexColStandby, hexCol40, hexCol50, hexCol60}; //Colour array


void setup() {
  FastLED.addLeds<NEOPIXEL, ledControl>(leds, ledNum);
  FastLED.setBrightness(masterBrightness);
  pinMode(ledControl, OUTPUT);
  pinMode(button, INPUT);
  attachInterrupt(digitalPinToInterrupt(button), ISR_buttonChange, CHANGE);
  Serial.begin(115200);
  Serial.println("Starting up...");
}

void ISR_buttonChange() {
  if (digitalRead(button)==HIGH)
  {
    buttonState = 1;
  } else {
    buttonState = 0;
    longPressStart = 0;
    longPressFlag = 0;
    once = 0;
  }
}

void standby(long colour){
  while(buttonState != 1) {
    if(buttonState==1) {break;}
    if(once==0) {
      for (int i = 0; i < ledNum; ++i) {
        if(buttonState==1) {break;}
        leds[i] = CRGB(colour);
        FastLED.show();
      }
      once=1;
    }
    for(int i=0; i<20; i++) {
      if(buttonState==1) {break;}
      for (int j = 0; j < ledNum; ++j) {
        leds[j] = blend(leds[j], fadeToCol, pulseDepth);
      }
      FastLED.delay(pulseSpeed);
      FastLED.show();
    }
    for(int i=0; i<20; i++) {
      if(buttonState==1) {break;}
      for (int j = 0; j < ledNum; ++j) {
        if(buttonState==1) {break;}
        leds[j] = blend(leds[j], colour, pulseDepth);
      }
      FastLED.delay(pulseSpeed);
      FastLED.show();
    }
  }
}

void heating(long colour){
  int temporaryCounter = 0;
  while(buttonState != 1 && temporaryCounter < 10) { //'&& temporaryCounter < 10' is temporary code until thermostat code is written, it just counts n loops of the spinner then moves to standby
    for (int i = 0; i < ledNum; ++i) {
      if(buttonState==1) {break;}
      leds[i] = blend(leds[i], colour, blendSpeed);
      FastLED.delay(wheelSpeed);
      for(int j=0; j<ledNum; j++) {
        if(buttonState==1) {break;}
        leds[j].fadeToBlackBy(wheelContrast);
      }
      FastLED.show();
    }
    ++temporaryCounter;
  }
  standby(colour);
}

void buttonReaction(){
  for (int i = 0; i < ledNum; ++i) {
    leds[i] = blend(leds[i], nextColour, 500);
    FastLED.show();
    FastLED.delay(1);
  }
  if (longPressFlag==0 && buttonState==1)
  {
    longPressStart = millis();
    longPressFlag = 1;
  }

  if (buttonState == 1 && (millis() - longPressStart) > (standbyDelay*1000)) {
    Serial.println("Shutdown");
    longPressFlag = 0;
    mode = 0;
    changeModeFlag = 0;
    while(buttonState==1) {
      for (int i = 0; i < ledNum; ++i) {
        leds[i] = blend(leds[i], hexColStandby, 500);
        FastLED.show();
        FastLED.delay(1);
      }
    }
  }
}

void loop() {
  if (buttonState==1)
  {
    changeModeFlag=1;
    buttonReaction();
  } else {
    if (changeModeFlag==1) {
      if (mode < 3) {
        mode++;
        Serial.println("Moved forward one mode");
      } else {
        Serial.println("Moving back to start of modes");
        mode = 1;
      }
      changeModeFlag=0;
    }
    switch (mode) {
    case 0:
      Serial.println("Cold Standby");
      standby(colour[mode]);
      nextColour = colour[1];
      break;
    case 1:
      Serial.println("Heating to 40");
      nextColour = colour[2];
      heating(colour[mode]);
      break;
    case 2:
      Serial.println("Heating to 50");
      nextColour = colour[3];
      heating(colour[mode]);
      break;
    case 3:
      Serial.println("Heating to 60");
      nextColour = colour[1];
      heating(colour[mode]);
      break;
    }
  }
}