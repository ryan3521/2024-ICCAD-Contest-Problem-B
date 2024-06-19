#ifndef _NETLIST_H_
#define _NETLIST_H_

#include <iostream>
#include <vector>
#include <string>
#include <list>

#include "inst.h"
#include "die_info.h"


using namespace std;



class netlist{
    public:
        int net_num;
        net* new_net;
        vector<net*> nets;

        // DIE IO PINS
        int DI_num; // (Die Input Num)
        unordered_map<std::string, pin*> Diopins_umap; // (Die IO Pins unordered)

        netlist();
        void set_net_num(int n);
        void add_net(string net_name);
        void add_pin(string str, inst& INST);
        void add_Di_pin(string, double, double);
        void add_Do_pin(string, double, double);
};





#endif
