#ifndef _MODIFYCLS_H_
#define _MODIFYCLS_H_

#include "inst.h"
#include "lib.h"

class cls{

};

class banking{
    private:
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;

    public:
        banking(inst* INST, lib* LIB, dieInfo* DIE);
        void run();

};



#endif