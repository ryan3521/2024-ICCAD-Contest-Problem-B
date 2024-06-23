#ifndef _INST_FF_H_
#define _INST_FF_H_

#include <iostream>
#include <vector>
#include <string>

#include "lib_cell.h"
#include "pin_net.h"


using namespace std;


class ffi{
    public:
        string name;
        ffcell* type;
        double coox;
        double cooy;
        double cen_x;
        double cen_y;
        vector<pin*> d_pins;
        vector<pin*> q_pins;
        pin* clk_pin;
        bool mark; // ??
        double allow_displace; // (HPWL) AVG slack * displacement coefficient



        // member function
        ffi(string, double, double);
        void set_type(ffcell* );
        void set_TSlack(string, double);
        pair<double, double> get_coor();
        string get_name();
        ffcell* get_type();
        void initial_PinInfo();
        
        // Function "new_coor()" will calculate the coox, cooy according to the d_pins and q_pins "old coordinate"
        // After find out the new coox, cooy; It will also calculate the new coox and cooy for each pin (D and Q)
        void new_coor();
};

class reg{ // This class represent each bit of register in the design
    public:
        pin* dpin;
        pin* qpin;
        double cen_x;
        double cen_y;

        // member function
        void update_cen();
}
#endif 