#include <Arduino.h>
#include <math.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ESP32httpUpdate.h>        //!!!!! под вопросом
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <myfunction.h>
#include <settings.h>
#include <Bounce2.h>
#include <webserverfunc.h>
#include <MyClassWachDog.h>
#include <MyClassPechka.h>
#include <MyClassNextion.h>

String version_prosh ="0.2b";            //---------------------Версия прошивки

Pechka pechka(cooler, moto_shnek, pomp, moto_clear, svecha, fotosensor, cur_shnek, cur_pump, cur_clear, cur_svecha, suh_cont, suh_cont_fotosensor, suh_cont_smog, sensor_temp);
MyNextion myNextion(Serial);

//--------------------------------------------------------------WachDog--------------------------------------------------------------------------------------------
MyWachDog wachdog(whatchdog);

//--------------------------------------------------------------Определение кнопки--------------------------------------------------------------------------------------------
Bounce debouncer = Bounce(); 

//--------------------------------------------------------------Сетевые настройки WIFI----------------------------------------------------------------------------------------
byte ip[4] = {192,168,1,2};
byte gateway[4] = {192,168,1,1};
byte subnet[4] = {255,255,255,0};
String SSDP_Name = "SmartPalnel";

//--------------------------------------------------------------переменные для MQTT--------------------------------------------------------------------------------------------
const char* ipmqtt = "192.168.2.42";
const char* CLIENT_ID = "";

//--------------------------------------------------------------настройки WEB Server--------------------------------------------------------------------------------------
int count_WIFI = 0;

WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
//AsyncEventSource events("/events");

StaticJsonDocument<400> netBuf;
StaticJsonDocument<400> pechkaBuf;


