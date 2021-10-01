TaskHandle_t dotstarHandle = NULL;
int dotstarCore = 0;

#include <Adafruit_DotStar.h>
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 120 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
//#define DATAPIN    23
//#define CLOCKPIN   18
#define DATAPIN    13
#define CLOCKPIN   14

// globals, tagged with DOTSTAR
struct {
  int gap = 0;
} DOTSTAR;

//--------------------------------------------
// task
//--------------------------------------------
void dotstar(void * pvParameters) {
  (void) pvParameters;

  Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

  Serial.println("in dotstar...");
  Serial.print("Created task: Executing on core ");
  Serial.println(xPortGetCoreID());

//  Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  for (;;) {
    static int dotstarIndex = 0;
    
    xEventGroupWaitBits(gamma_event_group, START_GAMMA, true, true, portMAX_DELAY);

    if (DOTSTAR.gap == 0 ) {
      strip.fill(0xFFFFFF);
      strip.show();                     // Refresh strip
      delay(12);
      strip.clear();
      strip.show();                     // Refresh strip
    } else {
      for ( int i= 0; i < NUMPIXELS; i++ ) {
        strip.setPixelColor(i, ((i+dotstarIndex) % DOTSTAR.gap ) == 0 ? 0xFFFFFF : 0x000000);
        }
      dotstarIndex = (dotstarIndex + 1) % DOTSTAR.gap;
      strip.show();                     // Refresh strip
    }
  }
}

//--------------------------------------------
//--------------------------------------------
void dotstarTaskCreate() {
  xTaskCreate( dotstar, "DotStar", 5000, NULL, 1, &dotstarHandle );
}
