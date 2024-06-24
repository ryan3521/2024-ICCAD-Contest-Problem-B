#include <bits/stdc++.h>
#include <ctime>

#include "func.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "placement.h"
#include "cluster.h"

using namespace std;

int main(int argc, char** argv){
    double start = clock();
    lib LIB;
    inst INST;
    dieInfo DIE;
    netlist NL;
    placement PM;
    list<cluster*> KCR; // Kmeans Cluster Result
    list<ffi*> NCLS; // Store the non cluster ffs after doing K-means Cluster
    list<ffi*> MBFFS; // Store the new MBFFs after doing K-means Cluster
    list<ffi*> UPFFS; // Store all ffs which need to be placed (== NCLS + MBFFS)
    list<ffi*> PFFS; // Store all ffs which are placed



    srand(time(NULL));
    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);
    LIB.construct_fftable(DIE);
    INST.SlackDispense_Q(DIE);


    // KmeansCls(LIB, INST, KCR, NCLS);  
    // MapClstoMBFF(LIB, KCR, MBFFS);

    for(auto& it: INST.ff_umap){
        NCLS.push_back(it.second);
    }

    FineTune(LIB, NCLS, MBFFS, UPFFS, DIE); // Not finish yet
    PM.placeGateInst(INST);
    PM.placeFlipFlopInst(LIB, INST, DIE, UPFFS, PFFS);
    Output("output.txt", PFFS, INST);





    // ffi* fi;
    // for(auto& it: INST.ff_umap){
    //     UPFFS.push_back(it.second);
    //     fi = it.second;
    //     for(auto& p: fi->d_pins){
    //         p->to_new_ff = p->to_ff;
    //         p->new_coox = p->coox;
    //         p->new_cooy = p->cooy;
    //     }
    //     for(auto& p: fi->q_pins){
    //         p->to_new_ff = p->to_ff;
    //         p->new_coox = p->coox;
    //         p->new_cooy = p->cooy;
    //     }
    // }

    double end = clock();
    cout << "Total execution time: " << (end - start) / 1000000.0  << " s" << '\n';

    // If placement success ...
    // Output PFFS Info 
    // Done 

    
    return 0;
}



