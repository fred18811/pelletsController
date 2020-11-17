#include <Arduino.h>
#include <MyClassTemperature.h>
#include <MyClassTimer.h>
#include <OneWire.h>
OneWire ds;
class Pechka{
    private:

        TempSensor tempterm;
//--------------Timer---------------------
        MyTimer timer_clear;
        MyTimer timer_cooler;
        MyTimer timer_shnek;
        MyTimer timer_svecha;
        MyTimer timer_sensor_ds;
        MyTimer timer_temp;
        float temperature = 0;
//----------------------------------------

        bool count_cooler = 0;
        bool flagFire = false;
        bool flagWorkPechkaStop = false;

        bool cooler_value = false;
        bool shnek_value = false;
        bool pump_value = false;
        bool clear_value = false;
        bool svecha_value = false;
        bool fotosensor_value = false;

        bool btn1 = HIGH;
        bool bounce_btn = 0;

        double delta_termistr = 0;

        int cooler_pech;
        int moto_shnek_pech;
        int pomp_pech;
        int moto_clear_pech;
        int svecha_pech;
        int fotosensor_pech;
        uint8_t temp_sensor_pech = 23;

        int cur_shnek_pech;
        int cur_pump_pech;
        int cur_clear_pech;
        int cur_svecha_pech;
        int suh_cont_pech;
        int suh_cont_fotosensor_pech;
        int suh_cont_smog_pech;

        int maxtemp_val = 70;
        uint32_t value_cooler = 0;

