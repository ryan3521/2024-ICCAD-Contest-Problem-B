#include <bits/stdc++.h>
#include <ctime>

#include "funcs.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "placement.h"

using namespace std;

int main(int argc, char** argv){

    double start = clock();
    double end;
    lib LIB;
    inst INST;
    dieInfo DIE;
    netlist NL;
    placement PM(&LIB, &INST, &DIE);
    list<ffi*> MBFFs;

    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);

    LIB.construct_fftable(DIE);
    INST.CalCriticalPath(DIE.displacement_delay);

    ReadOutput(argv[2], LIB, INST, DIE, NL, PM, MBFFs);
    Analyzer(LIB, INST, DIE, NL, MBFFs);
    DrawFFs(DIE, LIB, INST, MBFFs);
    DrawGates( DIE, LIB, INST);
    return 0;
}



