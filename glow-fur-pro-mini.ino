#include <FastLED.h>

#define LED_PIN      12   // which pin your pixels are connected to
#define NUM_LEDS    89   // how many LEDs you have
#define BRIGHTNESS 255  // 0-255, higher number is brighter. 
#define SATURATION 255   // 0-255, 0 is pure white, 255 is fully saturated color
#define SPEED       80   // How fast the colors move.  Higher numbers = faster motion
#define STEPS        3   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes
#define BUTTON_PIN   3   // button is connected to pin 2 and GND

#define COLOR_ORDER GRB  // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations
#define FSPEED 15
#define FADER 242
#define LOOPSTART 0

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
//CRGBPalette16 targetPalette( PartyColors_p );
TBlendType    currentBlending;
int ledMode = 3;
int fadeSpeed = FADER ;
int fadeAdder = 4 ;
long loopCounter = LOOPSTART ;

//char bstate1 = 0;
unsigned long lastButtonChange = 0; // button debounce timer.  Replicate as necessary.

byte currKeyState = LOW ;
byte prevKeyState = HIGH;         // button is active low


void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  currentBlending = LINEARBLEND;
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), shortKeyPress, RISING);
  //  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600) ;
  Serial.println( "Starting up") ;
}

void loop() {
  /*
    currKeyState = digitalRead(BUTTON_PIN);

    if ((prevKeyState == LOW) && (currKeyState == HIGH)) {
     shortKeyPress();
    }
    prevKeyState = currKeyState;

  */
  //  digitalWrite(LED_BUILTIN, HIGH);
  //  delay(10) ;
  //  digitalWrite(LED_BUILTIN, LOW);

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  if ( ledMode == 0 ) {
    currentPalette = ForestColors_p;    //Red & Yellow, Fire Colors
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / SPEED);

  } else if ( ledMode == 1 ) {
    currentPalette = RainbowStripeColors_p;   //Rainbow stripes
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / SPEED);

  } else if ( ledMode == 2 ) {
    currentPalette = PartyColors_p; //All the colors except the greens, which make people look a bit washed out
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / SPEED);

  } else if ( ledMode == 3 ) {
    currentPalette = OceanColors_p;    //Red & Yellow, Fire Colors
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / SPEED);

  } else if ( ledMode == 4 ) {
    Fire2012() ;
    FastLED.show();
    FastLED.delay(1000 / FSPEED);

  } else if ( ledMode == 5 ) {
    fadeSpeed = FADER ;
    cylon() ;

  } else if ( ledMode == 6 ) {
    fadeSpeed = 250 ;
    //fill_solid(leds, NUM_LEDS, CRGB::Black);
    fadeall(fadeSpeed);
    addGlitter(90);
    FastLED.show();
    FastLED.delay(2);

  } else if ( ledMode == 7 ) {
    //    fill_solid(leds, NUM_LEDS, CRGB::Black);
    //addGlitter(90);
    loopCounter++ ;
    if ( loopCounter >= NUM_LEDS ) {
      loopCounter = LOOPSTART ;
    }
    //    Serial.print(loopCounter) ;
    //    Serial.println(":  ") ;
    pulse( loopCounter, loopCounter + 20, 0 ) ;
    FastLED.show();
    FastLED.delay(20);

  } else if ( ledMode == 8 ) {
    pulse_static() ;

  } else if ( ledMode == 9 ) {
    pulse2() ;

  } else if ( ledMode == 10 ) {
    pulse_suck() ;

  } else if ( ledMode == 11 ) {
    glowPulse() ;

  }

}

void FillLEDsFromPaletteColors( uint8_t colorIndex) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, BRIGHTNESS, currentBlending);
    colorIndex += STEPS;
  }
  addGlitter(80);
}

void shortKeyPress() {
  if( millis() - lastButtonChange > 500 ) {
    ledMode++;
    Serial.print("ledMode = ") ;
    Serial.println( ledMode ) ;
    if (ledMode > 10) {
      ledMode = 0;
    }
    lastButtonChange = millis() ;
  } else {
     Serial.println( "Too short an interval" ) ;  
  }
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void addRGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] = CHSV( random8(), 255, 255);
  }
}


#define STARTHUE 0
#define ENDHUE 255
#define CSPEED 15

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

  FastLED.show();
  // now that we've shown the leds, reset the i'th led to black
  // leds[i] = CRGB::Black;
  fadeall(fadeSpeed);
  // Wait a little bit before we loop around and do it again
  delay(CSPEED);
}

#define MAX_BRIGHT 255
#define MIN_BRIGHT 10


void fadeall(uint8_t fSpeed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(fSpeed);
  }
}

void brightall(uint8_t fSpeed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] += leds[i].scale8(fSpeed) ;
  }
}


