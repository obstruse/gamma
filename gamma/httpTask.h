#ifndef HTTP_H
#define HTTP_H
#include <WebServer.h>

struct {
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
} HTTP;

WebServer server ( 80 );
#endif

//--------------------------------------------
void handleRoot() {

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
        
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type","text/html",true);
  server.sendHeader("Cache-Control","no-cache");
  server.send(200);
   
/*-------------------------------------------*/
  sprintf ( temp,
"<html>\
  <head>\
    <title>Gamma</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Gamma</h1>");
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
"<form action='/settings'>\
<table border='1' cellpadding='5'>\
 <tr><th colspan='2'>Settings</th></tr>\
 <tr><th>Display Gap</th><td><input type='number' min='0' name='gap' value='%d'></td></tr>\
 <tr><th>Amplitude</th><td><input type='number' min='0' name='amplitude' value='%d'></td></tr>\
 <tr><td colspan=2><center><button type='submit' name='set'>Set</button></center></td></tr>\
</table>\
</form>\
<br>",
  DOTSTAR.gap,
  I2S.amplitude
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
  "\
<table border='1' cellpadding='5'>\
<tr><th colspan=2>Core</th><tr>\
<tr><th>HTTP</th><td>%d</td></tr>\
<tr><th>WIFI</th><td>%d</td></tr>\
<tr><th>I2S</th><td>%d</td></tr>\
<tr><th>DotStar</th><td>%d</td></tr>\
</table>\
<br>",
    HTTP.taskCore, 
    WIFI.taskCore,
    I2S.taskCore,
    DOTSTAR.taskCore
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
  "\
<table border='1' cellpadding='5'>\
<tr><th colspan=2>Stack</th><tr>\
<tr><th>HTTP</th><td>%d</td></tr>\
<tr><th>WIFI</th><td>%d</td></tr>\
<tr><th>I2S</th><td>%d</td></tr>\
<tr><th>DotStar</th><td>%d</td></tr>\
</table>\
<br>",
    uxTaskGetStackHighWaterMark(HTTP.taskHandle),
    uxTaskGetStackHighWaterMark(WIFI.taskHandle),
    uxTaskGetStackHighWaterMark(I2S.taskHandle),
    uxTaskGetStackHighWaterMark(DOTSTAR.taskHandle)
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
  "\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>%s %s %s</p>\
    </body></html>",
    hr, min % 60, sec % 60,
    fileName, __DATE__, __TIME__
  );
  server.sendContent ( temp );

  server.sendContent (" ");
}

//--------------------------------------------
void settings() {

  DOTSTAR.gap = server.arg("gap").toInt();
  I2S.amplitude = server.arg("amplitude").toInt();
  
  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  server.send ( 204, "text/plain", "");
    
}

//--------------------------------------------
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

//--------------------------------------------
// task
//--------------------------------------------
void http(void * pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    HTTP.taskCore = xPortGetCoreID();
    server.handleClient();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

//--------------------------------------------
//--------------------------------------------
void httpTaskCreate() {
  
  server.on ( "/", handleRoot );
  server.on ( "/settings", settings );
  server.onNotFound ( handleNotFound );
  server.begin();
  
#ifndef HTTP_TASK
#define HTTP_TASK
  xTaskCreate( http, "HTTP", 5000, NULL, 1, &HTTP.taskHandle );
  Serial.println("...http task started");
#endif

}
