#define FASTLED_INTERRUPT_RETRY_COUNT 0 
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
FASTLED_USING_NAMESPACE


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3  //CHANGE BASED ON WHERE YOU PLUG IN YOUR DATA PIN(The one that connects to the DI on the led strip)
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    38  //CHANGE BASED ON THE NUMBER OF LEDS YOU HAVE
CRGBArray <NUM_LEDS> leds;
CRGBSet leds_white(leds);
//CRGBSet leds_white(leds(7,31)); //If your white leds are flickering, comment out the line above via // and uncomment this line, and set the two numbers to the first led num and last led num.
#define BRIGHTNESS          96 //BRIGHTNESS, Although it doesn't seem to do anything
#define FRAMES_PER_SECOND  100 //FPS of the patterns, 100 is best

//Change both variables if you don't want the arduino to directly start with rainbow when starting up: 1 for off, 2 for white, 3 for rgb, 4 for rgb2
int recent_button = 3;

//change below to your pins
int button_pin = 2;
int pi1 = 4;
int pi2 = 5;
int pi3 = 6;
int pi4 = 7;

//enable if you're using rgb button
boolean enable_rgb_button = true;
int rgb_r = 9;
int rgb_g = 10;
int rgb_b = 11;
//calibrate the rgb values to create a good white color on the button, mine was 160, 50, 0, but it'll be different for everyone
int rgb_white[] = {160, 50, 0};
//The rgb color you want the button to become when clicked
int clickColor[] = {100, 0, 255};
//The speed you want the rgb button to fade through the red, blue, and green colors
int buttonChangeTime = 40; 

//don't change
int dif = pi1 - 1;
int r_change = 0;
int g_change = 255;
int b_change = 255;
int current_rgb_change = 1;
int recent_pi = 0;
int recent_pi1 = 0;
boolean just_rainbow = false;
unsigned long previousMillis = 0;
unsigned long previousDebounce = 0;
unsigned long previousRGBButton = 0;
int previous_button = 0;

void setup() {
  delay(1000); // 2 second delay for recovery
  
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
  pinMode(rgb_r, OUTPUT);
  pinMode(rgb_g, OUTPUT);
  pinMode(rgb_b, OUTPUT);
}

typedef void (*SimplePatternList[])();

//Add in sinelon if you want that pattern(one dot sweeping back and forth)
SimplePatternList gPatterns = { rainbow, confetti, juggle, bpm };

typedef void (*SimplePatternList2[])();
SimplePatternList2 gPatterns2 = { rainbow2 };

uint8_t gCurrentPatternNumber = 0; 
uint8_t gHue = 0;
uint8_t gCurrentPatternNumber2 = 0;
uint8_t gHue2 = 0;

void loop() {
  if(digitalRead(button_pin) == LOW) {
    unsigned long currentDebounce = millis();
    if (currentDebounce - previousDebounce >= 400) { //check for debounce on button
      recent_button++; //add 1 to current button state
      previousDebounce = currentDebounce;
    }
  }
  //cycle through 1 --> 4 on the button
  if(recent_button > 4) {
    recent_button = 1;
  }
  //get pi input
  int pi = pi_input();
  int button = 0;
  //if button has not changed, set button to 0, then check if pi has changed, if it has, then it'll set button also to pi
  if(recent_button == previous_button) {
    button = 0;
    if ((pi != recent_pi) and (pi != 0)) {
      button = 0;
      recent_button = pi - dif;
      previous_button = pi - dif;
      recent_pi = pi;
      recent_pi1 = pi;
    } else {
      pi = 0;
    }
  //if button has changed, set pi and button to the value of recent_button, recent_button loops from 1-->4 and pi loops depending on the pins it's connected to, so we add by a difference
  } else {
    button = recent_button;
    pi = 0;
    previous_button = recent_button;
    rgb_button(clickColor[0], clickColor[1], clickColor[2]);
    while(digitalRead(button_pin) == LOW) {
      delay(1);
    }
  }
  //set previous button to the recent_button, so we don't count it the next time around

  // Updating variables when a button is pressed
  if ((button == 1)||(pi == pi1)) {
    
    FastLED.clear();
    FastLED.show();
    //change button to off
    rgb_button(255, 255, 255);
  }
  if ((button == 2)||(pi == pi2)) {
    FastLED.clear();
    fill_solid( leds_white, NUM_LEDS , CRGB::White);
    FastLED.show();
    //set button white
    rgb_button(rgb_white[0], rgb_white[1], rgb_white[2]);
  }
  if ((button == 3)||(pi == pi3)) {
    //just do rainbow pattern
    just_rainbow = true;
    colorful();
  }
  if ((button == 4)||(pi == pi4)) {
    just_rainbow = false;
    colorful();
  }

}


void colorful() {
  while (true) {
    if (check_button()) { //return is button is clicked
      return;
    } else {
      if (just_rainbow) {
        if (check_pi(pi3)) { //return if pi changes
          return;
        } else {
          unsigned long currentMillis = millis();
          //delay using millis() by the FPS specified
           if (currentMillis - previousMillis >= 1000 / FRAMES_PER_SECOND) {
             // Call the current pattern function once, updating the 'leds' array
             gPatterns2[gCurrentPatternNumber2]();
          
             // send the 'leds' array out to the actual LED strip
              FastLED.show();
             //update LED Button
              rgb_button_fade();
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
            //update LED Button
            rgb_button_fade();
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

//All the patterns
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
  addGlitter(80);
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
//loop through pins to check if pi has changed, and return pin number
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
//check if pi has changed, while avoiding specified value, returns true/false
bool check_pi(int not_this) {
  for (int x=pi4;x>=pi1;x--) {
    if ((digitalRead(x) == HIGH) && (x != not_this) && (x!= recent_pi1)) {
      return true;
      break;
    } else {
      continue;
    }
  }
  return false;
}
//check if button clicked
bool check_button() {
  unsigned long current_debounce = millis();
  if((digitalRead(button_pin) == LOW) and (current_debounce - previousDebounce >= 400)) {
    return true; 
  } else {
    return false;
  }
}
//send color by writing a pwm signal to the pins
void rgb_button (int r, int g, int b) {
  analogWrite(rgb_r, r);
  analogWrite(rgb_g, g);
  analogWrite(rgb_b, b);
}
//rgb fading of colors on the button
void rgb_button_fade () {
  unsigned long current_mil = millis();
  if (previousRGBButton - current_mil > buttonChangeTime) {
    if (current_rgb_change == 1) {
      r_change++;
      g_change--;
      if(r_change >= 255) {
        current_rgb_change = 2;
      }
    } else if (current_rgb_change == 2) {
      g_change++;
      b_change--;
      if(g_change >= 255) {
        current_rgb_change = 3;
      }
    } else {
      b_change++;
      r_change--;
      if(b_change >= 255) {
        current_rgb_change = 1;
      }
    }
    analogWrite(rgb_r, r_change);
    analogWrite(rgb_g, g_change);
    analogWrite(rgb_b, b_change);
    previousRGBButton = current_mil;
  }
  
}  
