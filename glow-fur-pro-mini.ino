/*
   Heavily modified from https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf

   This code will not work on a Gemma, it's too big. It will work on any Atmel with at least 16K memory.

   Blame: Costyn van Dongen

   Future ideas:
   - choose 1 color, brightenall to max, then fade to min
   - heartbeat pulse
   
*/


#include <FastLED.h>

#define LED_PIN     12   // which pin your Neopixels are connected to
#define NUM_LEDS    89   // how many LEDs you have
#define BRIGHTNESS 255  // 0-255, higher number is brighter. 
#define SATURATION 255   // 0-255, 0 is pure white, 255 is fully saturated color
#define PALETTE_SPEED  30   // How fast the palette colors move.  Higher numbers = faster motion
#define STEPS        2   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN   2   // button is connected to pin 2 and GND

#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define FIRE_SPEED  15   // Fire Speed; delay in millseconds. Higher delay = slower movement.
#define LOOPSTART 0

#define CYLON_SPEED 25  // Cylon Speed; delay in millseconds. Higher delay = slower movement.

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
CRGBPalette16 palettes[] = { RainbowColors_p, RainbowStripeColors_p, OceanColors_p, HeatColors_p, PartyColors_p, CloudColors_p, ForestColors_p } ;
TBlendType    currentBlending;

int ledMode = 10 ; // Which mode do we start with

long loopCounter = LOOPSTART ;  // loopCounter. (mis)used in some modes

unsigned long lastButtonChange = 0; // button debounce timer.
byte currKeyState = LOW ;
byte prevKeyState = HIGH;         // button is active low

char *routines[] = { "rb", "rb_stripe", "ocean", "heat", "party", "cloud", "forest", "fire2012", "cylon", "fglitter", "dglitter", "strobe", "pulse", "pulsestatic", "pulse2", "pulsesuck", "racers", "black" };
#define NUMROUTINES (sizeof(routines)/sizeof(char *)) //array size  

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  currentBlending = LINEARBLEND;
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), shortKeyPress, RISING);

  Serial.begin(9600) ;
  Serial.print( "Starting up. Numroutines = ") ;
  Serial.println( NUMROUTINES ) ;
}

void loop() {
  static uint8_t startIndex = 0;  // initialize at start
  startIndex = startIndex + 1;
  static uint8_t strobeCount = 10 ;

  if ( ledMode >= 0 and ledMode <= 6 ) {
    currentPalette = palettes[ledMode] ;
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / PALETTE_SPEED );

    // FastLED Fire2012 split down the middle, so the fire flows "down" from the neck of the scarf to the ends
  } else if ( strcmp(routines[ledMode], "fire2012") == 0 ) {
    Fire2012() ;
    FastLED.show();
    FastLED.delay(1000 / FIRE_SPEED );

    // Cylon / KITT / Larson scanner with fading tail and slowly changing color
  } else if ( strcmp(routines[ledMode], "cylon") == 0 ) {
    cylon() ;
    FastLED.show();
    fadeall(240);
    delay( CYLON_SPEED );

    // Fade glitter
  } else if ( strcmp(routines[ledMode], "fglitter") == 0 ) {
    fadeall(250);
    addGlitter(90);
    FastLED.show();
    FastLED.delay(2);

    //  Disco glitter
  } else if ( strcmp(routines[ledMode], "dglitter") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    addGlitter(90);
    FastLED.show();
    FastLED.delay(20);

    // With thanks to Hans for the strobe idea https://www.tweaking4all.nl/hardware/arduino/adruino-led-strip-effecten/#strobe
  } else if ( strcmp(routines[ledMode], "strobe") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::White);  // Pretty awful
    //    fill_solid(leds, NUM_LEDS, CHSV( random8(), 255, 255));  // A lot worse. Just terrible!
    FastLED.show();
    FastLED.delay(40);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    FastLED.delay(40);

    // Abuse the palette startIndex counter to pause 1 - 3.5 seconds every strobeCount flashes.
    if ( (startIndex % strobeCount) == 0 ) {
      FastLED.delay(random16(1000, 3500));
      strobeCount = random8(2, 10) ;
    }

    // Black - off
  } else if ( strcmp(routines[ledMode], "black") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    FastLED.delay(500);

  } else if ( strcmp(routines[ledMode], "pulse") == 0 ) {
    loopCounter++ ;
    if ( loopCounter >= NUM_LEDS ) {
      loopCounter = LOOPSTART ;
    }
    //    Serial.print(loopCounter) ;
    //    Serial.println(":  ") ;
    pulse( loopCounter, loopCounter + 20, 0 ) ;
    FastLED.show();
    FastLED.delay(20);

  } else if ( strcmp(routines[ledMode], "pulsestatic") == 0 ) {
    pulse_static() ;

  } else if ( strcmp(routines[ledMode], "pulse2") == 0 ) {
    pulse2() ;

    // Caterpillar walk
  } else if ( strcmp(routines[ledMode], "pulsesuck") == 0 ) {
    pulse_suck() ;

  } else if ( strcmp(routines[ledMode], "racers") == 0 ) {
    racingLeds() ;
    loopCounter++ ;
    FastLED.show();
    FastLED.delay(15);
  }

}

