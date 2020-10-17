#include <Arduino.h>

class MyNextion{
    private:
        bool serialReadFlag = false;
        bool stringReadFlag=false;
        bool switshflag =false;
        HardwareSerial & _serial = Serial;
        
        String incStr = ""; 
        String data_param="";
        String data_value="";

        void getDataUart(){

            if(_serial.available() > 0) {
                uint8_t inn = _serial.read();
                if(inn == 35){
                    serialReadFlag = true;
                    incStr = "";
                }
                if(serialReadFlag && inn != 59 && inn != 35){
                    incStr += (char)inn;
                }
                else if(serialReadFlag && inn == 59){
                    AnalyseString(incStr);
                    serialReadFlag = false;
                }
            }
        }

        bool AnalyseString(String incStr) {
            data_param="";
            data_value="";

            for (int i = 0; i <= incStr.length(); i++){
                if(incStr[i]== 61){
                    stringReadFlag=true;
                }
                if(!stringReadFlag){
                    data_param += incStr[i];
                    if(data_param=="switch") switshflag=true;
                }else if(stringReadFlag){
                    data_value += incStr[i];
                }
            }
            return true;
        }

    public:
        MyNextion(HardwareSerial & serial){
            _serial = serial;
        }
        void loop(){
            getDataUart();
        }
        void sendDataToNextion(String data,String val){
            _serial.print(data);
            _serial.print("=");
            _serial.print("\""+ val +"\"");
            _serial.write(0xff);
            _serial.write(0xff);
            _serial.write(0xff);
        }

        void sendDataToNextionVal(String data,String val){
            _serial.print(data);
            _serial.print("=");
            _serial.print(""+val+"");
            _serial.write(0xff);
            _serial.write(0xff);
            _serial.write(0xff);
        }
};