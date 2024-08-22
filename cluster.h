#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include "inst.h"
#include "lib.h"


class cluster{
    private:
        static bool cmp(cluster* a, cluster* b);
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
        list<cluster*> members; // sort by displace_distance decending order
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
        double displace_distance;

        // Member function
        cluster();
        bool UpdateCoor_CheckMove();
        void AddMember(cluster* new_member);
        void Clear();
        void Calculate_BestCost_FFtype(bool print, lib* LIB, inst* INST, dieInfo* DIE);
        bool TestQuality(bool print); // for top cluster
        void SortMembers();
};

#endif