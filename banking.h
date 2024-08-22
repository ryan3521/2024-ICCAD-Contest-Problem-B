#ifndef _BANKING_H_
#define _BANKING_H_

#include "inst.h"
#include "lib.h"
#include "cluster.h"
#include "placement.h"



class banking{
    private:
        placement* PM;
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        list<ffi*>* PFFS;
// ----------------------------------------
        list<list<ffi*>*> ff_groups;
        list<ffi*> banking_ffs;

// ----------------------------------------
        void CopyOriginalFFs();
        void Initial_Placement();
        void Run_Placement_Banking();

    public:
        banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS);
        void run();

};



#endif