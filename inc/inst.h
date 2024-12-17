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
class ffi;

class se{
    public:
        int type;
        double coor;
        ffi* to_ff;
        se(int type, double coor, ffi* to_ff);
};

class block{
    public:
        double xmin;
        double xmax;
        double ymin;
        double ymax;

};


class ffi{
    public:
        int size;
        string name;
        ffcell* type;
        double coox;
        double cooy;
        double cen_x;
        double cen_y;
        vector<pin*> d_pins;
        vector<pin*> q_pins;
        pin* clk_pin;

        double x_allow_dis; 
        double y_allow_dis; 

        list<ffi*>* to_list;
        list<ffi*>::iterator it_pointer;
        int index_to_placement_row;
        int index_to_site;

        block pseudo_block;
        list<se*>::iterator e_it;
        list<ffi*>::iterator x_track_list_it;
        list<ffi*>::iterator y_track_list_it;
        double dist_to_essential;
        double cost;
        list<ffi*> members;
        bool no_neighbor;
        


        // member function
        ffi(string, double, double);
        void set_type(ffcell* );
        void set_TSlack(string, double);
        pair<double, double> get_coor();
        string get_name();
        ffcell* get_type();
        void initial_PinInfo();
        void update_coor();
        void update_pin_loc();
        void Set_PseudoBlock_Size(double expand_rate);
        void CalculateCost(double alpha, double beta, double gamma, double displacement_delay);
        void getCriticalPath(int mode, double displacement_delay); // mode 0: original critical path , mode 1: new critical path
        double get_timing_cost(double x, double y, double displacement_delay);
};

class gatei{
    private:
        double critical_slack;

    public:
        bool v; // mark v = 1 if this gate is visited
        string name;
        gcell* type;
        double coox;
        double cooy;
        vector<pin*> ipins;
        vector<pin*> opins;
        double consume_time; // is used to trace the negative slack
        bool is_tracking;
        double min_cs; // smallest critical slack
        double criticalPath_HPWL;


        gatei(string, double, double);
        void set_type(gcell* );
        string get_name();
        gcell* get_type();
        void initial_PinInfo();
        void visit(double critical_slack);
        bool is_visited();
        double get_critical_slack();
        double getCriticalPath(int mode, double displacement_delay);

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
    private:
        struct LinkedPinInfo{
            pin* targetFloatPin; 
            pin* relatedFixedPin;
            double FixedHPWL;
        };

        list<LinkedPinInfo*> connected_SourcePins; // only for D pin
        void FindLinkedPin(gatei* g, double current_FixedHPWL, list<LinkedPinInfo*>& connected_SourcePins);
    public:
        string name;
        net* to_net;        // net that current pin belongs to
        gatei* to_gate;     // gate that current pin belongs to
        ffi* to_ff;         // FF that current pin belongs to
        char pin_type;      // f: flip flop pin; g: gate pin; d: die pin;
        double coox;
        double cooy;
        bool io;            // Only for pin type is 'd', 0 stand for IN; 1 stand for OUT;
        double slack;       // Only for pin type is 'f', and is belongs to "d_pins"
        double dspd_slk;    // Only for pin type is 'f', the dispensed slack for 'D' and 'Q' pin 
        double criticalPath_HPWL;
        // Below are the variable newed by your friend Yuri
        bool isVisited;

        // Belongs to the new MBFF
        string new_name;
        ffi* to_new_ff;
        double new_coox;
        double new_cooy;
        double new_criticalPath_HPWL;


        pin(){
            to_net  = NULL;
            to_gate = NULL;
            to_ff   = NULL;
            to_new_ff = NULL;
            isVisited = false;
        }

        double CalTns(double new_coox, double new_cooy, bool is_D, ffcell* new_type, double coeff);
        void updateCriticalHPWL(double displacementDelay);
        void ConstructConnectedSourcePins();
};


class inst{
    private:
        static bool preference_cmp(pair<int, double> a, pair<int, double> b);

        struct pin_pair{
            int idx;
            pin* dpin;
            pin* qpin;
            list<pair<int, double>> preference_list; // int: port index, double: slack
        };
        struct port_pair{
            double slack;
            pin_pair* like_most_pin_pair;
            list<pair<double, list<pin_pair*>::iterator>> choices_list;

        };
    public:
        int ff_num;
        unordered_map<string, bool> type_umap; // 0: ff, 1: gate
        unordered_map<string, ffi* > ff_umap;
        unordered_map<string, gatei* > gate_umap;
        list<list<ffi*>*> ffs_ori;
        list<list<ffi*>*> ffs_sing; // debank

        // member functions        
        inst();
        void add_inst(lib& LIB, string inst_name, string typename_, double coox, double cooy);
        void set_TSlack(string inst_name, string pin_name, double slack);
        void SlackDispense(dieInfo& DIE);
        void CalCriticalPath();
        void PrintFF();
        void PrintGate();
        void DebankAllFF(lib& LIB);
        double TnsTest(bool print, list<pin*>& dpins, list<pin*>& qpins, ffcell* type, double coeff, list<pin*>& optseq_D, list<pin*>& optseq_Q);

};

double get_critical_slack(net* net_ptr);
double get_min_cs(gatei* g);
#endif