void pulse(uint8_t startP, uint8_t endP, uint8_t hue ) {
  uint8_t middle = endP - round( (endP - startP) / 2 ) ;

  static int brightness = 0;
  static int bAdder = 15;
  static int bStartNew = random8(1, 30) ;

  startP = constrain(startP, 0, NUM_LEDS - 1) ;
  middle = constrain(middle, 0, NUM_LEDS - 1) ;
  endP = constrain(endP, 0, NUM_LEDS - 1) ;

  brightness += bAdder ;
  if ( brightness >= 250 ) {
    bAdder = random8(5, 15) * -1 ;
    brightness += bAdder ;
  }
  if ( brightness <= 0 ) {
    bAdder = 0 ;
    brightness = 0 ;
    if ( startP == bStartNew ) {
      bAdder = 15;
      bStartNew = random8(1, 70) ;
    }
  }
  /*

    Serial.print(" start: ") ;
    Serial.print(startP) ;
    Serial.print(" middle: ") ;
    Serial.print(middle) ;
    Serial.print(" end: ") ;
    Serial.print(endP) ;
    Serial.print(" bright: ") ;
    Serial.println(brightness) ;
  */

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
  fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
}


void pulse2() {
  int middle ;
  int startP ;
  int endP ;
  uint8_t hue ;
  int brightness;
  int bAdder ;
  bool flowDir ;

  while ( ledMode == 9  ) {
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
      /*
                  Serial.print(" start: ") ;
                  Serial.print(startP) ;
                  Serial.print(" middle: ") ;
                  Serial.print(middle) ;
                  Serial.print(" end: ") ;
                  Serial.print(endP) ;
                  Serial.print(" bright: ") ;
                  Serial.print(brightness) ;
                  Serial.print(" hue: ") ;
                  Serial.println(hue) ;
      */

      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_gradient(leds, startP, CHSV(hue, 255, 0), middle, CHSV(hue, 255, brightness), SHORTEST_HUES);
      fill_gradient(leds, middle, CHSV(hue, 255, brightness), endP, CHSV(hue, 255, 0), SHORTEST_HUES);
      FastLED.show();
      FastLED.delay(25);
    }
  }
}

void pulse_static() {
  int middle ;
  int startP ;
  int endP ;
  uint8_t hue ;
  int brightness;
  int bAdder ;
  // bool flowDir ;

  while ( ledMode == 8  ) {
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
  }
}

void pulse_suck() {
  int middle ;
  int startP ;
  int endP ;
  uint8_t hue ;
  int brightness;
  int bAdder ;
  int lastEnd ;
  // bool flowDir ;

  while ( ledMode == 10  ) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.delay(random16(200, 700)) ;

    hue = random8(0, 60) ;
    brightness = MIN_BRIGHT + 1 ;
    bAdder = 10 ;
    startP = lastEnd ;
    endP = startP ;

    while ( brightness > MIN_BRIGHT ) {
      if ( bAdder < 0 and startP < endP ) {
        startP += 2 ;
        if ( startP == endP ) {
          lastEnd = startP ;
          if ( endP > 88 ) {
            lastEnd = 0 ;
          }
          break ;
        }
      }
      if ( bAdder > 0  and ( endP - startP < 20 ) ) {
        endP += 2 ;
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
      FastLED.delay(25);
    }
  }
}

void glowPulse() {
  static int bdirection = 1 ;
  /*
    if ( leds[0].val < MIN_BRIGHT ) {
      fill_solid(leds, NUM_LEDS, CHSV( random8(), 255, MIN_BRIGHT));
      bdirection *= -1 ;
    }

    if ( leds[0].val == MAX_BRIGHT ) {
      bdirection *= -1 ;
    }

    if ( bdirection < 0 ) {
      brightenAll( FADER ) ;
    } else {
      fadeAll( FADER ) ;
    }
  */
}


#define COOLING  55
#define SPARKING 120
#define FIRELEDS 45

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

  /*     Mapping needed:
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




// Have we completed the specified interval since last confirmed event?
// "marker" chooses which counter to check
boolean timeout(unsigned long *marker, unsigned long interval) {
  if (millis() - *marker >= interval) {
    *marker += interval;    // move on ready for next interval
    return true;
  }
  else return false;
}

// Deal with a button read; true if button pressed and debounced is a new event
// Uses reading of button input, debounce store, state store and debounce interval.
boolean butndown(char button, unsigned long *marker, char *butnstate, unsigned long interval) {
  switch (*butnstate) {               // Odd states if was pressed, >= 2 if debounce in progress
    case 0: // Button up so far,
      if (button == HIGH) return false; // Nothing happening!
      else {
        *butnstate = 2;                 // record that is now pressed
        *marker = millis();             // note when was pressed
        return false;                   // and move on
      }

    case 1: // Button down so far,
      if (button == LOW) return false; // Nothing happening!
      else {
        *butnstate = 3;                 // record that is now released
        *marker = millis();             // note when was released
        return false;                   // and move on
      }

    case 2: // Button was up, now down.
      if (button == HIGH) {
        *butnstate = 0;                 // no, not debounced; revert the state
        return false;                   // False alarm!
      }
      else {
        if (millis() - *marker >= interval) {
          *butnstate = 1;               // jackpot!  update the state
          return true;                  // because we have the desired event!
        }
        else
          return false;                 // not done yet; just move on
      }

    case 3: // Button was down, now up.
      if (button == LOW) {
        *butnstate = 1;                 // no, not debounced; revert the state
        return false;                   // False alarm!
      }
      else {
        if (millis() - *marker >= interval) {
          *butnstate = 0;               // Debounced; update the state
          return false;                 // but it is not the event we want
        }
        else
          return false;                 // not done yet; just move on
      }
    default:                            // Error; recover anyway
      {
        *butnstate = 0;
        return false;                   // Definitely false!
      }
  }
}

