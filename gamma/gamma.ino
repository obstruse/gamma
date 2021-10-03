int mainCore = 0;

char fileName[] = __FILE__;
char temp[1000];

static EventGroupHandle_t gamma_event_group;
const int START_GAMMA = BIT0;

#include "wifiTask.h"
#include "i2sTask.h"
#include "dotstarTask.h"
#include "httpTask.h"
#include "httpWifiTask.h"


void setup() {
  Serial.begin(57600);

  gamma_event_group = xEventGroupCreate();
  
  Serial.println("create task: wifi");
  wifiTaskCreate();
  
  Serial.println("create task: http");
  httpTaskCreate();
  httpWifiTaskCreate();

  Serial.println("create task: i2s");
  i2sTaskCreate();

  Serial.println("create task: dotstar");
  dotstarTaskCreate();

  delay(2000);  
}
  
void loop() {
  vTaskDelay(portMAX_DELAY);
}
