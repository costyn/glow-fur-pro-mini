/*
   Heavily modified from https://learn.adafruit.com/animated-neopixel-gemma-glow-fur-scarf

   This code will not work on a Gemma, it's too big. It will work on any Atmel with at least 16K memory.

   Blame: Costyn van Dongen

   Future ideas:
   - choose 1 color, brightenall to max, then fade to min
   - heartbeat pulse
   - color rain https://www.youtube.com/watch?v=nHBImYTDZ9I
   - two "faders" moving back and forth
*/

#include <FastLED.h>
#include <TaskScheduler.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)       Serial.print (x)
#define DEBUG_PRINTDEC(x)    Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)     Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

#define LED_PIN     12   // which pin your Neopixels are connected to
#define NUM_LEDS    89   // how many LEDs you have
#define BRIGHTNESS 200  // 0-255, higher number is brighter. 
#define SATURATION 255   // 0-255, 0 is pure white, 255 is fully saturated color
#define STEPS        2   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN   3   // button is connected to pin 2 and GND
#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define LOOPSTART 0

#define LEDMODE_SELECT_DEFAULT_INTERVAL 50  // default scheduling time for LEDMODESELECT
#define PALETTE_SPEED  30   // How fast the palette colors move.   Higher delay = slower movement.
#define FIRE_SPEED  85   // Fire Speed; delay in millseconds. Higher delay = slower movement.
#define CYLON_SPEED 25  // Cylon Speed; delay in millseconds. Higher delay = slower movement.
#define FADEGLITTER_SPEED 10  // Cylon Speed; delay in millseconds. Higher delay = slower movement.
#define DISCOGLITTER_SPEED 20  // Cylon Speed; delay in millseconds. Higher delay = slower movement.

CRGB leds[NUM_LEDS];

int ledMode = 17 ; // Which mode do we start with


unsigned long lastButtonChange = 0; // button debounce timer.
byte currKeyState = LOW ;
byte prevKeyState = HIGH;         // button is active low

char *routines[] = {
  "rb",         // 0
  "rb_stripe",  // 1
  "ocean",      // 2
  "heat",       // 3
  "party",      // 4
  "cloud",      // 5
  "forest",     // 6
  "fire2012",   // 7
  "cylon",      // 8
  "cylonmulti", // 9
  "fglitter",   // 10
  "dglitter",   // 11
  "strobe",     // 12
  "flashbpm",   // 13
  "pulse",      // 14
  "pulsestatic",// 15
  "pulse2",     // 16
  "pulsesuck",  // 17
  "racers",     // 18
  "black"       // 19
};
#define NUMROUTINES (sizeof(routines)/sizeof(char *)) //array size  

/* Scheduler stuff */
void ledModeSelect() ; // prototype method
Scheduler runner;
Task taskLedModeSelect( LEDMODE_SELECT_DEFAULT_INTERVAL, TASK_FOREVER, &ledModeSelect); // routine which adds/removes tasks according to ledmode
#define _TASK_SLEEP_ON_IDLE_RUN

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), shortKeyPress, RISING);

  Serial.begin(115200) ;
  DEBUG_PRINT( "Starting up. Numroutines = ") ;
  DEBUG_PRINTLN( NUMROUTINES ) ;

  /* Start the scheduler */
  runner.init();
  runner.addTask(taskLedModeSelect);
  taskLedModeSelect.enable() ;
}

void loop() {
  runner.execute();
}


