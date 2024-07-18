#ifndef _MODIFYCLS_H_
#define _MODIFYCLS_H_

#include "inst.h"
#include "lib.h"

class cls{
    public:
        int size;
        double pos_x;
        double pos_y;
        list<ffi*> memb_ffs; 
        list<ffi*> cand_ffs;

        // Member functions
        cls();
        void update_loc();
};

class banking{
    private:
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        void modifyKmeans();
        

    public:
        banking(inst* INST, lib* LIB, dieInfo* DIE);
        void run();

};



#endif