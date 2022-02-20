/* KTV Trivia Button

  * 100mm Arcade button with a ring of 17 pixel LEDs around its base and 12 LEDs in the top.
  * Maybe an extra pixel for the table edge light... but maybe not because I didn't wire in the DOUT from the button pixels
  * Also needs to trigger sound effect when the button is hit



 */

#include <Adafruit_NeoPixel.h>

// IO Pins
#define PIN_BTN         0
#define PIN_PIXEL       2                     // Data pin for LEDs
#define PIN_SFX         3                     // Pin to trigger playing of SFX (using Adafruit soundboard)

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
#define DEBOUNCE        20                    // (ms) debounce time
#define FX_LENGTH       500                   // (ms) Length of effect
#define SFX_TRIG_TIME   50                    // (ms) Time to hold audio trigger pin high


void setup() 
{
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_SFX, OUTPUT);
  digitalWrite(PIN_SFX, LOW);                 // Turn off SFX

  leds.begin();                               // Initialize Pixels
  leds.show();                                // Initialize all pixels to 'off'
}


void loop() 
{
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
      btnStartTime = millis();                // Record time button was initially pressed
  }

  // Trigger SFX
  if (millis() - btnStartTime < SFX_TRIG_TIME)
    digitalWrite(PIN_SFX, HIGH);
  else
    digitalWrite(PIN_SFX, LOW);

  btnState_Last = btnState;                   // Record btnState for next time
  updateLEDs();
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


