/* KTV Trivia Button

  * 100mm Arcade button with a ring of 17 pixel LEDs around its base and 12 LEDs in the top.
  * Maybe an extra pixel for the table edge light... but maybe not because I didn't wire in the DOUT from the button pixels
  * Also needs to trigger sound effect when the button is hit

AUDIO NOTES
  Using Adafruit audio FX sound board
   * Board can only play one audio file at a time and for a basic trigger, it doesn't read inputs until an audio file has finished
   * I want to be able to cut off an audio file while it's playing if the button is pushed quickly in succession
   * Solution!
      * Set auio files to use 'HOLD' mode - in this mode they're played while the pin is held low
      * Sound board scans for changes in pins while audio is played in this mode (counting up)
      * My solution is to tie two of the same audio files to two pins (SFX 1-2)
        and alternate between triggering each pin when the button is pushed in quick succession.


 */

#include <Adafruit_NeoPixel.h>

// IO Pins
#define PIN_BTN         0
#define PIN_PIXEL       2                     // Data pin for LEDs
#define PIN_SFX_1       3                     // Pins to trigger auio FX - see note above
#define PIN_SFX_2       4                                       

// Pixel Setup
#define NUM_LEDS        29                    // Total number of LEDs
#define NUM_LEDS_B      17                    // Number of LEDs around the base of the button

Adafruit_NeoPixel leds(NUM_LEDS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

// Some colours
#define COL_DEFAULT 0x002200                  // Dim green
#define COL_WHITE   0xFFFF7F

// Global Variables
bool btnState_Last = HIGH;                    // Button state on last loop
uint32_t btnStartTime = 0;                    // (ms) time button was pushed

// Parameters
#define DEBOUNCE        300                   // (ms) debounce time
#define FX_LENGTH       500                   // (ms) Length of effect
#define SFX_TRIG_TIME   1000                  // (ms) Time to hold audio trigger pin high (length of audio file + 120ms sound board takes to begin playing)


void setup() 
{
  // Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);

  pinMode(PIN_SFX_1, OUTPUT);
  pinMode(PIN_SFX_2, OUTPUT);

  digitalWrite(PIN_SFX_1, HIGH);              // Turn off SFX
  digitalWrite(PIN_SFX_2, HIGH);

  leds.begin();                               // Initialize Pixels
  leds.show();                                // Initialize all pixels to 'off'
}


void loop() 
{
  static uint8_t SFX_Playing = 0;             // Holds number of file being played (0 = not playing)
  static uint8_t trig_SFXnum = 1;             // SFX number to trigger

  // Light up LEDs
  updateLEDs();

  // Process button press
  bool btnState = digitalRead(PIN_BTN);
  if (!btnState)                              // If button pressed ...
  {
    if (millis() < btnStartTime)              // (handle timer overflow)
    {
      btnStartTime = millis();
      return;
    }

    if (millis() - btnStartTime < DEBOUNCE)   // Debounce 
      return;

    if (btnState != btnState_Last)            // ... and wasn't previously
    {
      btnStartTime = millis();                // Record time button was initially pressed

      if (SFX_Playing == 1)                   // Decide which audio FX to trigger
        trig_SFXnum = 2;
      else if (SFX_Playing == 2)
        trig_SFXnum = 1;
      else
        trig_SFXnum = 1;      
    }
  }

  btnState_Last = btnState;                   // Record btnState for next time


  // Trigger SFX
  if (millis() - btnStartTime < SFX_TRIG_TIME)
  {
    if (trig_SFXnum == 1)
    {
      digitalWrite(PIN_SFX_1, LOW);
      digitalWrite(PIN_SFX_2, HIGH);
      SFX_Playing = 1;
    }
    else if (trig_SFXnum == 2)
    {
      digitalWrite(PIN_SFX_2, LOW);
      digitalWrite(PIN_SFX_1, HIGH);
      SFX_Playing = 2;
    }
  }
  else
  {
    digitalWrite(PIN_SFX_1, HIGH);
    digitalWrite(PIN_SFX_2, HIGH);
    SFX_Playing = 0;
  }

}


void updateLEDs()
{
  // Lights LEDs when button is pressed 
  static uint8_t StepNum = 0;
  static uint32_t FX_StartTime = millis();
  const uint8_t FX_Time = 10;
  const uint8_t width = 3;

  if (millis() - btnStartTime < FX_LENGTH)
  {
    if (millis() - FX_StartTime < FX_Time)
      return;
    else
      FX_StartTime = millis();

    // Fun effect on bottom of button
    leds.fill(COL_DEFAULT);                   // Set all LEDs to default colour

    for (uint8_t i = 0; i < width; ++i)       // Light up some LEDs white - chase around button
    {
      if (StepNum + i >= NUM_LEDS_B)
        leds.setPixelColor(StepNum + i - NUM_LEDS_B, COL_WHITE);
      else
        leds.setPixelColor(StepNum + i, COL_WHITE);
    }

    // Flash top of button white
    if (StepNum < NUM_LEDS_B/2)      
      leds.fill(COL_WHITE, NUM_LEDS_B, NUM_LEDS);
    else
      leds.fill(COL_DEFAULT, NUM_LEDS_B, NUM_LEDS);

    if (++StepNum >= NUM_LEDS_B)
      StepNum = 0;
  }
  else
  {
    StepNum = 0;                              // Reset for next effect run
    // Solid colour or slow pulsing effect
    leds.fill(COL_DEFAULT);
  }


  leds.show();
  return;
}


