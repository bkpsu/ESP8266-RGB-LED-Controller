#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 150

// How many effects available? (Add up all switch cases below)
#define MAX_FX 11

// How much time for each effect?
#define FX_TIME 30000

// Define lowest/highest hue (for hue-based effects)
#define HUE_MIN 0
#define HUE_MAX 255

// Define maximum brightness for all-LED effects (up to 100 LEDs, 255 is fine, for 150 LEDs, use 128). Note: FX that don't light up all LEDs don't need this.
#define MAX_BRIGHTNESS 64

// Define number of stars
#define NUM_STARS NUM_LEDS/10 //Number of stars to track (lower number means more stars, e.g. /10 is 10 percent of string, /5 is 20 percent, etc..) <- make sure this calculation yields an integer value!!!

// Define number of string segments (trees, windows, objects)...
#define NUM_SEGMENTS 1

// For debugging, effect to lock on
#define FX_LOCK -1

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 4
//#define CLOCK_PIN 13

#define SELECT_FX_PIN 33

int FX_CONFIG[MAX_FX][2] = { //Master configuration array for each effect (each Row corresponds to effect, each column to one of the parameters of that effect. Note: pot inputs may modify this at runtime
  10/*20*/,            0,  //Rainbow Spin         - A = Delay speed (lower = faster spin), B = Not used
  0,            0,  //Green/Red Alternate  - A = Not used, B = Not used
  5/*10*/,          200,  //Random Stars         - A = Delay speed (lower = faster), B = Hold time for fully lit stars & between star sparkles
  10/*20*/,            5,  //Knight Rider         - A = Delay speed (lower = faster spin), B = Chaser length (number of LEDs on)
  10/*20*/,            5,  //Knight Rider Hue     - A = Delay speed (lower = faster spin), B = Chaser length (number of LEDs on)
  25/*50*/,            2,  //Sine wave            - A = Delay speed, B = Frequency (higher # = lower wavelength) <- also # of "dark spots" in the string
  10/*20*/,            0,  //Sine Hue             - A = Delay speed, B = Not used
  40/*80*/,            0,  //Sine Saturation      - A = Delay speed, B = Not used
  40/*80*/,            10,  //Single point chasers - A = Delay speed, B = Frequency (higher # = lower wavelength)
  60/*120*/,            0,  //Fill Up Hue          - A = Delay speed, B = Not used
  30,                   48 //Noise                - A = scale, B = max changes value for blending palettes
};

// Define the array of leds
CRGB myleds[NUM_LEDS];

static uint8_t fx_idx = 0; //Index of the current effect we're displaying

//****NOISE variables
static uint16_t dist;         // A random number for our noise generator.
CRGBPalette16 targetPalette(OceanColors_p);
CRGBPalette16 currentPalette(CRGB::Black);

unsigned long timeCur = 0, timePre = 0;

void setup() {
  // Uncomment/edit one of the following lines for your leds arrangement.
  // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
  //FastLED.addLeds<WS2811, DATA_PIN, GRB>(myleds, NUM_LEDS);
   FastLED.addLeds<WS2812, DATA_PIN, GRB>(myleds, NUM_LEDS);
  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<NEOPIXEL, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);

  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, RGB>(leds, NUM_LEDS);

 // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  //pinMode (SELECT_FX_PIN, INPUT);

  if (FX_LOCK != -1) {
    fx_idx = FX_LOCK; //set the locked effect if debugging
  }

//  Serial.begin(9600);
//  Serial.println("--- Start Serial Monitor SEND_RCVE ---");
//  Serial.println(" Type in Box above, . ");
//  Serial.println("(Decimal)(Hex)(Character)");
//  Serial.println();
}

