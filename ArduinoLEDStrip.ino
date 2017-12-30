#include "FastLED.h"
FASTLED_USING_NAMESPACE


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3  //CHANGE BASED ON WHERE YOU PLUG IN YOUR DATA PIN(The one that connects to the DI on the led strip)
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    39  //CHANGE BASED ON THE NUMBER OF LEDS YOU HAVE
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96 //BRIGHTNESS
#define FRAMES_PER_SECOND  100

//Change both variables if you don't want the arduino to directly start with rainbow when starting up: 1 for off, 2 for white, 3 for rgb, 4 for rgb2
int recent_button = 3;
int previous_button = 3;

//change below to your pins
int button_pin = 2;
int pi1 = 4;
int pi2 = 5;
int pi3 = 6;
int pi4 = 7;

//don't change
int recent_pi = 0;
boolean just_rainbow = false;
unsigned long previousMillis = 0;
unsigned long previousDebounce = 0;

void setup() {
  delay(2000); // 2 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  // initialize button, using internal resistor so external resistor is not needed
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(pi1, INPUT);
  pinMode(pi2, INPUT);
  pinMode(pi3, INPUT);
  pinMode(pi4, INPUT);
}

typedef void (*SimplePatternList[])();

//Add in sinelon if you want that pattern(one dot sweeping back and forth)
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, juggle, bpm };

typedef void (*SimplePatternList2[])();
SimplePatternList2 gPatterns2 = { rainbow2 };

uint8_t gCurrentPatternNumber = 0; 
uint8_t gHue = 0;
uint8_t gCurrentPatternNumber2 = 0;
uint8_t gHue2 = 0;

void loop() {
  if(digitalRead(button_pin) == LOW) {
    unsigned long currentDebounce = millis();
    if (currentDebounce - previousDebounce >= 400) {
      recent_button++;
      previousDebounce = currentDebounce;
    }
  }
  if(recent_button > 4) {
    recent_button = 1;
  }
  int pi = pi_input();
  int button = 0;
  if(recent_button == previous_button) {
    button = 0;
    if(pi != recent_pi) {
      button = pi;
    }
  } else {
      if(pi == recent_pi) {
        button = recent_button;
        pi = recent_button  + 3;
      } else {
        recent_pi = pi;
        recent_button = pi - 3;
        button = recent_button;
      }
  }
  // Updating variables when a button is pressed
  if ((button == 1)||(pi == pi1)) {
    FastLED.clear();
    FastLED.show();
  }
  if ((button == 2)||(pi == pi2)) {
    fill_solid( leds, NUM_LEDS, CRGB::White);
    FastLED.show();
  }
  if ((button == 3)||(pi == pi3)) {
    just_rainbow = true;
    colorful();
  }
  if ((button == 4)||(pi == pi4)) {
    just_rainbow = false;
    colorful();
  }
  previous_button = button;
}


void colorful() {
  while (true) {
    if (check_button()) {
      return;
    } else {
      if (just_rainbow) {
        if (check_pi(pi3)) {
          return;
        } else {
          unsigned long currentMillis = millis();
           if (currentMillis - previousMillis >= 1000 / FRAMES_PER_SECOND) {
           // Call the current pattern function once, updating the 'leds' array
           gPatterns2[gCurrentPatternNumber2]();
        
           // send the 'leds' array out to the actual LED strip
            FastLED.show();
            
            // do some periodic updates
            EVERY_N_MILLISECONDS( 20 ) {
              gHue2++;  // slowly cycle the "base color" through the rainbow
            }
          }
        }
      } else {
        if (check_pi(pi4)) {
          return;
        } else {
          unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= 1000 / FRAMES_PER_SECOND) {
            // Call the current pattern function once, updating the 'leds' array
            gPatterns[gCurrentPatternNumber]();
          
            // send the 'leds' array out to the actual LED strip
            FastLED.show();
            // insert a delay to keep the framerate modest
            
            // do some periodic updates
            EVERY_N_MILLISECONDS( 20 ) {
              gHue++;  // slowly cycle the "base color" through the rainbow
            }
            EVERY_N_SECONDS( 10 ) {
              nextPattern();  // change patterns periodically
            }
          }
        }
      }
    }
  }
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
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}
void rainbow2()
{
  fill_rainbow( leds, NUM_LEDS, gHue2, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  //uncomment line below if you want sparkly glitter
  //addGlitter(80);
}


void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
int pi_input() {
  for (int y=pi4;y>=pi1;y--) {
    if (digitalRead(y) == HIGH) {
      return y;
      break;
    } else {
      continue;
    }
  }
  recent_pi = 0;
  return 0;
}
bool check_pi(int not_this) {
  for (int x=pi4;x>=pi1;x--) {
    if ((digitalRead(x) == HIGH) && (x != not_this) && (x != recent_pi)){
      return true;
      break;
    } else {
      continue;
    }
  }
  return false;
}

bool check_button() {
  if(digitalRead(button_pin) == LOW) {
    return true; 
  } else {
    return false;
  }
}



