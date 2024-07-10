#ifndef _LIB_CELL_H_
#define _LIB_CELL_H_

#include <iostream>
#include <vector>
#include <string>


using namespace std;




class pin_relpos{ // relative position
    public:
        string name;
        double x_plus;
        double y_plus;
};

// base class
class cell{
    public:

        string name;
        string type; // FlipFlop or Gate
        int pin_num;
        double size_x;
        double size_y;
        double area;

        // member functions
        string get_name();
        string get_typename();
        pair<double, double> get_size();
        virtual void add_pin(string, double, double) = 0;

};

// derive class : gate cell
class gcell : public cell{
    public:
        vector<pin_relpos> in_pins;
        vector<pin_relpos> out_pins;

        // member functions
        gcell(string, string, double, double, int);
        void add_pin(string, double, double);

};

// derive class : ff cell
class ffcell : public cell{
    public:
        int bit_num;
        vector<pin_relpos> d_pins;
        vector<pin_relpos> q_pins;
        pin_relpos clk_pin;
        double Qpin_delay;
        double gate_power;

        // member functions
        ffcell(string, string, int, double, double, int);
        void add_pin(string, double, double);
        void set_CLKpin(string, double, double);
        void set_Qpin_delay(double);
        void set_gate_power(double);
        int get_bit_num();
        double get_Qpin_delay();
        double get_gate_power();
};



#endif