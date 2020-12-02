#include <MyClassNextion.h>

class MyNextion;

MyNextion::MyNextion(HardwareSerial & serial){
    _serial = serial;
}

bool MyNextion::getDataUart(){
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
            if(AnalyseString(incStr)){
                    serialReadFlag = false;
                    return true;
            };
        }
    }
    return false;
}

bool MyNextion::AnalyseString(String incStr) {
    data_param="";
    data_value="";

    for (int i = 0; i <= incStr.length(); i++){
        if(incStr[i]== 61){
            stringReadFlag=true;
        }
        if(!stringReadFlag && incStr[i]!= 61){
            data_param += incStr[i];
            if(data_param=="switch") switshflag=true;
        }else if(stringReadFlag && incStr[i]!= 61){
            data_value += incStr[i];
        }
    }
    return true;
}

bool MyNextion::loop(){
        return getDataUart();
}
String MyNextion::getDataParam(){
    return data_param;
}
String MyNextion::getDataValue(){
    return data_value;
}
void MyNextion::sendDataToNextionStr(String data,String val){
    _serial.print(data);
    _serial.print("=");
    _serial.print("\""+ val +"\"");
    _serial.write(0xff);
    _serial.write(0xff);
    _serial.write(0xff);
}
void MyNextion::sendDataToNextionVal(String data,String val){
    _serial.print(data);
    _serial.print("=");
    _serial.print(""+val+"");
    _serial.write(0xff);
    _serial.write(0xff);
    _serial.write(0xff);
}