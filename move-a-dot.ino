
  //  MOVE A DOT
  //  Version 11-03-2017
  //  Move one or more dots over a circle of leds with a fading effect
  //  Jonas Vorwerk 

#define FASTLED_ALLOW_INTERRUPTS 0 //fix for esp8266
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#define DATA_PIN            14 
#define CLOCK_PIN           15
#define LED_TYPE            NEOPIXEL
#define COLOR_ORDER         RGB

#define NUM_LEDS 60
#define BRIGHTNESS 255
#define DITHER 0
#define CORRECTION TypicalLEDStrip

#define FRAMES_PER_SECOND  60
#define SECONDS_PER_PALETTE  60
#define SECONDS_PER_PATTERN  60

#define NUM_DOTS 3
#define MAX_DOT_AGE 3000
#define FADEOUT 5

uint8_t hue_dot[NUM_DOTS];
uint8_t sat_dot[NUM_DOTS];
uint8_t val_dot[NUM_DOTS];
int age_dot[NUM_DOTS];
float spd_dot[NUM_DOTS];
float pos_dot[NUM_DOTS];

CRGB leds[NUM_LEDS];

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

void setup() {
  Serial.begin(9600);
  delay(3000);

  //FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(DITHER);
  FastLED.setCorrection(CORRECTION);
  randomSeed(analogRead(0));

  chooseNextColorPalette(gTargetPalette);
  init_all_dots();
}

void loop() { 
  
  EVERY_N_SECONDS( SECONDS_PER_PALETTE ) { 
    chooseNextColorPalette( gTargetPalette ); 
  }

  EVERY_N_MILLISECONDS( 10 ) {
    nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 12);
  }
    
  move_a_dot();
  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void move_a_dot() {

  //fadeout
  fadeToBlackBy(leds, NUM_LEDS, FADEOUT);

  //calculate all dots
  for (int i = 0 ; i < NUM_DOTS ; i++) {

    //show the dot
    //leds[ int(pos_dot[i]) ] = CHSV(hue_dot[i], sat_dot[i], val);
    leds[ int(pos_dot[i]) ] = ColorFromPalette( gCurrentPalette, hue_dot[i], val_dot[i], NOBLEND);

    //change dot pos_position
    pos_dot[i] = pos_dot[i] + spd_dot[i];

    //increase age
    age_dot[i] = age_dot[i] + 1;
    if(age_dot[i] > MAX_DOT_AGE) init_dot(i);

    //if dot at end, move to beginning 
    if( pos_dot[i] >= NUM_LEDS && spd_dot[i] > 0) pos_dot[i] = 0;  
    //other way around, if at beginning, move to the end
    if( pos_dot[i] < 0 && spd_dot[i] < 0) pos_dot[i] = NUM_LEDS - 1;

  }
}

//init one dot
void init_dot(int i) {
  Serial.print("init_dot:" + String(i));
  spd_dot[i] = map(random(256), 0, 255, 5, 30)/100.0 * (random(2) - 1 | 1);
  pos_dot[i] = random(NUM_LEDS-1);
  age_dot[i] = random(int(MAX_DOT_AGE/100));
  hue_dot[i] = random(256); 
  sat_dot[i] = 255; //random(256);
  val_dot[i] = 255; //random(256);
  
  Serial.print(" spd:" + String(spd_dot[i])); 
  Serial.print(" pos:" + String(pos_dot[i])); 
  Serial.print(" age:" + String(age_dot[i])); 
  Serial.print(" hue:" + String(hue_dot[i])); 
  Serial.print(" sat:" + String(sat_dot[i])); 
  Serial.print(" val:" + String(val_dot[i]));
  Serial.println(""); 
}

//init all dots
void init_all_dots() {
  Serial.println("init_all_dots"); 
  for (int i = 0 ; i < NUM_DOTS ; i++) {
    init_dot(i);
  }
}

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM = {
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0x304048,
  0x304048, 0x304048, 0x304048, 0xE0F0FF
};

// Orange warm feel with white dots
const TProgmemRGBPalette16 warm_orange FL_PROGMEM = {
  0xFED464, 0xFED464, 0xFEF23C, 0xFEF23C,
  0xFED464, 0xFFFEB5, 0xFED464, 0xFEF23C,
  0xF7D639, 0xF7D639, 0xFEF23C, 0xFEF23C,
  0xFFFEB5, 0xFEF23C, 0xF7D639, 0xFFFEB5
};

// Add or remove palette names from this list to control which color
// palettes are used, and in what order.
const TProgmemRGBPalette16* ActivePaletteList[] = {
  &warm_orange,
  &Snow_p,
};

// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal){
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1; 
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);
  pal = *(ActivePaletteList[whichPalette]);
}