void ledModeSelect() {
  static long loopCounter = LOOPSTART ;  // loopCounter. (mis)used in pulse

  if ( ledMode >= 0 and ledMode <= 6 ) {
    FillLEDsFromPaletteColors() ;
    taskLedModeSelect.setInterval( PALETTE_SPEED ) ;

    // FastLED Fire2012 split down the middle, so the fire flows "down" from the neck of the scarf to the ends
  } else if ( strcmp(routines[ledMode], "fire2012") == 0 ) {
    Fire2012() ;
    taskLedModeSelect.setInterval( FIRE_SPEED ) ;

    // Cylon / KITT / Larson scanner with fading tail and slowly changing color
  } else if ( strcmp(routines[ledMode], "cylon") == 0 ) {
    taskLedModeSelect.setInterval( CYLON_SPEED ) ;
    cylon() ;

    // Cylon / KITT / Larson scanner with 4 "movers"
  } else if ( strcmp(routines[ledMode], "cylonmulti") == 0 ) {
    taskLedModeSelect.setInterval( CYLON_SPEED ) ;
    cylonMulti() ;

    // Fade glitter
  } else if ( strcmp(routines[ledMode], "fglitter") == 0 ) {
    taskLedModeSelect.setInterval( FADEGLITTER_SPEED ) ;
    fadeGlitter() ;

    //  Disco glitter
  } else if ( strcmp(routines[ledMode], "dglitter") == 0 ) {
    taskLedModeSelect.setInterval( DISCOGLITTER_SPEED ) ;
    discoGlitter() ;

    // With thanks to Hans for the strobe idea https://www.tweaking4all.nl/hardware/arduino/adruino-led-strip-effecten/#strobe
  } else if ( strcmp(routines[ledMode], "strobe") == 0 ) {
    //    setInterval is done in the subroutine itself
    strobe( 0, 10 ) ;

    // Black - off
  } else if ( strcmp(routines[ledMode], "black") == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    taskLedModeSelect.setInterval( 500 ) ;  // long because nothing is going on anyways.


  } else if ( strcmp(routines[ledMode], "pulse") == 0 ) {
    loopCounter++ ;
    if ( loopCounter >= NUM_LEDS ) {
      loopCounter = LOOPSTART ;
    }
    //    Serial.print(loopCounter) ;
    //    Serial.println(":  ") ;
    pulse( loopCounter, loopCounter + 20, 0 ) ;
    FastLED.show();
    taskLedModeSelect.setInterval( 20 ) ;

  } else if ( strcmp(routines[ledMode], "pulsestatic") == 0 ) {
    pulse_static() ;

  } else if ( strcmp(routines[ledMode], "pulse2") == 0 ) {
    pulse2() ;

    // Caterpillar walk
  } else if ( strcmp(routines[ledMode], "pulsesuck") == 0 ) {
    pulse_suck() ;

  } else if ( strcmp(routines[ledMode], "flashbpm") == 0 ) {
    strobe( 130, 2 ) ;

  } else if ( strcmp(routines[ledMode], "racers") == 0 ) {
    racingLeds(loopCounter) ;
    loopCounter++ ;
    FastLED.show();
    taskLedModeSelect.setInterval( 5 ) ;
  }
}

// interrupt triggered button press with a very simple debounce (discard multiple button presses < 500ms)
void shortKeyPress() {
  if ( millis() - lastButtonChange > 300 ) {
    ledMode++;
    DEBUG_PRINT("ledMode = ") ;
    DEBUG_PRINT( routines[ledMode] ) ;
    DEBUG_PRINT( " mode " ) ;
    DEBUG_PRINTLN( ledMode ) ;

    if (ledMode >= NUMROUTINES ) {
      ledMode = 0;
    }

    lastButtonChange = millis() ;
  } else {
    //    DEBUG_PRINTLN( "Too short an interval" ) ;
  }
}

void FillLEDsFromPaletteColors() {
  const CRGBPalette16 palettes[] = { RainbowColors_p, RainbowStripeColors_p, OceanColors_p, HeatColors_p, PartyColors_p, CloudColors_p, ForestColors_p } ;
  static uint8_t startIndex = 0;  // initialize at start
  startIndex++;

  uint8_t colorIndex = startIndex ;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( palettes[ledMode], colorIndex, BRIGHTNESS, LINEARBLEND);
    colorIndex += STEPS;
  }
  addGlitter(80);

  FastLED.show();
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

void fadeGlitter() {
  addGlitter(90);
  FastLED.show();
  fadeall(250);
}

void discoGlitter() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  addGlitter(90);
  FastLED.show();
}


// If you want to restrict the color cycling to a HSV range, adjust these:
#define STARTHUE 0
#define ENDHUE 255

void cylon() {
  static uint8_t hueAdder = 1 ;  // set to higher for faster color cycling
  static uint8_t hue = STARTHUE;
  static uint8_t ledPosAdder = 1 ;
  static uint8_t ledPos = 0;

  leds[ledPos] = CHSV(hue, 255, 255);

  // If current LED position is divisible by 5, increase hue by hueAdder
  if ( ledPos % 5 == 0 ) {
    hue += hueAdder ;
  }
  // if we reach ENDHUE or STARTHUE, start cycling back.
  if ( hue > ENDHUE or hue == STARTHUE ) {
    hueAdder *= -1 ;
  }

  ledPos += ledPosAdder ;
  if ( ledPos == 0 or ledPos == NUM_LEDS ) {
    ledPosAdder *= -1 ;
    ledPos += ledPosAdder ;
  }

  FastLED.show();
  fadeall(230);

}

