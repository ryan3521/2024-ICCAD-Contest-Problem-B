#include "modifycls.h"

cls::cls(){
    size  = 0;
    pos_x = 0;
    pos_y = 0;
    memb_ffs.clear();
    cand_ffs.clear();
}

void update_loc(){
    // 1. If memb_ffs size is 0, check cand_ffs list do INTEGRA.
    //    If:   INTEGRA find new cluster, create new cluster. Delete new cluster.
    //    ELSE: Delete this cluster.

    // 2. If memb_ffs size not zero, do INTEGRA 

    // 3. If memb_ffs size is max ff size. 
    
}


banking::banking(inst* INST, lib* LIB, dieInfo* DIE){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
}

void banking::modifyKmeans(){
    list<cls*> cluster_list;
    // Step 1. Initial clusters number and location.

    // Step 2. Do cluster.
}