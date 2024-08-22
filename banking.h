#ifndef _BANKING_H_
#define _BANKING_H_

#include "inst.h"
#include "lib.h"
#include "cluster.h"
#include "placement.h"



class banking{
    private:
        bool SUCCESS;
        bool FAIL;
// ----------------------------------------
        placement* PM;
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        list<ffi*>* PFFS;
// ----------------------------------------
        list<list<ffi*>*> ff_groups;
        list<ffi*> placing_ffs;
        list<ffi*> banking_ffs;

// ----------------------------------------
        static bool cmp_ff_x(ffi* a, ffi* b);
// ----------------------------------------
        void CopyOriginalFFs();
        void Initial_Placement();
        void OriginalFFs_Placment();
        void Run_Placement_Banking();

    public:
        banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS);
        void run();

};



#endif