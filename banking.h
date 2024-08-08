#ifndef _MODIFYCLS_H_
#define _MODIFYCLS_H_

#include "inst.h"
#include "lib.h"

class cls{
    public:
        int size;
        list<ffi*> pos_slack_members;
        list<ffi*> neg_slack_members;

};

class banking{
    private:
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        list<ffi*>* UPFFS;
        list<pair<int ,double>> size_priority;
        list<ffi*> banking_ffs;
        list<ffi*> ncls_ffs;
        list<cls*> clusters;


        void initial_size_priority();
        // ********* INTEGRA ********* //
        struct se;
        
        struct ff2se{
            ff2se(ffi* f_): f{f_} {} 

            ffi* f;
            list<ff2se*>::iterator xtrack_it;
            list<ff2se*>::iterator ytrack_it;
            list<se*>::iterator x_seq_e_it;
        };

        struct se{
            se(int t, double coo, ff2se* tf): type{t}, coor{coo}, to_ff{tf} {}

            int type; // 0: start, 1: end
            double coor;
            ff2se* to_ff;
        };

        static bool cmp_se(se* a, se* b);
        static bool cmp_ff2se(ff2se* a, ff2se* b);
        void integra(int target_size);

              

    public:
        banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS);
        void run();

};



#endif