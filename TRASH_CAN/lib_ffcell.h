#include <iostream>
#include <vector>
#include <string>

using namespace std;

#ifndef _LIB_FFCELL_H_
#define _LIB_FFCELL_H_

class ffcell{
    private:
        typedef struct pin{
            string name;
            double coox;
            double cooy;
        } pin;

        string name;
        int bit_num;
        int pin_num;
        double size_x;
        double size_y;
        vector<pin> d_pins;
        vector<pin> q_pins;
        pin clk_pin;
        double Qpin_delay;
        double gate_power;



    public:
        ffcell(string, int, double, double, int);
        void add_Dpin(string, double, double);
        void add_Qpin(string, double, double);
        void set_CLKpin(string, double, double);
        void set_Qpin_delay(double);
        void set_gate_power(double);
        string get_name();
        int get_bit_num();
        pair<double, double> get_size();
        double get_Qpin_delay();
        double get_gate_power();


};

#endif