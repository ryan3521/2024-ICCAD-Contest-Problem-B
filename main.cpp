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
    double orig_area  = 0;
    double orig_power = 0;
    double opt_area = 0;
    double opt_power = 0;
    double ori_cost = 0;
    double opt_cost = 0;
    int ori_bitnum = 0;
    int aft_bitnum = 0;



    srand(time(NULL));
    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);
    LIB.construct_fftable(DIE);
    INST.SlackDispense_Q(DIE);
    for(auto it: INST.ff_umap){
        auto f = it.second;
        orig_area = orig_area + f->type->area;
        orig_power = orig_power + f->type->gate_power;
        ori_bitnum = ori_bitnum + f->d_pins.size();
    }

    KmeansCls(DIE, LIB, INST, KCR, NCLS);  
    MapClstoMBFF(LIB, KCR, MBFFS);

    // for(auto fi: INST.ff_umap){
    //     NCLS.push_back(fi.second);
    // }

    FineTune(LIB, NCLS, MBFFS, UPFFS, DIE); // Not finish yet
    PM.placeGateInst(INST);
    PM.placeFlipFlopInst(LIB, INST, DIE, UPFFS, PFFS);

    for(auto f: PFFS){
        opt_area = opt_area + f->type->area;
        opt_power = opt_power + f->type->gate_power;
        aft_bitnum = aft_bitnum + f->d_pins.size();
    }
    Output("output.txt", PFFS, INST);


    double end = clock();
    cout << "Total execution time: " << (end - start) / 1000.0  << " s" << '\n';

    if(ori_bitnum == aft_bitnum)
        cout << endl << "- bit num match -" << endl;
    else 
        cout << endl << "error: bit num not match" << endl;

    ori_cost = DIE.Beta*orig_power + DIE.Gamma*orig_area;
    opt_cost = DIE.Beta*opt_power + DIE.Gamma*opt_area;


    cout << endl;
    cout << "===============================" << endl;
    cout << "Ori Power: " << orig_power << endl;
    cout << "Opt Power: " << opt_power << endl;
    cout << "Reduce: " << 100*(orig_power - opt_power)/orig_power << " %" << endl;
    cout << "-------------------------------" << endl;
    cout << "Ori Area: " << orig_area << endl;
    cout << "Opt Area: " << opt_area << endl;
    cout << "Reduce: " << 100*(orig_area - opt_area)/orig_area << " %" << endl;
    cout << "-------------------------------" << endl;
    cout << "Ori Cost: " << ori_cost << endl;
    cout << "Opt Cost: " << opt_cost << endl;
    cout << "Reduce: " << 100*(ori_cost - opt_cost)/ori_cost << " %" << endl;
    cout << "===============================" << endl;
    
    return 0;
}



