/* 
 * Heavily modified from https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf
 * 
 * This code will not work on a Gemma, it's too big. It will work on any Atmel with at least 16K memory.
 *  
 * Blame: Costyn van Dongen
 * 
 */


#include <FastLED.h>

#define LED_PIN     12   // which pin your Neopixels are connected to
#define NUM_LEDS    89   // how many LEDs you have
#define BRIGHTNESS 255  // 0-255, higher number is brighter. 
#define SATURATION 255   // 0-255, 0 is pure white, 255 is fully saturated color
#define PALETTE_SPEED  80   // How fast the palette colors move.  Higher numbers = faster motion
#define STEPS        3   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN   3   // button is connected to pin 2 and GND

#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define FIRE_SPEED  15        // Fire Speed
#define LOOPSTART 0

#define CYLON_SPEED 25   // Cylon Speed

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
CRGBPalette16 palettes[] = { RainbowColors_p, RainbowStripeColors_p, OceanColors_p, HeatColors_p, PartyColors_p, CloudColors_p, ForestColors_p } ;
TBlendType    currentBlending;

int ledMode = 0;

long loopCounter = LOOPSTART ;

unsigned long lastButtonChange = 0; // button debounce timer.
byte currKeyState = LOW ;
byte prevKeyState = HIGH;         // button is active low

char *routines[] = { "rb", "rb_stripe", "ocean", "heat", "party", "cloud", "forest", "fire2012", "cylon", "fglitter", "dglitter", "strobe", "pulse", "pulsestatic", "pulse2", "pulsesuck", "black" };

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
    fadeall(242);
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
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    FastLED.delay(50);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    FastLED.delay(50);

    // Abuse the palette startIndex counter to pause 1 second every 10 flashes.
    if( (startIndex % 10) == 0 ) {
          FastLED.delay(1000);
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

  if ( ledPos % 3 == 0 ) {
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
  bool flowDir ;

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(random16(200, 2000)) ;

  hue = random8(0, 60) ;
  brightness = 1 ;
  bAdder = 15 ;
  flowDir = random8(2) ;

  if ( flowDir ) {
    endP = random8(20, 89);
  } else {
    startP = random8(1, 70);
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