void loop() {

  static uint8_t hue_index = HUE_MIN;
  static uint8_t led_index = 0;
  static uint8_t toggle = 0;
  static uint8_t string_segment = NUM_LEDS / NUM_SEGMENTS;

  //Look for input on the select_Fx pin, and increment effect if active
  //if (SELECT_FX_PIN == 1)
  //{
  //  fx_idx++;
  //}

  //Now, process each effect
  switch (fx_idx)
  {
    case 0: //Rainbow Spin (change hue, rotate through the LED string)
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        fill_rainbow(&(myleds[i]), string_segment, hue_index);
      }

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */

      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;

    case 1: //Green/Red Alternate (set every odd pixel green, set every even pixel red, then switch every delay cycle)
      for (int i = 0; i < NUM_LEDS; i++)
      {
        if ((toggle + i) % 2 == 0)
        {
          myleds[i] = CRGB::Crimson;
        }
        else
        {
          myleds[i] = CRGB::DarkGreen;
        }
      }
      if (toggle == 0) {
        toggle = 1;
      }
      else {
        toggle = 0;
      }
      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(1000);
      break;

    case 2: //Random Stars (randomize pixel, set color to dimmer value of blue hue, brightness up then down)
      static int stars[NUM_STARS];
      //First, initialize the stars array with randomly selected pixels
      for (int i = 0; i < NUM_STARS; i++)
      {
        stars[i] = random(0, NUM_LEDS);
      }
      //Now, clear out the old effect
      fill_solid (&(myleds[0]), NUM_LEDS, CHSV(160, 255, 64));
      //Now, iterate through the stars array, and set those pixels on
      /*    for (int i = 0; i < NUM_STARS; i++)
          {
              myleds[stars[i]] = CHSV(random(FX_CONFIG[fx_idx][0],FX_CONFIG[fx_idx][1]),255,64);//255);
          }
      */    //FastLED.show();
      for (int i = 0; i < 255; i = i + 2) //Ramp up brightness
      {
        for (int j = 0; j < NUM_STARS; j++)
        {
          myleds[stars[j]] ++;
        }
        FastLED.show();
        delay(FX_CONFIG[fx_idx][0]);
      }
      delay(FX_CONFIG[fx_idx][1]);
      for (int i = 255; i > 0; i = i - 2) //=i+1) //Ramp down brightness
      {
        for (int j = 0; j < NUM_STARS; j++)
        {
          myleds[stars[j]] --; //.fadeLightBy(i);
        }
        FastLED.show();
        delay(FX_CONFIG[fx_idx][0]);
      }
      delay(FX_CONFIG[fx_idx][1]);
      break;

    case 3: //Knight Rider (bounce set of adjacent leds back and forth between the ends of the string)

      if (led_index >= string_segment) {  //Start off at 0 if the led_index was incremented past the segment size in some other effect
        led_index = 0;
        toggle = 0;
      }
      FastLED.clear(); //Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        //fill_solid (&(myleds[led_index]), FX_CONFIG[fx_idx][1], CHSV(0,255,255)); //Light up a chaser bar
        fill_solid (&(myleds[i + led_index]), FX_CONFIG[fx_idx][1], CHSV(0, 255, 255)); //Light up a chaser bar
      }
      if (toggle == 0) //Iterate up/down through the LED string as a start point for the chaser bar
      {
        led_index++;
        //if (led_index >= NUM_LEDS-FX_CONFIG[fx_idx][1]){toggle = 1;}
        if (led_index >= string_segment - FX_CONFIG[fx_idx][1]) {
          toggle = 1;
        }
      }
      else
      {
        led_index--;
        if (led_index <= 0) {
          toggle = 0;
        }
      }
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]); //delay before moving to the next LED

      break;

    case 4: //Knight Rider Hue (bounce set of adjacent leds back and forth between the ends of the string while rotating through the hue)

      if (led_index >= string_segment) {  //Start off at 0 if the led_index was incremented past the segment size in some other effect
        led_index = 0;
        toggle = 0;
      }
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        //fill_solid (&(myleds[led_index]), FX_CONFIG[fx_idx][1], CHSV(0,255,255)); //Light up a chaser bar
        fill_solid (&(myleds[i + led_index]), FX_CONFIG[fx_idx][1], CHSV(hue_index, 255, 255)); //Light up a chaser bar
      }
      if (toggle == 0) //Iterate up/down through the LED string as a start point for the chaser bar
      {
        led_index++;
        //if (led_index >= NUM_LEDS-FX_CONFIG[fx_idx][1]){toggle = 1;}
        if (led_index >= string_segment - FX_CONFIG[fx_idx][1]) {
          toggle = 1;
        }
      }
      else
      {
        led_index--;
        if (led_index <= 0) {
          toggle = 0;
        }
      }
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]); //delay before moving to the next LED

      hue_index++; //Iterate through the hue palette
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }

      break;

    case 5: //Sin wave of hue, modulating to black along the string (parameters: central hue, and wavelength/amplitude?)
      if (led_index >= NUM_LEDS) {  //Start off at 0 if the led_index was incremented past the segment size in some other effect
        led_index = 0;
      }
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + 1)
      {
        //fill_solid (&(myleds[i+led_index]), 1, CHSV(hue_index,255,int(abs(sin(float(led_index)/string_segment*FX_CONFIG[fx_idx][1]*3.14159)*255)))); //Light up a chaser bar
        myleds[i] = CHSV(hue_index, 255, 255 - int(abs(sin(float(i + led_index) / NUM_LEDS * FX_CONFIG[fx_idx][1] * 3.14159) * 255)));
        /* Serial.print(i);
          Serial.print(":");
          Serial.print(int(sin(float(i)/string_segment*2*3.14159)*255));
          Serial.print("\n");*/
      }

      led_index++;

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */


      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;

    case 6: //Sin wave of visibility, modulating with changing hue (parameters: delay) (Whole String)
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        fill_solid (&(myleds[i]), string_segment, CHSV(hue_index, 255, int(abs(sin(float(hue_index) / HUE_MAX * 2 * 3.14159) * 255)))); //Light up a chaser bar
        //myleds[i]=CHSV(FX_CONFIG[fx_idx][1],255,int(sin(float(hue_index)/HUE_MAX*16*3.14159)*255));
        /* Serial.print(i);
          Serial.print(":");
          Serial.print(int(sin(float(i)/string_segment*2*3.14159)*255));
          Serial.print("\n");*/
      }

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */

      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;


    case 7: //Sin wave of saturation, modulating with changing hue (parameters: delay) (Whole String)
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        fill_solid (&(myleds[i]), string_segment, CHSV(hue_index, int(abs(sin(float(hue_index) / HUE_MAX * 2 * 3.14159) * 255)), 255)); //Light up a chaser bar
        //myleds[i]=CHSV(FX_CONFIG[fx_idx][1],255,int(sin(float(hue_index)/HUE_MAX*16*3.14159)*255));
        /* Serial.print(i);
          Serial.print(":");
          Serial.print(int(sin(float(i)/string_segment*2*3.14159)*255));
          Serial.print("\n");*/
      }

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */

      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;

    case 8: //Single point chasers (hue)
      if (led_index >= string_segment) {//NUM_LEDS) {  //Start off at 0 if the led_index was incremented past the segment size in some other effect
        led_index = 0;
      }
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < NUM_LEDS; i = i + string_segment)
      {
        fill_solid (&(myleds[i + led_index]), 1, CHSV(hue_index, 255, int(abs(sin(float(led_index) / string_segment * FX_CONFIG[fx_idx][1] * 3.14159) * 255)))); //Light up a chaser bar
        //myleds[i]=CHSV(hue_index,255,255-int(abs(sin(float(i+led_index)/NUM_LEDS*FX_CONFIG[fx_idx][1]*3.14159)*255)));
        /* Serial.print(i);
          Serial.print(":");
          Serial.print(int(sin(float(i)/string_segment*2*3.14159)*255));
          Serial.print("\n");*/
      }

      led_index++;

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */


      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;

    case 9: //Fill Up with changing hue (parameter: delay)
      if (led_index >= string_segment) {//NUM_LEDS) {  //Start off at 0 if the led_index was incremented past the segment size in some other effect
        led_index = 0;
      }
      FastLED.clear(); // Clear LEDs
      for (int i = 0; i < string_segment; i = i + 1)
      {
        fill_solid (&(myleds[string_segment - led_index - 1]), i, CHSV(hue_index, 255, 255)); //int(abs(sin(float(i) / string_segment * FX_CONFIG[fx_idx][1] * 3.14159) * 255)))); //Light up a chaser bar
        //myleds[i]=CHSV(hue_index,255,255-int(abs(sin(float(i+led_index)/NUM_LEDS*FX_CONFIG[fx_idx][1]*3.14159)*255)));
        /* Serial.print(i);
          Serial.print(":");
          Serial.print(int(sin(float(i)/string_segment*2*3.14159)*255));
          Serial.print("\n");*/
      }

      led_index++;

      FastLED.setBrightness(MAX_BRIGHTNESS);
      FastLED.show();
      delay(FX_CONFIG[fx_idx][0]);

      /* TODO: Read analog input (pot) and constrain to use as input in delay function above (slow down/speed up spin) <- this should be done for both parameters in the FX_CONFIG array above */


      hue_index++;
      if (hue_index >= HUE_MAX) {
        hue_index = HUE_MIN;
      }
      break;
      case 10: //Noise
        EVERY_N_MILLISECONDS(10) {
          nblendPaletteTowardPalette(currentPalette, targetPalette, FX_CONFIG[fx_idx][1]);  // FOR NOISE ANIMATION
          //{ gHue++; }
       
        for (int i = 0; i < NUM_LEDS; i++) {                                     // Just ONE loop to fill up the LED array as all of the pixels change.
          uint8_t index = inoise8(i * FX_CONFIG[fx_idx][0], dist + i * FX_CONFIG[fx_idx][0]) % 255;            // Get a value from the noise function. I'm using both x and y axis.
          myleds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
        }
        dist += beatsin8(10, 1, 4);                                              // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
        }
        FastLED.show();
        EVERY_N_SECONDS(5) {
          targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
        }

        FastLED.delay(50);
        
      break;
    /*
       case ?: //Walk (intermittent spaced bright, then less bright, then dim -> cycle through)
         break;
    */
    /*
       case ?: //Falling Rain (Start at end of dark-color(or unlit) segment, light up the last LED, then randomly add 2-3 pixels per cycle and light up the next pixel while turning off the previous one) <- If the
               //string is wrapped around a tree, it should look like a falling raindrop
    */
    default:
      break;
  }

  timeCur = millis();
  if (timeCur - timePre > FX_TIME) //timer to change effect has elapsed
  {
    timePre = timeCur;
    if (FX_LOCK != -1) //If we're locking for debugging, set the fx_idx
    {
      fx_idx = FX_LOCK;
    }
    else //If not debugging a specific effect, increment to the next effect
    {
      fx_idx++;
      if (fx_idx >= MAX_FX)
      {
        fx_idx = 0;
      }
    }
  }

}

