#ifndef _MODIFYCLS_H_
#define _MODIFYCLS_H_

#include "inst.h"
#include "lib.h"

class cls{
    public:
        int size_limit;
        int size;
        double pos_x;
        double pos_y;
        list<ffi*> memb_ffs; 
        list<ffi*> cand_ffs;
        list<ffi*> cand_canmerge_ffs;
        list<ffi*> cand_cannotmerge_ffs;

        double fsr_xmin;
        double fsr_xmax;
        double fsr_ymin;
        double fsr_ymax;

        int potential_cnt;
        int potential_score;
        bool have_potential;

        // Member functions
        cls(int size_limit, double pos_x, double pos_y);
        void update_loc();
        void clear_ffs();
        void add_ff(ffi* f);
        bool can_merge(ffi* f, double& hpwl_diff);
};

class banking{
    private:
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        list<ffi*>* UPFFS;
        list<cls*> CLS; 
        list<ffi*> NCLS;

        struct MultiClsFF{
            ffi* f;
            cls* to_cls;
            bool is_memb;
        };

        void modifyKmeans();
        void cls_to_mbff();
        
        

    public:
        banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS);
        void run();

};



#endif