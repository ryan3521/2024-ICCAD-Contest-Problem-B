#include <bits/stdc++.h>
#include <ctime>

#include "func.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "banking.h"
#include "costeva.h"
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
    costeva COST(&DIE, &LIB, &INST, argv[1]);
    list<ffi*> PFFS; // Store all ffs which are placed
    list<ffi*> UPFFS;
    banking FFBANK(&PM, &INST, &LIB, &DIE, &PFFS);

    srand(time(NULL));
    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);

    LIB.construct_fftable(DIE);
    // INST.SlackDispense(DIE);
    INST.CalCriticalPath(DIE.displacement_delay);
    INST.DebankAllFF(LIB);





    FFBANK.run();

    Output(argv[2], PFFS, INST);

    end = clock();

    COST.PrintParameter();
    COST.InitialCost();
    COST.ResultCost(&PFFS);

    cout << endl << "Total execution time: " << (end - start) / 1000000.0  << " s" << '\n';

    DrawFFs(DIE, LIB, INST, UPFFS, PFFS);
    DrawGates( DIE, LIB, INST);
    return 0;
}



