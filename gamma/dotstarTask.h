#include <Adafruit_DotStar.h>
#include <SPI.h>

// globals, tagged with DOTSTAR
struct {
  int gap = 0;
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;
} DOTSTAR;

//--------------------------------------------
// task
//--------------------------------------------
void dotstar(void * pvParameters) {
  (void) pvParameters;

  #define NUMPIXELS 120 // Number of LEDs in strip
  #define DATAPIN    13
  #define CLOCKPIN   14

  Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  pinMode(12,OUTPUT);

  for (;;) {
    static int dotstarIndex = 0;
    DOTSTAR.taskCore = xPortGetCoreID();
    
    xEventGroupWaitBits(gamma_event_group, START_GAMMA, true, true, portMAX_DELAY);

    if (DOTSTAR.gap == 0 ) {
      strip.fill(strip.Color(DOTSTAR.r, DOTSTAR.g, DOTSTAR.b));
      strip.show();                     // Refresh strip
      digitalWrite (12,HIGH);
      delay(12);
      strip.clear();
      strip.show();                     // Refresh strip
      digitalWrite (12,LOW);
    } else {
      for ( int i= 0; i < NUMPIXELS; i++ ) {
        strip.setPixelColor(i, ((i+dotstarIndex) % DOTSTAR.gap ) == 0 ? strip.Color(DOTSTAR.r, DOTSTAR.g, DOTSTAR.b) : 0x000000);
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
