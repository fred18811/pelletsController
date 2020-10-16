#include "ESPAsyncWebServer.h"

void onRequest(AsyncWebServerRequest *request){
  request->send(404);
}