// interrupt triggered button press with a very simple debounce (discard multiple button presses < 500ms)
void shortKeyPress() {
  if ( millis() - lastButtonChange > 500 ) {
    ledMode++;
    Serial.print("ledMode = ") ;
    Serial.print( routines[ledMode] ) ;
    Serial.print( " mode " ) ;
    Serial.println( ledMode ) ;

    if (ledMode >= NUMROUTINES ) {
      ledMode = 0;
    }

    lastButtonChange = millis() ;
  } else {
    // Debugging output
    // Serial.println( "Too short an interval" ) ;
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, BRIGHTNESS, currentBlending);
    colorIndex += STEPS;
  }
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

// Not used anywhere, but feel free to replace addGlitter with addColorGlitter in FillLEDsFromPaletteColors() above
void addColorGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] = CHSV( random8(), 255, 255);
  }
}


#define STARTHUE 0
#define ENDHUE 255

void cylon() {
  static uint8_t hueAdder = 1 ;
  static uint8_t hue = STARTHUE;
  static uint8_t ledPosAdder = 1 ;
  static uint8_t ledPos = 0;

  leds[ledPos] = CHSV(hue, 255, 255);

  // If current LED position is divisible by 5, increase hue by 1
  if ( ledPos % 5 == 0 ) {
    hue += hueAdder ;
  }
  if ( hue > ENDHUE or hue == STARTHUE ) {
    hueAdder *= -1 ;
  }

  ledPos += ledPosAdder ;
  if ( ledPos == 0 or ledPos == NUM_LEDS ) {
    ledPosAdder *= -1 ;
    ledPos += ledPosAdder ;
  }

}


void fadeall(uint8_t fade_all_speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(fade_all_speed);
  }
}

void brightall(uint8_t bright_all_speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] += leds[i].scale8(bright_all_speed) ;
  }
}


