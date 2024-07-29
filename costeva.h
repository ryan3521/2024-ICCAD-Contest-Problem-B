#ifndef _COSTEVA_H_
#define _COSTEVA_H_

#include "lib.h"
#include "inst.h"
#include "die_info.h"

using namespace std;

class costeva{
    private:
        dieInfo* DIE;
        lib* LIB;
        inst* INST;
        list<ffi*>* ffs;

        double calArea();
        double calPower();
        double calTns();
        double calDensity();

    public:
        costeva(dieInfo* DIE, lib* LIB, inst* INST);
        double evaluate(list<ffi*>* ff_list);

};

#endif