void cylonMulti() {
  static uint8_t ledPos[] = {0, 29, 48, 87}; // Starting position
  static int ledAdd[] = {1, 1, 1, 1}; // Starting direction

  for (int i = 0; i < 4; i++) {
    leds[ledPos[i]] = CHSV(40 * i, 255, 255);
    if ( (ledPos[i] + ledAdd[i] == 0) or (ledPos[i] + ledAdd[i] == NUM_LEDS) ) {
      ledAdd[i] *= -1 ;
    }
    ledPos[i] += ledAdd[i] ;
  }

  FastLED.show();
  fadeall(180);
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

#define STROBE_ON_TIME 40 

// Pretty awful - current timings are like lightning
void strobe( int bpm, uint8_t numStrobes ) {
  static uint8_t strobesToDo = numStrobes ;
  
  taskLedModeSelect.setInterval(STROBE_ON_TIME); // run this task every STROBE_ON_TIME seconds

  //  DEBUG_PRINTLN( taskLedModeSelect.getRunCounter() ) ;

  if ( (taskLedModeSelect.getRunCounter() % 2 ) == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();

  // use getRunCounter (number of iterations of taskLedModeSelect), and if evenly divisible by strobesToDo, wait a bit
  if ( (taskLedModeSelect.getRunCounter() % strobesToDo) == 0 ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    if ( bpm != 0 ) {
      // If we want to numStrobes of STROBE_ON_TIME and numStrobes of black we need to subtract it from the BPM to delay calculation
      taskLedModeSelect.setInterval( round(60000 / bpm) - ( STROBE_ON_TIME * numStrobes * 2) );
    } else {
      // Lightning simulation
      strobesToDo = random8(4, 12) ;
      taskLedModeSelect.setInterval(random16(1000, 2500));
    }
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
//  taskLedModeSelect.delay(random16(100, 1000)) ;

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
//    taskLedModeSelect.delay(25);
  }
}

#define MIN_BRIGHT 10

void pulse_static() {
  int middle ;
  static int startP ;
  static int endP ;
  static uint8_t hue ;
  static int bAdder ;

  static int brightness = 0 ;
  static bool sequenceEnd ;

  if ( brightness < MIN_BRIGHT ) {
    sequenceEnd = true ;
  }

  // while brightness is more than MIN_BRIGHT, keep increasing brightness etc.
  // If brightness drops below MIN_BRIGHT, we start a new sequence at a new position
  if ( not sequenceEnd ) {
    if ( bAdder < 0 and startP < endP ) {
      startP++ ;
      endP-- ;
      if ( startP == endP ) {
        sequenceEnd = true ;
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
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
    fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
    FastLED.show();
    taskLedModeSelect.setInterval( 25 ) ;
  }

  if ( sequenceEnd ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT + 1 ;
    bAdder = 10 ;
    startP = random8(1, 70);
    endP = startP + 30 ;
    sequenceEnd = false ;
    taskLedModeSelect.setInterval(random16(200, 700)) ;
  }
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
  taskLedModeSelect.delay(random16(200, 700)) ;

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
    taskLedModeSelect.delay(25);
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

  FastLED.show();
}

void racingLeds(long loopCounter ) {
  static uint8_t racer[] = {0, 1, 2}; // Starting positions
  static int racerDir[] = {1, 1, 1}; // Current direction
  static int racerSpeed[] = { random8(1, 4), random8(1, 4) , random8(1, 4) }; // Starting speed
  CRGB racerColor[] = { CRGB::Red, CRGB::Blue, CRGB::White }; // Racer colors

#define NUMRACERS sizeof(racer) //array size  

  fill_solid(leds, NUM_LEDS, CRGB::Black);    // Start with black slate

  for ( int i = 0; i < NUMRACERS ; i++ ) {
    leds[racer[i]] = racerColor[i]; // Assign color

    // If "loopcounter" is evenly divisible by 'speed' then check if we've reached the end (if so, reverse), and do a step
    if ( loopCounter % racerSpeed[i] == 0 ) {
      if ( (racer[i] + racerDir[i] >= NUM_LEDS) or (racer[i] + racerDir[i] <= 0) ) {
        racerDir[i] *= -1 ;
      }
      racer[i] += racerDir[i] ;
    }

    if ( loopCounter % 40 ) {
      racerSpeed[i] = random8(2, 4) ;  // Randomly speed up or slow down
    }
  }
}

