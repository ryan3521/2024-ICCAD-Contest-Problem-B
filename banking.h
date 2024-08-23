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
        list<se*>  xseq;
        list<se*>  yseq;
        list<ffi*> tracking_list;
        cluster* cls;
        int target_size;
        ffi* essential_ff;
        comb* target_comb;
// ----------------------------------------
        static bool cmp_ff_x(ffi* a, ffi* b);
        static bool cmp_se(se* a, se* b);
// ----------------------------------------
        void CopyOriginalFFs();
        void Initial_Placement();
        void OriginalFFs_Placment();
        void InitialFFsCost();
        void Run_Placement_Banking();
        void SetPseudoBlock();
        void ConstructXSequence();
        void ConstructYSequence();
        bool FindNewCluster();
        void FindRelatedFF();
        void FindBestCombtoPlace();
    public:
        banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS);
        void run();

};



#endif