        double getCurValue(double cur_value)
        {
            double state_value = ((2500.0-(cur_value/4096.0*4000.0))/100)-0.78;
            return  state_value>0?state_value:0 ;
        }
        double getAverageCur(double cur_state){
            double val=0;
            for(int i = 0 ; i<10;i++){
                val+=getCurValue(cur_state);
            }
            return val/10;
        }
//---------------------Set Value Rele-------------------------------------------------------------------------------
        bool setValueRele(String name, uint32_t mode){
            if (name == "cooler"){
                if(mode == 0){
                    value_cooler = mode;
                    ledcWrite(cooler_pech, mode);
                    cooler_value = true;
                }
                else if (mode > 0 && mode < 255){
                    value_cooler = mode;
                    ledcWrite(cooler_pech, mode);
                    cooler_value = false;
                }
                return 1;
            }
            return 0;
        }
        bool setValueRele(String name, String mode)
        {
            if (name == "cooler"){
                if(mode == "on"){
                    digitalWrite(cooler_pech, HIGH);
                    cooler_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(cooler_pech, LOW);
                    cooler_value = false;
                }
                return 1;
            }
            else if (name == "shnek"){
                if(mode == "on"){
                    digitalWrite(moto_shnek_pech, HIGH);
                    shnek_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(moto_shnek_pech, LOW);
                    shnek_value = false;
                }
                return 1;
            }
            else if (name == "pomp"){
                if(mode == "on"){
                    digitalWrite(pomp_pech, HIGH);
                    pump_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(pomp_pech, LOW);
                    pump_value = false;
                }
                return 1;
            }
            else if (name == "clear"){
                if(mode == "on"){
                    digitalWrite(moto_clear_pech, HIGH);
                    clear_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(moto_clear_pech, LOW);
                    clear_value = false;
                }
                return 1;
            }
            else if (name == "svecha"){
                if(mode == "on"){
                    digitalWrite(svecha_pech, HIGH);
                    svecha_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(svecha_pech, LOW);
                    svecha_value = false;
                }
                return 1;
            }
            else if (name == "fotosensor"){
                if(mode == "on"){
                    digitalWrite(fotosensor_pech, HIGH);
                    fotosensor_value = true;
                }
                else if (mode == "off"){
                    digitalWrite(fotosensor_pech, LOW);
                    fotosensor_value = false;
                }
                return 1;
            }
            else return 0;
        }
        void getTempDS(){
            byte data[12];
            byte i;
            if(timer_temp.startTimer()){}
            else
            {
                if(ds.reset())
                {
                    ds.write(0xCC);
                    ds.write(0x44);
                    ds.reset();
                    ds.write(0xCC);
                    ds.write(0xBE);
                    for ( i = 0; i < 9; i++)
                    {
                        data[i] = ds.read();
                    }
                    int16_t raw = (data[1] << 8) | data[0];
                    byte cfg = (data[4] & 0x60);
                    if (cfg == 0x00) raw = raw & ~7;
                    else if (cfg == 0x20) raw = raw & ~3;
                    else if (cfg == 0x40) raw = raw & ~1;
                    temperature = (float)raw / 16.0;

                    timer_temp.stopTimer();
                }
                else
                    temperature = -127;
            }
        }
    public:
        Pechka(int cooler_in, int moto_shnek_in, int pump_in, int moto_clear_in, int svecha_in, int fotosensor_in, int cur_shnek_in, int cur_pump_in, int cur_clear_in, int cur_svecha_in, int suh_cont_in, int suh_cont_fotosensor_in, int suh_cont_smog_in, uint8_t temp_sensor_in)
        {
            cooler_pech = cooler_in;
            moto_shnek_pech = moto_shnek_in;
            pomp_pech = pump_in;
            moto_clear_pech = moto_clear_in;
            svecha_pech = svecha_in;
            fotosensor_pech = fotosensor_in; 
            cur_shnek_pech = cur_shnek_in;
            cur_pump_pech = cur_pump_in;
            cur_clear_pech = cur_clear_in;
            cur_svecha_pech = cur_svecha_in;
            suh_cont_pech = suh_cont_in;
            suh_cont_fotosensor_pech = suh_cont_fotosensor_in; 
            suh_cont_smog_pech = suh_cont_smog_in;
            temp_sensor_pech = temp_sensor_in;
            ds.begin(temp_sensor_pech);                       //--Устанавливаем в ds18b20 пин 
            timer_temp.setValueTime(2);                       //--Устанавливаем таймер
        }
        void startPechka()                                    //---Инициализация портов
        {
            pinMode(cooler_pech, OUTPUT);
            pinMode(moto_shnek_pech, OUTPUT);
            pinMode(pomp_pech, OUTPUT);
            pinMode(moto_clear_pech, OUTPUT);
            pinMode(svecha_pech, OUTPUT);
            pinMode(fotosensor_pech, OUTPUT);

            pinMode(suh_cont_pech, INPUT_PULLUP);
            pinMode(suh_cont_smog_pech, INPUT_PULLUP);
            pinMode(suh_cont_fotosensor_pech,INPUT);
  
            digitalWrite(cooler_pech, LOW);
            digitalWrite(moto_shnek_pech, LOW);
            digitalWrite(pomp_pech, LOW);
            digitalWrite(moto_clear_pech, LOW);
            digitalWrite(svecha_pech, LOW);
            digitalWrite(fotosensor_pech, LOW);
        }
        void loop(){
            getTempDS();
        }
        bool extinguishFire(){
            if (getStatusSmog()) {
                setValueRele("pomp","on");
                stopRele("clear");
                stopRele("cooler");
                stopRele("shnek");
                stopRele("svecha");
                stopRele("fotosensor");
                flagFire = true;
                return true;
            }
            else {
                setValueRele("pomp","off");
                return false;
                }
        }
        void stopPechcka(){
                flagWorkPechkaStop =true;
        }
        void setMaxTempVal(int val){maxtemp_val = val;}
        int getMaxTempVal(){return maxtemp_val;}

        bool getStatuFire() {return flagFire;}
        void setStatuFire(bool val) { flagFire = false;}

        bool getStatusRele(String name)
        {
            if (name=="cooler"){
                return cooler_value;
            }
            else if (name=="shnek"){
                return shnek_value;
            }
            else if (name=="pomp"){
                return pump_value;
            }
            else if (name=="clear"){
                return clear_value;
            }
            else if (name=="svecha"){
                return svecha_value;
            }
            else if (name=="fotosensor"){
                return fotosensor_value;
            }
            else return false;
        }
//--------------------------------------------------------------------------------------------------------------
        double getCurShnek(){
            return getAverageCur(analogRead(cur_shnek_pech));
        }
        double getCurPump(){
            return getAverageCur(analogRead(cur_pump_pech));
        }
        double getCurClear(){
            return getAverageCur(analogRead(cur_clear_pech));
        }
        double getCurSvecha(){
            return getAverageCur(analogRead(cur_shnek_pech));
        }

