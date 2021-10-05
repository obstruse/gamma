#include <Adafruit_DotStar.h>
#include <SPI.h>

// globals, tagged with DOTSTAR
struct {
  int gap = 0;
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
} DOTSTAR;

//--------------------------------------------
// task
//--------------------------------------------
void dotstar(void * pvParameters) {
  (void) pvParameters;

  #define NUMPIXELS 120 // Number of LEDs in strip
  #define DATAPIN    13
  #define CLOCKPIN   14

  Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  for (;;) {
    static int dotstarIndex = 0;
    DOTSTAR.taskCore = xPortGetCoreID();
    
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
  xTaskCreate( dotstar, "DotStar", 5000, NULL, 1, &DOTSTAR.taskHandle );
  Serial.println("... dotstar task stared");
}
