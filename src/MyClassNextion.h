#include <Arduino.h>

class Nextion{

    private:

        bool serialReadFlag = false;
        bool switshflag =false;
        
        String incStr = ""; 
        String a_param = "";
        String a_value = "";
        String data_param="";
        String data_value="";

        void getDataUart(){
            
            if(Serial.available() > 0) {
                uint8_t inn = Serial.read();
                if(serialReadFlag) {
                    if(inn == 59){
                        if(incStr.length() > 0) {
                            AnalyseString(incStr);
                            }
                        serialReadFlag = false;
                    } else {
                        incStr += (char)inn;
                    }
                } else {
                    if(inn == 35){
                        serialReadFlag = true;
                        incStr = "";
                    }
                }
            }
        }

        void AnalyseString(String incStr) {

            for (int i = 0; i <= incStr.length(); i++){

                if(!stringReadFlag){
                    if(incStr[i]!= 61){
                        a_param += incStr[i];
                        if(a_param=="switch") switshflag=true;
                    }
                    else{
                        stringReadFlag=true;
                    }
                }
                else{
                    a_value+= incStr[i];
                }
            }

        data_param = a_param;
        data_value = a_value; }

    public:

};