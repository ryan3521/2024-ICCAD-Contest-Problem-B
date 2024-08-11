#include "banking.h"



banking::banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->UPFFS = UPFFS;
}


void banking::run(){
    // Steps:
    // 1. initial: for each single bit ff, choose the ff-type such that minimum the cost.
    // 2. for each size of mbff, fist run k-means to form new clusters,
    //    for each new clusters, choose the lowest cost ff type from LIB,
    //    compare cost per bit with the original one which the bit belongs to, if the new mbff cost per bit is lower than the original one, 
    //    "and" also the original cluster member bit have over half of the member bits find new lower cost cluster, than destroy the orignal cluster,
    //    and retain the new cluster, otherwise, destroy the new cluster.

    for(auto ff_list: INST->ffs_sing){
        // Initial begin
        banking_ffs.clear();
        for(auto f: *ff_list) { banking_ffs.push_back(f); }

        // Initial end
    }



    return;
}

void banking::Initial_SingleBit_Cls(){
    for(auto f: banking_ffs){
        ffcell* mincost_ftype = NULL;
        double  mincost = numeric_limits<double>::max(); // = cost per bit (included: tns, area, power)

        for(auto ftype: LIB->fftable_cost[1]){
            double cost = 0;
        }
    }
}