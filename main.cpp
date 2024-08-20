#include <bits/stdc++.h>
#include <ctime>

#include "func.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "placement.h"
#include "banking.h"
#include "costeva.h"

using namespace std;



int main(int argc, char** argv){

    double start = clock();
    lib LIB;
    inst INST;
    dieInfo DIE;
    netlist NL;
    placement PM(&LIB, &INST, &DIE);
    costeva COST(&DIE, &LIB, &INST, argv[1]);
    list<ffi*> NCLS; // Store the non cluster ffs after doing K-means Cluster
    list<ffi*> MBFFS; // Store the new MBFFs after doing K-means Cluster
    list<ffi*> UPFFS; // Store all ffs which need to be placed (== NCLS + MBFFS)
    list<ffi*> PFFS; // Store all ffs which are placed
    banking FFBANK(&INST, &LIB, &DIE, &UPFFS);

    srand(time(NULL));
    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);

    LIB.construct_fftable(DIE);
    INST.SlackDispense(DIE);
    INST.DebankAllFF(LIB);

    FFBANK.run();
    FFBANK.PrintResult();



    // PM.placeGateInst();
    // PM.placeFlipFlopInst( UPFFS, PFFS);
    // Output(argv[2], PFFS, INST);

    double end = clock();

    COST.PrintParameter();
    COST.InitialCost();
    COST.ResultCost(&UPFFS);

    cout << endl << "Total execution time: " << (end - start) / 1000000.0  << " s" << '\n';

    // DrawFFs(DIE, LIB, INST, UPFFS);
    return 0;
}



