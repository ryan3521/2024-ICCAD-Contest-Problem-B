#include <iostream>
#include <vectot>
#include <string>

using namespace std;

#ifndef _LIB_GCELL_H_
#define _LIB_GCELL_H_

class gcell{
    private:
        typedef struct pin{
            string name;
            double coox;
            double cooy;
        } pin;

        string name;
        int pin_num;
        double size_x;
        double size_y;
        vecotr<pin> in_pins;
        vector<pin> out_pins;


    public:
        gcell(string, double, double, int);
        void add_pin(string, double, double);
        

}


#endif