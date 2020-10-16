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



String version_prosh ="0.2b";            //---------------------Версия прошивки


Pechka pechka(cooler, moto_shnek, pomp, moto_clear, svecha, fotosensor, cur_shnek, cur_pump, cur_clear, cur_svecha, suh_cont, suh_cont_fotosensor, suh_cont_smog, termistr_temp);

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
        pechka.setMaxTempVal(ulong(pechkaBuf["temval"]));          //Установка максимольной температуры работы печьки
       }
      else{
        pechka.setTimeRele("clear",3);    //--Установка времени работы очистителя
        pechka.setTimeRele("cooler",10);    //--Установка времени работы кулер
        pechka.setTimeRele("shnek",10);    //--Установка времени работы шнек
        pechka.setTimeRele("svecha",120);    //--Установка времени работы свеча
        pechka.setMaxTempVal(70);          //Установка максимольной температуры работы печьки
      }
     confFile.close();

     File netFile = SPIFFS.open ("/config.json","r");   
     if(netFile && netFile.size()){                               //---Проверяем существует ли файл и не нулевой ли он
     deserializeJson(netBuf, netFile);
      
        if(netBuf["wifimode"]){                                   //---Проверяем состояние параметра
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

                    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("max-age=10");

                    server.on("/setting", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(SPIFFS, "/setting.html", "text/html");
                    });
                    server.on("/settingmqtt", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(SPIFFS, "/settingmqtt.html", "text/html");
                    });
                    server.on("/settingpechka", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(SPIFFS, "/settingpechka.html", "text/html");
                    });
                    server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(SPIFFS, "/style.css", "text/css");
                    });
                    server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
                      //request->send(SPIFFS, "/config.json", "text/json");
                      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/config.json", "text/json");
                      response->addHeader("Cache-Control", "max-age=0");
                      request->send(response);
                    });
                    server.on("/configpechka.json", HTTP_ANY, [](AsyncWebServerRequest *request){
                      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/configpechka.json", "text/json");
                      response->addHeader("Cache-Control", "max-age=0");
                      request->send(response);

                      //request->send(SPIFFS, "/configpechka.json", "text/json");
                    });
                    server.on("/function.js", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(SPIFFS, "/function.js", "application/javascript");
                    });

                    server.on("/saveether", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(200, "text/html", handleSaveSettingEth(netBuf, request));
                      ESP.restart();
                    });
                    server.on("/savepechka", HTTP_ANY, [](AsyncWebServerRequest *request){
                      request->send(200, "text/html", handleSaveSettingPechka(pechkaBuf, request));
                      ESP.restart();
                    });
                    server.onNotFound(onRequest);
                    
              /*
                   server.on("/savemqtt", handleSaveSettingMQTT);
              */
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
                     };
              }
           else {
             SoftAP_init();
              server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/ap.html", "text/html");
              });
              server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/style.css", "text/css");
              });
              server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
                AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/config.json", "text/json");
                response->addHeader("Cache-Control", "max-age=0");
                request->send(response);
              });
              server.on("/function.js", HTTP_ANY, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/function.js", "application/javascript");
              });
              server.on("/ok", HTTP_POST, [](AsyncWebServerRequest *request){
                request->send(200, "text/html", handleOk(netBuf, request, version_prosh));
                ESP.restart();
              });

              server.on("/clearflash", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(200, "text/html", clearflash());
                ESP.restart();
              });
              server.begin();
            }
          }
     else{                                 //---Перевод в режим работы SoftAP
          Serial.println("File not opened");
          SoftAP_init();
          server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/ap.html", "text/html");
          });
          server.on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/style.css", "text/css");
          });
          server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/config.json", "text/json");
            response->addHeader("Cache-Control", "max-age=0");
            request->send(response);
          });
          server.on("/function.js", HTTP_ANY, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/function.js", "application/javascript");
          });    
          server.on("/ok", HTTP_POST, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", handleOk(netBuf, request, version_prosh));
          });
          server.on("/clearflash", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/html", clearflash());
          });
          server.begin();
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
  if(pechka.extinguishFire()){}
  else if(!pechka.extinguishFire() && pechka.getStatuFire() && !pechka.getStatusWorkPechka()) {
    pechka.setStatuFire(false);
    }
  else if (!pechka.extinguishFire() && !pechka.getStatuFire()){
    if(pechka.getStatusWorkPechka() && pechka.getTemp() < pechka.getMacTempVal())
    {
      pechka.startRele("clear");
      pechka.startRele("shnek");
      pechka.startRele("svecha");
      pechka.startRele("cooler",true);
      pechka.startRele("fotosensor");
    }
    else if (!pechka.getStatusWorkPechka() || pechka.getTemp() > pechka.getMacTempVal())
    {
      pechka.stopRele("clear");
      pechka.startRele("cooler");
      pechka.stopRele("cooler");
      pechka.stopRele("shnek");
      pechka.stopRele("svecha");
      pechka.stopRele("fotosensor");
    }
  }
}