        bool getStatusWorkPechka(){
            unsigned long endtime = 0 ;
            if (! bounce_btn && btn1 != digitalRead(suh_cont_pech)) {
                bounce_btn = 1;                               
                endtime = millis();
            }
            else if ( bounce_btn && millis() - endtime >= 50 ) { 
                bounce_btn = 0;
                btn1 = digitalRead(suh_cont_pech) ;
            }
            if((digitalRead(suh_cont_pech) && !bounce_btn) || flagWorkPechkaStop) return false;
            else if(!digitalRead(suh_cont_pech) && !bounce_btn)  return true;
        }

        bool getStatusFotosensor(){
            return digitalRead(suh_cont_fotosensor_pech);
        }
        bool getStatusSmog(){
            if(digitalRead(suh_cont_smog_pech)) return false;
            else return true;
        }
        
        double getTemp(String val){
            if(val == "term")
                return (tempterm.GetAverageTemp(analogRead(temp_sensor_pech))- delta_termistr);
            else if(val == "ds")
                return temperature;
            else
                return -127;
        }
//---------------------------Set time Rele--------------------------------------------
        void setDeltaTemp(double delta_termistr_in){
            delta_termistr = delta_termistr_in;
        }
        bool setTimeRele(String name, unsigned long val){
            if (name == "cooler"){
                timer_cooler.setValueTime(val);
                return 1;
            }
            else if (name == "shnek"){
                timer_shnek.setValueTime(val);
                return 1;
            }
            else if (name == "clear"){
                timer_clear.setValueTime(val);
                return 1;
            }
            else if (name == "svecha"){
                timer_svecha.setValueTime(val);
                return 1;
            }
            return 0;
        }
        bool stopRele(String name){
            if (name == "cooler"){
                if(count_cooler == 0 ){
                    setValueRele("cooler","off");
                    timer_cooler.stopTimer();
                    return 1;
                }
                else if (flagFire){
                    count_cooler = 0;
                    setValueRele("cooler","off");
                    timer_cooler.stopTimer();
                    return 1;
                }
                return 0;
            }
            else if (name == "shnek"){
                setValueRele("shnek","off");
                timer_shnek.stopTimer();
                return 1;
            }
            else if (name == "clear"){
                setValueRele("clear","off");
                timer_clear.stopTimer();
                return 1;
            }
            else if (name == "svecha"){
                setValueRele("svecha","off");
                timer_svecha.stopTimer();
                return 1;
            }
            else if (name=="fotosensor"){
                setValueRele("fotosensor","off");
                return 1;
            }
            return 0;
        }
        bool startRele(String name){
            if (name == "cooler"){
                if(count_cooler == 1 && digitalRead(suh_cont_pech)){
                    if(timer_cooler.startTimer()) setValueRele("cooler","on");
                    else if(!timer_cooler.startTimer()){ 
                        setValueRele("cooler","off");
                        count_cooler = 0;
                    }
                    return 1;
                }
                else if (!digitalRead(suh_cont_pech)){
                    timer_cooler.stopTimer();
                    return 1;
                }
                return 0;
            }
            else if (name == "shnek"){
                if(timer_shnek.startTimer()){ 
                    setValueRele("shnek","on");
                    }
                else if(!timer_shnek.startTimer()){ 
                    setValueRele("shnek","off");
                    return 0;
                    }
                return 1;
            }
            else if (name == "clear"){
                if(timer_clear.startTimer()) setValueRele("clear","on");
                else if(!timer_clear.startTimer()) setValueRele("clear","off");
                return 1;
            }
            else if (name == "svecha"){
                if(timer_svecha.startTimer()) setValueRele("svecha","on");
                else if(!timer_svecha.startTimer()) setValueRele("svecha","off");
                return 1;
            }
            else if (name=="fotosensor"){
                setValueRele("fotosensor","on");
                return 1;
            }
            return 0;
        }
        bool startRele(String name,bool val, uint32_t speed){
            if (name == "cooler" && val){
                setValueRele("cooler",speed);
                count_cooler = 1;
                return 1;
            }
            return 0;
        }
        bool startRele(String name,bool val){
            if (name == "cooler" && val){
                setValueRele("cooler","on");
                count_cooler = 1;
                return 1;
            }
            return 0;
        }
};