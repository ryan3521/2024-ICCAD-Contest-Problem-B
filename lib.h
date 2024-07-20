#ifndef _LIB_H_
#define _LIB_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <utility>

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
        vector<int> ffmapping_list; // if ff bit_size=b exist ffmapping_list[b]=-1, else ffmapping_list[b]=next big_ff bit_size
        vector<int> mincost_list;
        double find_min_cost(int bit, dieInfo& DIE);
        void construct_optlist(int bit, list<ffcell*>& opt_list);
        void check_opt_result();
        vector<pair<int, int>> best_choice;
    public:
        int max_ff_size;
        // The "fftable_cost" only record the MBFF cell which truely exist in the given cell library
        // if the list size in the fftable is 0, this mean the corresponding bit size MBFF doesn't exist in the LIB.
        // the list of each bit size is sorted by "cost per bit" (power*Beta + area*Gamma)/bit
        vector<list<ffcell*>> fftable_cost;
        vector<double> mbff_cost;

        // The "fftable_c2q" only record the MBFF cell which truely exist in the given cell library
        // if the list size in the fftable is 0, this mean the corresponding bit size MBFF doesn't exist in the LIB.
        // the list of each bit size is sorted by "Qpin_delay"
        vector<list<ffcell*>> fftable_c2q;
        vector<list<ffcell*>> fftable_area;

        // the "opt_fftable" will store the best MBFF configuration for each bit size
        // (the optimize base on the "cost per bit")
        // ex: bit size 7 can be composed of 1+2+4 ... 
        vector<list<ffcell*>> opt_fftable; // optimize ff table

        unordered_map<std::string, cell* > cell_umap;


    
        lib();
        void add_cell(cell* new_cell);
        void set_Qpin_delay(string, double);
        void set_gate_power(string, double);
        void construct_fftable(dieInfo& DIE);
        cell* get_cell(string name);

        void Print_member();


};

#endif