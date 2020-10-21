#include <Arduino.h>

/*
//----------------------------------------------------------------Сохранение настроек MQTT------------------------------------------------------------------------
void handleSaveSettingMQTT(String webPage){
  webPage = "";
  webPage += "<html>\
 <head>\
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
   <title>ESP8266 Settings MQTT</title>\
    "+style_css+"\
 </head>";
  webPage += "<body>";
   
 File configFile = SPIFFS.open ("/config.json","w");

      if(server.arg("ip_mqtt")!="")netBuf["ip_mqtt"] = server.arg("ip_mqtt");
      if(server.arg("port_mqtt")!="")netBuf["port_mqtt"] = server.arg("port_mqtt");
      if(server.arg("id_mqtt")!="")netBuf["id_mqtt"] = server.arg("id_mqtt");
      if(server.hasArg("check"))netBuf["mqtton"] = true;
      else netBuf["mqtton"] = false;
  
  serializeJson(netBuf, configFile);
  configFile.close();

  webPage += "<p>Setting MQTT saved.</p>";
  webPage += "<a href='/'>Back</a>";
  webPage += "</body></html>";
  server.send ( 200, "text/html", webPage );
 // ESP.restart();
}
*/
//----------------------------------------------------------------Сохранение настроек Печки------------------------------------------------------------------------
String handleSaveSettingPechka(ArduinoJson6161_11::StaticJsonDocument<400u> pechkaBuf, AsyncWebServerRequest *request){
  String webPage = "";
  webPage += "<html>\
 <head>\
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
    <meta http-equiv='Refresh' content='0; url=/'>\
   <title>ESP8266 Settings ethernet</title>\
 </head>";
  webPage += "<body style=\"text-align: center;\">";

 File pechkaFile = SPIFFS.open ("/configpechka.json","w");
      if(request->arg("deltaTempval")!="")pechkaBuf["deltaTempval"] = request->arg("deltaTempval");
      if(request->arg("timerClear")!="")pechkaBuf["timerClear"] = request->arg("timerClear");
      if(request->arg("timerShnek")!="")pechkaBuf["timerShnek"] = request->arg("timerShnek");
      if(request->arg("timerSvecha")!="")pechkaBuf["timerSvecha"] = request->arg("timerSvecha");
      if(request->arg("timerVent")!="")pechkaBuf["timerVent"] = request->arg("timerVent");
      if(request->arg("maxTemp")!="")pechkaBuf["maxTemp"] = request->arg("maxTemp");
  serializeJson(pechkaBuf, pechkaFile);
  pechkaFile.close();

  webPage += "<p>Setting Ethernet saved.</p>";
  webPage += "</body></html>";
  return webPage;
}

//----------------------------------------------------------------Сохранение настроек сети------------------------------------------------------------------------
String handleSaveSettingEth(ArduinoJson6161_11::StaticJsonDocument<400u> netBuf, AsyncWebServerRequest *request){
  String webPage = "";
  webPage += "<html>\
 <head>\
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
    <meta http-equiv='Refresh' content='0; url=/'>\
   <title>ESP8266 Settings ethernet</title>\
 </head>";
  webPage += "<body style=\"text-align: center;\">";
   
 File configFile = SPIFFS.open ("/config.json","w");

      if(request->arg("ip")!="")netBuf["ip"] = request->arg("ip");
      if(request->arg("gw")!="")netBuf["gw"] = request->arg("gw");
      if(request->arg("subnet")!="")netBuf["subnet"] = request->arg("subnet");
      if(request->arg("dhsp")!="")netBuf["dhsp"] = request->arg("dhsp");
      
  serializeJson(netBuf, configFile);
  configFile.close();

  webPage += "<p>Setting Ethernet saved.</p>";
  webPage += "</body></html>";
  return webPage;
}
//-----------------------------------------------------------Сохранение настроек WIFI для режима SoftAP------------------------------------------------------------------- 
String handleOk(ArduinoJson6161_11::StaticJsonDocument<400u> netBuf, AsyncWebServerRequest *request, String version_prosh){
  String webPage ="";
  webPage += "<html>\
 <head>\
  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
   <title>ESP8266 Settings</title>\
 </head>";
  webPage += "<body>";

  if(SPIFFS.begin()){
    File configFile = SPIFFS.open ("/config.json","w");
    if(!configFile)Serial.println("File open failed");
    else{

    netBuf["wifimode"] = true;
    netBuf["mqtton"] = true;
    netBuf["SSDP_Name"] = request->arg("SSDP_Name");
    netBuf["ssid"] = request->arg("ssid");
    netBuf["pswd"] = request->arg("pswd");
    netBuf["ip"] = request->arg("ip");
    netBuf["gw"] = request->arg("gw");
    netBuf["subnet"] = request->arg("subnet");
    if(request->arg("dhsp")!="")netBuf["dhsp"] = request->arg("dhsp");
    netBuf["version_prosh"] = version_prosh;

       serializeJson(netBuf, configFile);
       configFile.close();
    }
    SPIFFS.end();
    webPage +="Settings saved! </br>Syestem reboot!</br>";
   }
  else {
    Serial.println("File System not maked");
    webPage +="File System not maked</br>";}
  webPage += "</body></html>";
  return webPage;
  }

//-----------------------------------------------------------Форматирование файловой системы------------------------------------------------------------------- 
String clearflash(){
  String webPage ="";
  webPage += "<html>\
  <head>\
  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
  <title>ESP8266 Settings</title>\
  </head>";
  webPage += "<body>"; 
      if(SPIFFS.format()){webPage += "File System is Formated</br>";
       Serial.println("File System is Formated");
      }
      else{webPage += "File System is NOT Formated</br>";
       Serial.println("File System is NOT Formated");
      }
  webPage += "<a href=\"/\">Return to settings page</a></br>";
  webPage += "</body></html>";
  return webPage;
  }