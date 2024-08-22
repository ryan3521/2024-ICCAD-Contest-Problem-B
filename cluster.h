#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include "inst.h"
#include "lib.h"


class cluster{
    private:

    public:
        // Rule 1:
        // if      "is_top" == true,  "to_cluster" == NULL;
        // else if "is_top" == false, "to_cluster" point to cluster.
        
        // Rule 2:
        // if "size" == 1, "single bit ff" != NULL;
        // else "single bit ff" == NULL.

        // Rule 3:
        // if "size" != 1, size of clusters in member list will only be "1".
        int size;
        double cost_per_bit; 
        ffi* essential_ff;
        list<ffi*> related_ffs; 
        list<ffi*> members; 
        ffcell* type;

        list<pin*> optseq_D;
        list<pin*> optseq_Q;

        // Member function
        cluster();
        void AddMember(ffi* new_member);
        void Clear();
        // void Calculate_BestCost_FFtype(bool print, lib* LIB, inst* INST, dieInfo* DIE);
        // bool TestQuality(bool print); // for top cluster

};

#endif