void pulse(uint8_t startPixelPos, uint8_t endPixelPos, uint8_t hue ) {
  uint8_t middlePixelPos = endPixelPos - round( (endPixelPos - startPixelPos) / 2 ) ;

  static int brightness = 0;
  static int brightAdder = 15;
  static int brightStartNew = random8(1, 30) ;

  // Writing outside the array gives weird effects
  startPixelPos  = constrain(startPixelPos, 0, NUM_LEDS - 1) ;
  middlePixelPos = constrain(middlePixelPos, 0, NUM_LEDS - 1) ;
  endPixelPos    = constrain(endPixelPos, 0, NUM_LEDS - 1) ;

  brightness += brightAdder ;
  if ( brightness >= 250 ) {
    brightAdder = random8(5, 15) * -1 ;
    brightness += brightAdder ;
  }
  if ( brightness <= 0 ) {
    brightAdder = 0 ;
    brightness = 0 ;
    if ( startPixelPos == brightStartNew ) {
      brightAdder = 15;
      brightStartNew = random8(1, 70) ;
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fill_gradient(leds, startPixelPos, CHSV(hue, 255, 0), middlePixelPos, CHSV(hue, 255, brightness), SHORTEST_HUES);
  fill_gradient(leds, middlePixelPos, CHSV(hue, 255, brightness), endPixelPos, CHSV(hue, 255, 0), SHORTEST_HUES);
}


void pulse2() {
  int middle ;
  int startP ;
  static int endP ;
  uint8_t hue ;
  int brightness;
  int bAdder ;
  static bool flowDir = 1; // remember flowDir between calls to pulse2

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(random16(100, 1000)) ;

  hue = random8(0, 60) ;
  brightness = 1 ;
  bAdder = 15 ;
  flowDir = ! flowDir ; // flip it!

  if ( flowDir ) {
    endP = random8(30, 70);
  } else {
    startP = random8(30, 70);
  }

  while ( brightness > 0 ) {
    if ( flowDir ) {
      endP-- ;
      startP = endP - 20 ;
    } else {
      startP++ ;
      endP = startP + 20 ;
    }

    if ( startP == 89 or endP == 1 ) {
      break ;
    }

    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, 255) ;
    if ( brightness >= 250 ) {
      bAdder = -10 ;
      //        Serial.print(" bAdder: ") ;
      //        Serial.print(bAdder) ;
      //        brightness += bAdder ;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
    FastLED.delay(25);
  }
}

#define MIN_BRIGHT 10

void pulse_static() {
  int middle ;
  int startP ;
  int endP ;
  uint8_t hue ;
  int brightness;
  int bAdder ;
  // bool flowDir ;

  //  while ( ledMode == 8  ) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(random16(200, 700)) ;

  hue = random8() ;
  brightness = MIN_BRIGHT + 1 ;
  bAdder = 10 ;
  startP = random8(1, 70);
  endP = startP + 30 ;


  while ( brightness > MIN_BRIGHT ) {
    if ( bAdder < 0 and startP < endP ) {
      startP++ ;
      endP-- ;
      if ( startP == endP ) {
        break ;
      }
    }
    if ( bAdder > 0  and ( endP - startP < 30 ) ) {
      startP-- ;
      endP++ ;
    }
    middle = endP - round( (endP - startP) / 2 ) ;

    startP = constrain(startP, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endP = constrain(endP, 0, NUM_LEDS - 1) ;

    brightness += bAdder ;
    brightness = constrain(brightness, 0, 255) ;
    if ( brightness >= 250 ) {
      bAdder = -5 ;
      //        Serial.print(" bAdder: ") ;
      //        Serial.print(bAdder) ;
      //        brightness += bAdder ;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
    FastLED.delay(25);
  }
  // }
}

void pulse_suck() {
  int middle ;
  int startPixelPos ;
  int endPixelPos ;
  uint8_t hue ;
  int brightness;
  int brightnessAdder ;
  static int lastPixelEndPos ;

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(random16(200, 700)) ;

  hue = random8(0, 60) ;
  brightness = MIN_BRIGHT + 1 ;
  brightnessAdder = 10 ;
  startPixelPos = lastPixelEndPos ;
  endPixelPos = startPixelPos ;

  while ( brightness > MIN_BRIGHT ) {
    if ( brightnessAdder < 0 and startPixelPos < endPixelPos ) {
      startPixelPos += 2 ;
      if ( startPixelPos == endPixelPos ) {
        lastPixelEndPos = startPixelPos ;
        if ( lastPixelEndPos > 70 ) {
          lastPixelEndPos = 0 ;
        }
        break ;
      }
    }
    if ( brightnessAdder > 0  and ( endPixelPos - startPixelPos < 20 ) ) {
      endPixelPos += 2 ;
    }
    middle = endPixelPos - round( (endPixelPos - startPixelPos) / 2 ) ;

    startPixelPos = constrain(startPixelPos, 0, NUM_LEDS - 1) ;
    middle = constrain(middle, 0, NUM_LEDS - 1) ;
    endPixelPos = constrain(endPixelPos, 0, NUM_LEDS - 1) ;

    brightness += brightnessAdder ;
    brightness = constrain(brightness, 0, 255) ;
    if ( brightness >= 250 ) {
      brightnessAdder = -5 ;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_gradient(leds, startPixelPos, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endPixelPos, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
    FastLED.delay(25);
  }
}


#define COOLING  55
#define SPARKING 120
#define FIRELEDS round( NUM_LEDS / 2 )

// Adapted Fire2012. This version starts in the middle and mirrors the fire going down to both ends.
// Works well with the Adafruit glow fur scarf.
// FIRELEDS defines the position of the middle LED.

void Fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[FIRELEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < FIRELEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / FIRELEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = FIRELEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = FIRELEDS; j < NUM_LEDS; j++) {
    int heatIndex = j - FIRELEDS ;
    CRGB color = HeatColor( heat[heatIndex]);
    leds[j] = color;
  }

  /*  "Reverse" Mapping needed:
      ledindex 44 = heat[0]
      ledindex 43 = heat[1]
      ledindex 42 = heat[2]
      ...
      ledindex 1 = heat[43]
      ledindex 0 = heat[44]
  */
  for ( int j = 0; j <= FIRELEDS; j++) {
    int ledIndex = FIRELEDS - j ;
    CRGB color = HeatColor( heat[j]);
    leds[ledIndex] = color;
  }

}

void racingLeds() {
  // Current position of racer
  static byte racer1 = 0;
  static byte racer2 = 0;
  static byte racer3 = 0;

  // "speed"; higher is slower.
  int racer1speed = random8(1, 4) ;
  int racer2speed = random8(1, 4) ;
  int racer3speed = random8(1, 4) ;

  // Direction of racer
  static int racer1dir = 1 ;
  static int racer2dir = 1 ;
  static int racer3dir = 1 ;

  // Start with black slate
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Assign some colors
  leds[racer1] = CRGB::Red;
  leds[racer2] = CRGB::Blue;
  leds[racer3] = CRGB::White;

  // If the global variable "loopcounter" is evenly divisible by 'speed' then check if we've reached the end, and add a step
  if ( loopCounter % racer1speed == 0 ) {
    if ( (racer1 + racer1dir >= NUM_LEDS) or (racer1 + racer1dir <= 0) ) {
      racer1dir *= -1 ;
    }
    racer1 += racer1dir ;
  }

  if ( loopCounter % racer2speed == 0 ) {
    if ( ( racer2 + racer2dir >= NUM_LEDS) or (racer2 + racer2dir <= 0) ) {
      racer2dir *= -1 ;
    }
    racer2 += racer2dir ;
  }

  if ( loopCounter % racer3speed == 0 ) {
    if ( ( racer3 + racer3dir >= NUM_LEDS) or (racer3 + racer3dir <= 0) ) {
      racer3dir *= -1 ;
    }
    racer3 += racer3dir ;
  }

  if ( loopCounter % 40 ) {
    racer1speed = random8(2, 4) ;
    racer2speed = random8(2, 4) ;
    racer3speed = random8(2, 4) ;
  }

}

