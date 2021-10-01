TaskHandle_t httpWifiHandle = NULL;
int httpWifiCore = 0;

#ifndef HTTP_H
#define HTTP_H
#include <WebServer.h>

WebServer server ( 80 );
#endif

// second page of http code

//--------------------------------------------
void handleConfig() {

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type", "text/html", true);
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200);

  /*-------------------------------------------*/
  sprintf ( temp,
            "<html>\
<head><title>WIFI Config</title></head>\
<style> body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } </style>\
<body>\
<h1>WIFI Config</h1>" );
  server.sendContent ( temp );

  /*-------------------------------------------*/
  sprintf ( temp,
            "<form action='/httpReconnect'>\
<table border=1 cellpadding=5>\
<tr><td><input type='submit' name='status' value='Reconnect STA'></td></tr>\
</table>\
</form>");
  server.sendContent ( temp );

  /*-------------------------------------------*/
  sprintf ( temp,
            "<form action='/httpAP'>\
<table border=1 cellpadding=5>\
<tr><td><input type='submit' name='status' value='Connect AP'></td></tr>\
</table>\
</form>");
  server.sendContent ( temp );

  //--------------------------------------------
  sprintf ( temp,
            "<table border=1 cellpadding=5>\
<tr><th colspan=2>STA</th></tr>\
<form action='/httpSTA'>\
<tr><th>SSID</th><td><input type='text' name='staSSID' placeholder='SSID'></td></tr>\
<tr><th>PW</th>  <td><input type='text' name='staPW'   placeholder='PW'  ></td></tr>\
<tr><td colspan=2><center><input type='submit' name='STA' value='Connect'></center></form></td></tr> \
</table><br>"
          );
  server.sendContent ( temp );

  //--------------------------------------------
  sprintf ( temp,
            "<table border=1 cellpadding=5>\
<tr><th colspan=2>IP</th></tr>\
<tr><th>STA</th><td>%s</td></tr>\
<tr><th>AP</th><td>%s</td></tr>\
</table><br>",
            WiFi.localIP().toString().c_str(),
            WiFi.softAPIP().toString().c_str()
          );
  server.sendContent ( temp );


  /*-------------------------------------------*/
  sprintf ( temp, "  </body></html>" );
  server.sendContent ( temp );

  server.sendContent ("");
}

//--------------------------------------------
void httpSTA() {

  strcpy(WIFI.ssid,server.arg("staSSID").c_str() );
  strcpy(WIFI.psk, server.arg("staPW").c_str() );

  wifiSTA();
  
  server.sendHeader("Location", String("/config"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
void httpReconnect() {

  wifiReconnect();
  
  server.sendHeader("Location", String("/config"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
void httpAP() {

  wifiAP();
 
  server.sendHeader("Location", String("/config"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
// task
//--------------------------------------------
void httpWifi(void * pvParameters) {
  (void) pvParameters;

  Serial.print("httpWiFi task: Executing on core ");
  Serial.println(xPortGetCoreID());
  
  for (;;) {
    server.handleClient();
    httpWifiCore = xPortGetCoreID();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

//--------------------------------------------
//--------------------------------------------
void httpWifiTaskCreate() {
  
  server.on ( "/config", handleConfig );
  server.on ( "/httpReconnect", httpReconnect );
  server.on ( "/httpAP", httpAP );
  server.on ( "/httpSTA", httpSTA );

  server.begin();
  Serial.println("http started...");

#ifndef HTTP_TASK
#define HTTP_TASK
  xTaskCreate( httpWifi, "HTTP", 5000, NULL, 1, &httpWifiHandle );
#endif

}