void setup() {

  pinMode(btn_reset, INPUT_PULLUP);
  debouncer.attach(btn_reset);
  debouncer.interval(50);
  wachdog.start();                                              //-----Start WachDog
  pechka.startPechka();                                         //-----Start Pechka

  delay(1000);
  Serial.begin(115200);
  
  if(SPIFFS.begin()){
    
     File confFile = SPIFFS.open ("/configpechka.json","r+"); 
     if(confFile && confFile.size()){
       deserializeJson(pechkaBuf, confFile);
        pechka.setTimeRele("clear",ulong(pechkaBuf["timerClear"]));    //--Установка времени работы очистителя
        pechka.setTimeRele("cooler",ulong(pechkaBuf["timerVent"]));    //--Установка времени работы кулер
        pechka.setTimeRele("shnek",ulong(pechkaBuf["timerShnek"]));    //--Установка времени работы шнек
        pechka.setTimeRele("svecha",ulong(pechkaBuf["timerSvecha"]));    //--Установка времени работы свеча
        pechka.setMaxTempVal(ulong(pechkaBuf["maxTemp"]));               //Установка максимольной температуры работы печьки
        pechka.setDeltaTemp(double(pechkaBuf["deltaTempval"]));
       }
      else{                               //--По умолчанию
        pechka.setTimeRele("clear",3);    //--Установка времени работы очистителя
        pechka.setTimeRele("cooler",10);    //--Установка времени работы кулер
        pechka.setTimeRele("shnek",10);    //--Установка времени работы шнек
        pechka.setTimeRele("svecha",120);    //--Установка времени работы свеча
        pechka.setMaxTempVal(70);          //Установка максимольной температуры работы печьки
        pechka.setDeltaTemp(0);
      }
     confFile.close();

     File netFile = SPIFFS.open ("/config.json","r");   
     deserializeJson(netBuf, netFile);
      
        if(netBuf["wifimode"]){}                                   //---Проверяем состояние параметра
        else {SoftAP_init();}
        if((digitalRead(btn_reset) == LOW)){                   //---Сбрасываем параметр wifimode для перехода точки в режим AP
          netBuf["wifimode"] = false;
          serializeJson(netBuf, netFile);
          netFile.close();
          Serial.println("Please reboot module for coniguration");
          ESP.restart();        
          }
        else{
          String SSDP_Name_value =  netBuf["SSDP_Name"];
          SSDP_Name = SSDP_Name_value;           //---------------Чтение имя WIFI
          const char* ssid = netBuf["ssid"];     //---------------Чтение логина WIFI
          const char* pass = netBuf["pswd"];     //---------------Чтение пароля WIFI
                  
          if(netBuf["ip"]&&netBuf["ip"]!="")writeNetworkSetting(netBuf["ip"],ip) ;
          if(netBuf["gw"]&&netBuf["gw"]!="")writeNetworkSetting(netBuf["gw"],gateway) ;
          if(netBuf["subnet"]&&netBuf["subnet"]!="")writeNetworkSetting(netBuf["subnet"],subnet) ;

          WiFi.mode(WIFI_STA);
          Serial.println();
          Serial.println("Connecting to ");
          Serial.println(ssid);
          Serial.println(pass);

          WiFi.begin(ssid, pass);
                    
          if(netBuf["dhsp"]=="Off")WiFi.config(ip, gateway, subnet);
        
          while (WiFi.status() != WL_CONNECTED) {
            if(digitalRead(btn_reset) != LOW){
              delay(1000);
              count_WIFI++;
              if(count_WIFI>=60){ESP.restart();}
              else{Serial.print(".");}
            }
            else{
              Serial.println("reboot awp");
              File netFile = SPIFFS.open ("/config.json","r");
              if(netFile && netFile.size()){
                  netBuf["wifimode"] = false;
                  serializeJson(netBuf, netFile);
                  netFile.close();
                  Serial.println("Reboot");
                  ESP.restart();
                }
              }
          }

          Serial.println("");
          if(netBuf["dhsp"]=="Off")Serial.println("DHSP off");
          else Serial.println("DHSP on");
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());

// --------------------------------------------------------------Настройка WEB--------------------------------------------------------------------------------------------

          ws.onEvent(onWsEvent);
          server.addHandler(&ws);
          //server.addHandler(&events);

          server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("max-age=10");

          server.on("/setting", HTTP_ANY, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/setting.html", "text/html");});
          server.on("/settingmqtt", HTTP_ANY, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/settingmqtt.html", "text/html");});
          server.on("/settingcontroller", HTTP_ANY, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/settingcontroller.html", "text/html");});
          server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/style.css", "text/css");});
          server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/config.json", "text/json");
            response->addHeader("Cache-Control", "max-age=0");
            request->send(response);
          });
          server.on("/configpechka.json", HTTP_ANY, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/configpechka.json", "text/json");
            response->addHeader("Cache-Control", "max-age=0");
            request->send(response);
          });
          server.on("/function.js", HTTP_ANY, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/function.js", "application/javascript");});
          server.on("/saveether", HTTP_ANY, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", handleSaveSettingEth(netBuf, request));
            ESP.restart();
          });
          server.on("/savepechka", HTTP_ANY, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", handleSaveSettingPechka(pechkaBuf, request));
            ESP.restart();
          });
          server.onNotFound(onRequest);   
          /*server.on("/savemqtt", handleSaveSettingMQTT);*/
// ---------------------------------- Настройки возврата MQTT-------------------------------------------------------------------------------------------------------------
          if (!netBuf["mqtton"]){  
            CLIENT_ID = netBuf["id_mqtt"];
            int port_mqtt = netBuf["port_mqtt"];
            ipmqtt = netBuf["ip_mqtt"];

            client.setServer(ipmqtt,port_mqtt);
            Serial.println("Connecting to MQTT server");
            client.connect(CLIENT_ID);
            Serial.println("connect mqtt...");
            client.connected();
            delay(10);
            Serial.print("IP сервера MQTT: ");
            Serial.println(ipmqtt);
            Serial.println("ID клиенты MQTT: " + String(CLIENT_ID));
            Serial.print("Port сервера MQTT: ");
            Serial.println(port_mqtt);
            client.setCallback(getData);
          }

//--------------------------------------HTTP server подключение----------------------------------------------------------------------------------------------------------- 
            server.begin();
            Serial.println("HTTP server started");
        }
    }
}
void loop() {
//-------------------------------------------------Перевод модуля в режим конфигурации путем замыкания GPIO0 на массу-----------------------------------------------------
  if((digitalRead(btn_reset) == LOW)){
    File netFile = SPIFFS.open ("/config.json","r+");
    if(netFile && netFile.size()){
      netBuf["wifimode"] = false;
      serializeJson(netBuf, netFile);
      netFile.close();
      Serial.println("Please reboot module for coniguration");
      ESP.restart();
    }
  }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------ 
  if(netBuf["wifimode"]){
    client.loop();
  }

  wachdog.loop();
  pechka.loop();

//------------------------отправка данных клиенту вебсокет-------------------------------------------------------
  if(pechka.startTimerEvents()){
    ws.textAll(pechka.getDataFromDigitals());
    }

//---------------------------work pellets controller--------------------------------------------------------------

  if(pechka.extinguishFire()){}
  else if(!pechka.extinguishFire() && pechka.getStatuFire() && !pechka.getStatusWorkPechka()) {
    pechka.setStatuFire(false);
  }
  else if (!pechka.extinguishFire() && !pechka.getStatuFire()){
    if(pechka.getStatusWorkPechka() && pechka.getTemp("ds") < pechka.getMaxTempVal())
    {
      pechka.startRele("clear");
      pechka.startRele("svecha");
      pechka.startRele("cooler",true);
      pechka.startRele("fotosensor");

      if(pechka.getStatusFotosensor())
      {
        if(pechka.startRele("shnek")){
        }else{
          pechka.stopPechcka();
        }
      }else{
        pechka.stopRele("shnek");
      }

    }else if (!pechka.getStatusWorkPechka() || pechka.getTemp("ds") > pechka.getMaxTempVal()){
      pechka.stopRele("clear");
      pechka.startRele("cooler");
      pechka.stopRele("cooler");
      pechka.stopRele("shnek");
      pechka.stopRele("svecha");
      pechka.stopRele("fotosensor");
    }
  }

//----------------------work Uart-------------------------------------------------------------------
  if(myNextion.loop()){
    if(myNextion.getDataParam() == "state"){

      myNextion.sendDataToNextionStr("temp.txt",String(pechka.getTemp("ds"))+"C");

      if(pechka.getStatusRele("cooler")) myNextion.sendDataToNextionVal("cooler.pic","1");
      else myNextion.sendDataToNextionVal("cooler.pic","2");

      if(pechka.getStatusRele("shnek")) myNextion.sendDataToNextionVal("shnek.pic","1");
      else myNextion.sendDataToNextionVal("shnek.pic","2");

      if(pechka.getStatusRele("clear")) myNextion.sendDataToNextionVal("clear.pic","1");
      else myNextion.sendDataToNextionVal("clear.pic","2");

      if(pechka.getStatusRele("svecha")) myNextion.sendDataToNextionVal("cabdle.pic","1");
      else myNextion.sendDataToNextionVal("cabdle.pic","2");

      if(pechka.getStatusFotosensor()) myNextion.sendDataToNextionVal("controlPellets.pic","1");
      else myNextion.sendDataToNextionVal("controlPellets.pic","2");

      if(pechka.getStatusWorkPechka()) myNextion.sendDataToNextionVal("work.pic","1");
      else{ 
        myNextion.sendDataToNextionVal("controlPellets.pic","2");
        myNextion.sendDataToNextionVal("work.pic","2");
        }
    }
  }
}