#ifndef _INST_H_
#define _INST_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <limits>

#include "lib.h"
#include "die_info.h"

using namespace std;


class pin;
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
        

        double allow_dis; // (HPWL) AVG slack * displacement coefficient



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

        // Given coordinate x and y, base on this coordinate calculate if the negative slack pin numbers are more then a half
        // if the neg pin numbers are over the half, then return true, else false
        bool is_too_far(double x, double y, double displacement_delay);
        bool allow_displace(double x, double y, double displacement_delay);

};

class reg{ // This class represent each bit of register in the design
    public:
        pin* dpin;
        pin* qpin;
        double cen_x;
        double cen_y;

        // member function
        void update_cen();
};

class gatei{
    private:
        bool v; // mark v = 1 if this gate is visited
        double critical_slack;

    public:
        string name;
        gcell* type;
        double coox;
        double cooy;
        vector<pin*> ipins;
        vector<pin*> opins;


        gatei(string, double, double);
        void set_type(gcell* );
        string get_name();
        gcell* get_type();
        void initial_PinInfo();
        void visit(double critical_slack);
        bool is_visited();
        double get_critical_slack();

};


class net{
    public:
        string name;
        list<pin*> ipins;
        list<pin*> opins;

        net(string net_name){
            name = net_name;
            ipins.clear();
            opins.clear();
        }
};

class pin{ // pin prototype
    public:
        string name;
        net* to_net;
        gatei* to_gate;
        ffi* to_ff;
        char pin_type;      // f: flip flop pin; g: gate pin; d: die pin;
        double coox;
        double cooy;
        bool io;            // Only for pin type is 'd', 0 stand for IN; 1 stand for OUT;
        double slack;       // Only for pin type is 'f', and is belongs to "d_pins"
        double dspd_slk;    // Only for pin type is 'f', the dispensed slack for 'D' and 'Q' pin 

        // Belongs to the new MBFF
        string new_name;
        ffi* to_new_ff;
        double new_coox;
        double new_cooy;
        pin(){
            to_net  = NULL;
            to_gate = NULL;
            to_ff   = NULL;
            to_new_ff = NULL;
        }
};


class inst{
    public:
        int ff_num;
        unordered_map<string, bool> type_umap; // 0: ff, 1: gate
        unordered_map<string, ffi* > ff_umap;
        unordered_map<string, gatei* > gate_umap;

        // member functions        
        inst();
        void add_inst(lib& LIB, string inst_name, string typename_, double coox, double cooy);
        void set_TSlack(string inst_name, string pin_name, double slack);
        void SlackDispense_Q(dieInfo& DIE);
        void PrintFF();
        void PrintGate();

};

double get_critical_slack(net* net_ptr);
#endif