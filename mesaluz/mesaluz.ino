
#include <IRremote.h>
#include "FastLED.h"

FASTLED_USING_NAMESPACE


#define RECV_PIN 11
#define NEO_PIN  6 
#define NEO_LEDS 300

#define MODE_COLOR 0
#define MODE_WHITE 1
#define MODE_DEMO 2

#define CHARGER  5000

CRGB leds[NEO_LEDS];

int HUE = 0;
int SAT = 255;
int BR = 127;

int BR_STEP=5;
int HUE_STEP=5;
int SAT_STEP=10;

long command;
long lastCommand;
boolean repeatCommand;

int currentMode;
boolean on=true;


#define VOL_UP 0xFF906F
#define VOL_DN 0xFFA857
#define PT_UP 0xFFC23D
#define PT_DN 0xFF02FD
#define MODE_SWITCH 0xFF629D
#define MODE_ON_OFF 0xFFA25D

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {



  FastLED.addLeds<NEOPIXEL, NEO_PIN>(leds, NEO_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,CHARGER);
  
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
 
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

#define FRAMES_PER_SECOND  120

void loop() {

    ircontrol();
    
    delay(1000/FRAMES_PER_SECOND);
  
}

void ircontrol()
{
  if (irrecv.decode(&results) ) {

    if ((results.value == 0xFFFFFFFF)){
      if (repeatCommand){
        results.value = lastCommand;  
      } else {
        results.value = -1  ;
      }
    } 
    
    switch(results.value) {
      case VOL_UP:
        BR = constrain(BR+BR_STEP, 0,255 );
        changeColor(HUE,SAT,BR);
        repeatCommand = true;  
        
      break;
      case VOL_DN:
        BR = constrain(BR-BR_STEP, 0,255 );
        changeColor(HUE,SAT,BR);
      break;
      case PT_UP:
        HUE = mod(HUE+HUE_STEP, 0, 255 );
       
        changeColor(HUE,SAT,BR);
        repeatCommand = true;  
      break;
      case PT_DN:
        HUE = mod(HUE-HUE_STEP, 0, 255 );
        changeColor(HUE,SAT,BR);
        repeatCommand = true;  
      break;
      case MODE_SWITCH:
        currentMode = mod(currentMode+1,0,2);
        switch(currentMode) {
          case MODE_DEMO:
           
          break;  
          default:
            changeColor(HUE,SAT,BR);
          break;
          
        }
        repeatCommand = false;
        Serial.println(currentMode);
      break;
      case MODE_ON_OFF:
         on = !on;
         if (!on){
            changeColor(HUE,SAT,0); 
         } else {
           changeColor(HUE,SAT,BR); 
         }
         repeatCommand = false;
      break;
      case -1:
      break;
      default:
        if (currentMode == MODE_DEMO) {
            currentMode = mod(currentMode+1,0,2);
            changeColor(HUE,SAT,BR);
        }
      
        Serial.println(results.value, HEX);
        repeatCommand = false;
      break;
    }

    lastCommand = results.value;

    
    irrecv.resume(); // Receive the next value

  } 

   if (currentMode== MODE_DEMO) {
     demo();
   }
  
}

int mod(int val, int mi, int ma){
  if (val<mi) {
    return ma; 
  } else if (val> ma) {
    return mi;  
  } else{
    return val;  
  }

}

void changeColor(int h, int s, int b){
 
  setAll(h,s,b);
 
  
}

void setPixel(int Pixel, byte h, byte s, byte b) {
 
   switch(currentMode){
    case MODE_COLOR:
       leds[Pixel].setHSV(h,s,b);
    break;
    case MODE_WHITE:
      leds[Pixel].setHSV(h,0,b);
    break;
    
    
   }
}

void setAll(byte h, byte s, byte b) {
  for(int i = 0; i < NEO_LEDS; i++ ) {
    setPixel(i, h, s, b); 
  }
  showStrip();
}

void showStrip() {
   // FastLED
   FastLED.show();
}

void demo(){
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) {
    if (currentMode==MODE_DEMO){ 
        nextPattern(); 
    }
  } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NEO_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NEO_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NEO_LEDS, 10);
  int pos = random16(NEO_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NEO_LEDS, 20);
  int pos = beatsin16(13,0,NEO_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NEO_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NEO_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NEO_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

