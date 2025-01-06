#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include "inst.h"
#include "lib.h"
#include "legalizer.h"


class comb{
    private:
    public:
        int size;
        list<ffi*> members;

        list<pin*> optseq_D;
        list<pin*> optseq_Q;

        ffcell* type;
        double cost_per_bit;

        comb();
        void AddMember(ffi* f);
        void Calculate_BestCost_FFtype(bool print, lib* LIB, inst* INST, dieInfo* DIE);
        bool TestQuality(bool print, Legalizer* LG, dieInfo* DIE);
        ffi* GetNewFF();
};

class cluster{
    private:
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        static bool cmp_cost(comb* a, comb* b);
        static bool cmp_dist(ffi* a, ffi* b);
    public:
        int size;

        ffi* essential_ff;
        list<ffi*> related_ffs; 
        list<ffi*> members; 
        ffcell* type;

        list<comb*> comb_list;

        // Member function
        cluster(inst* INST, lib* LIB, dieInfo* DIE);
        void AddMember(ffi* new_member);
        void Clear();
        void ConstructCombs(int target_size);
};

#endif