#ifndef _LIB_H_
#define _LIB_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <utility>
#include <limits>

#include "die_info.h"

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
    private:
        int in_cnt;
        int out_cnt;
        unordered_map<std::string, int> idx_umap; 
    public:
        vector<pin_relpos> in_pins;
        vector<pin_relpos> out_pins;
        // member functions
        gcell(string, string, double, double, int);
        void add_pin(string, double, double);
        int get_PinIDX(string);

};

// derive class : ff cell
class ffcell : public cell{
    private:
        int d_cnt;
        int q_cnt;
        unordered_map<std::string, int> idx_umap; 
    public:
        int bit_num;
        vector<pin_relpos> d_pins;
        vector<pin_relpos> q_pins;
        pin_relpos clk_pin;
        double Qpin_delay;
        double gate_power;
        double cost_per_bit;
        double fsr_min_w;
        double fsr_min_h;


        // member functions
        ffcell(string, string, int, double, double, int);
        void add_pin(string, double, double);
        void set_CLKpin(string, double, double);
        void set_Qpin_delay(double);
        void set_gate_power(double);
        int get_bit_num();
        int get_PinIDX(string);
        double get_Qpin_delay();
        double get_gate_power();
};


class lib{
    private:
        static bool cmp_clk2Q(ffcell* a, ffcell* b);
        static bool cmp_APcost(ffcell* a, ffcell* b);
        static bool cmp_area(ffcell* a, ffcell* b);
       
    public:
        int max_ff_size;
        int min_ff_size;

        unordered_map<std::string, cell* > cell_umap;
    
        vector<list<ffcell*>> fftable_cost;
        vector<list<ffcell*>> fftable_c2q;
        vector<list<ffcell*>> fftable_area;

        lib();
        void add_cell(cell* new_cell);
        void set_Qpin_delay(string, double);
        void set_gate_power(string, double);
        void construct_fftable(dieInfo& DIE);
        cell* get_cell(string name);


};

#endif