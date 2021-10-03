#include <driver/i2s.h>

// globals, tagged with I2S
struct {
  int amplitude = 30000;
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
} I2S;

//--------------------------------------------
// task
//--------------------------------------------
void i2s(void * pvParameters) {
  (void) pvParameters;

  // Initialize the I2S peripheral
  const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

  const int I2S_SAMPLE_RATE = 48000;
  
  //const int I2S_SAMPLES = I2S_SAMPLE_RATE / 1000 * 2;  // (samples/second) * (second/1000) * (channels)
  //const int I2S_SAMPLES = I2S_SAMPLE_RATE * 3 / 1000 * 2;  // (samples/second) * (3 second/1000) * (channels) // 3 ms
  const int I2S_SAMPLES = I2S_SAMPLE_RATE * 25 / 1000 * 2;  // (samples/second) * (25 second/1000) * (channels) // 25 ms

  const int I2S_BUFFER_LEN = I2S_SAMPLES * 2 ;     // (samples) * (bytes/sample)
  float   sampleData[I2S_SAMPLES] = {};
  int16_t sampleOut[I2S_SAMPLES];

  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate =  I2S_SAMPLE_RATE,              
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, 
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // I2S DACs and ADCs are inherently two-channel, there isn't a mono mode,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 10,     // must be between 2 and 128
    .dma_buf_len = I2S_BUFFER_LEN / 10,    // must be between 8 and 1024
    .use_apll = false
  };
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 26, //this is BCK pin // was 26
    .ws_io_num = 25, // this is LRCK pin
    .data_out_num = 22, // this is DATA output pin
    .data_in_num = -1   //Not used
   };

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin(i2s_num, &pin_config);

  // generate data buffer
  const int oneMillisec = I2S_SAMPLE_RATE / 1000 * 2; // (samples/second) * (second/1000) * (channels)
  for (int i = 0; i < oneMillisec; i++) {
    float envelope = sin( (2.0*PI) * (0.5/1.0) *  (1.0 / oneMillisec) * i); // (radians/cycle) * (cycles/buffer) * ( buffer/samples) * sample# ---- half a cycle/buffer
    float content  = sin( (2.0*PI) * (10.0/1.0) * (1.0 / oneMillisec) * i); // (radians/cycle) * (cycles/buffer) * ( buffer/samples) * sample# ---- 10 cycles/buffer
//    sampleData[i] = float(I2S.amplitude) * envelope * content; 
    sampleData[i] = envelope * content; 
    i++;
    sampleData[i] = sampleData[i-1];
  }
  
  for (;;) {
    size_t bytesWritten;
    I2S.taskCore = xPortGetCoreID();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (int i = 0; i < I2S_SAMPLES; i++) sampleOut[i] = sampleData[i] * float(I2S.amplitude);
    i2s_write(i2s_num, sampleOut, I2S_BUFFER_LEN, &bytesWritten, 1);

    delay(1);
    xEventGroupSetBits(gamma_event_group, START_GAMMA);
    
    vTaskDelayUntil( &xLastWakeTime, 25 / portTICK_PERIOD_MS );
    //delay(25);

  }
}

//--------------------------------------------
//--------------------------------------------
void i2sTaskCreate() {
  xTaskCreate( i2s, "i2s", 20000, NULL, 1, &I2S.taskHandle );
  Serial.println("...i2s task started");
}
