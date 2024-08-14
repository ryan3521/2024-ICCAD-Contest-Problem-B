#ifndef _BANKING_H_
#define _BANKING_H_

#include "inst.h"
#include "lib.h"


class cluster{
    public:
        // Rule 1:
        // if      "is_top" == true,  "to_cluster" == NULL;
        // else if "is_top" == false, "to_cluster" point to cluster.
        
        // Rule 2:
        // if "size" == 1, "single bit ff" != NULL;
        // else "single bit ff" == NULL.

        // Rule 3:
        // if "size" != 1, size of clusters in member list will only be "1".
        int  size;
        bool is_top;
        cluster* to_top_cluster;
        cluster* to_new_cluster;
        double cost_per_bit; 
        list<cluster*> members;
        ffcell* type;
        ffi* single_bit_ff;
        double old_coox;
        double old_cooy;
        double coox;
        double cooy;
        bool is_better_than_new;
        bool mark_remove; // for top cluster
        bool mark_recalculate; // for new cluster
        list<pin*> optseq_D;
        list<pin*> optseq_Q;

        // Member function
        bool UpdateCoor_CheckMove();
        void AddMember(cluster* new_member);
        void Clear();
        void Calculate_BestCost_FFtype(lib* LIB, inst* INST, dieInfo* DIE);
        bool TestQuality(); // for top cluster
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
        list<cluster*> single_bit_clusters;


        void Initial_SingleBit_Cls(); 
        static bool sort_single_bit_cls(cluster* a, cluster* b);

    public:
        banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS);
        void run();

};



#endif