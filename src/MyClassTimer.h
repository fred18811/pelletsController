
#include <Arduino.h>

class MyTimer{
    private:
        unsigned long timing = 0;
        unsigned long value = 1;
        bool timer_work = true;
        bool timer_initialization = true;

    public:
        MyTimer(unsigned long val = 1)
        {
            value = val;
        }
        void setValueTime(unsigned long val){
            value = val;
        }
        bool startTimer(){
            if(timer_initialization){
                timing = millis();
                timer_initialization = false;
            }
            if(timer_work)
            {
                if (millis() - timing > value*1000){
                    timer_work = false;
                    return false;
                }
                else return true;
            }
            else return false;
        }
        void stopTimer(){
            timer_work = true;
            timer_initialization = true;
        }
};