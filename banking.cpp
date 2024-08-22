#include "banking.h"

banking::banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS){
    this->PM   = PM;
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->PFFS = PFFS;
}

void banking::Initial_Placement(){
    PM->GatePlacement();

}

void banking::Run_Placement_Banking(){
    



    return;
}

void banking::run(){


    return;
}

