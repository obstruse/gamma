#include <driver/i2s.h>

// globals, tagged with I2S
struct {
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
  int amplitude = 0;
  int pulseMsec = 1;
  int pulseFreq = 10000;
  float attackMS = 0.1;
  bool refresh = true;
  int loopTime;
  int writeTime;
  size_t bytesWritten;
} I2S;

const int I2S_SAMPLE_RATE = 96000;
const int I2S_samplesPerMsec = I2S_SAMPLE_RATE / 1000;   // (samples/second) * (second/1000msec) // samples per msec
const int I2S_samplesLen = 25 * I2S_samplesPerMsec;          // 25 ms of samples
const int I2S_DMA_BUF_COUNT = 50;                 // must be between 2 and 128
const int I2S_DMA_BUF_LEN = I2S_samplesLen * 2 / I2S_DMA_BUF_COUNT;   // length in bytes, 2 bytes per sample,  must be between 8 and 1024

int16_t sampleData[I2S_samplesLen] = {};
  
//--------------------------------------------
void i2sRefresh() {
  if (!I2S.refresh) return;

  int pulseSamples = I2S.pulseMsec * I2S_samplesPerMsec;
  int pulseCycles = I2S.pulseFreq * I2S.pulseMsec / 1000;

  // clear samples
  for (int i = 0; i < I2S_samplesLen; sampleData[i++] = 0);

  // put pulse in samples
  for (int i = 0; i < pulseSamples; i++) {

    float content  = sin( (2.0 * PI) * pulseCycles * i / pulseSamples);

    float envelope = 1.0;
    if ( i < ((I2S.attackMS)                 * I2S_samplesPerMsec)) envelope = sin( (PI / 2.0) * i                  / (I2S.attackMS * I2S_samplesPerMsec));
    if ( i > ((I2S.pulseMsec - I2S.attackMS) * I2S_samplesPerMsec)) envelope = sin( (PI / 2.0) * (pulseSamples - i) / (I2S.attackMS * I2S_samplesPerMsec));

    sampleData[i] = float(I2S.amplitude) * envelope * content; 
  }

  I2S.refresh = false;
  return;
}

//--------------------------------------------
// task
//--------------------------------------------
void i2s(void * pvParameters) {
  (void) pvParameters;

  // Initialize the I2S peripheral
  const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate =  I2S_SAMPLE_RATE,              
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, 
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, 
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,                  // default interrupt priority
    .dma_buf_count = I2S_DMA_BUF_COUNT,     // must be between 2 and 128
    .dma_buf_len = I2S_DMA_BUF_LEN,         // length in bytes, must be between 8 and 1024
    .use_apll = false,
    .tx_desc_auto_clear = true
  };
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 26,       // this is BCK pin // was 26
    .ws_io_num = 25,        // this is LRCK pin
    .data_out_num = 22,     // this is DATA output pin
    .data_in_num = -1       // Not used
   };

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin(i2s_num, &pin_config);

  for (;;) {
    size_t bytesWritten;
    I2S.taskCore = xPortGetCoreID();

    // i2s_write will block until it can write to the DMA buffers...
    i2s_write(i2s_num, (void *)(sampleData), sizeof(sampleData), &bytesWritten, portMAX_DELAY);
    xEventGroupSetBits(gamma_event_group, START_GAMMA);

    i2sRefresh();
  }
}

//--------------------------------------------
//--------------------------------------------
void i2sTaskCreate() {
  xTaskCreate( i2s, "i2s", 20000, NULL, 1, &I2S.taskHandle );
  Serial.println("...i2s task started");
}
