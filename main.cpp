#include <bits/stdc++.h>
#include <ctime>

#include "func.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "placement.h"
#include "cluster.h"
#include "modifycls.h"
#include "costeva.h"
// #include "INTEGRA.h"

using namespace std;

int main(int argc, char** argv){

    double start = clock();
    lib LIB;
    inst INST;
    dieInfo DIE;
    netlist NL;
    placement PM(&LIB, &INST, &DIE);
    costeva COST(&DIE, &LIB, &INST);
    // INTEGRA itgra(&INST, &NL);
    list<cluster*> KCR; // Kmeans Cluster Result
    list<ffi*> NCLS; // Store the non cluster ffs after doing K-means Cluster
    list<ffi*> MBFFS; // Store the new MBFFs after doing K-means Cluster
    list<ffi*> UPFFS; // Store all ffs which need to be placed (== NCLS + MBFFS)
    list<ffi*> PFFS; // Store all ffs which are placed
    banking FFBANK(&INST, &LIB, &DIE, &UPFFS);

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
    INST.SlackDispense(DIE);
    INST.DebankAllFF(LIB);
    INST.ConstructFSR(DIE);

    for(auto it: INST.ff_umap){
        auto f = it.second;
        orig_area = orig_area + f->type->area;
        orig_power = orig_power + f->type->gate_power;
        ori_bitnum = ori_bitnum + f->d_pins.size();
    }

    // KmeansCls(DIE, LIB, INST, KCR, NCLS);  
    // MapClstoMBFF(LIB, KCR, MBFFS);
    // FineTune(LIB, NCLS, MBFFS, UPFFS, DIE); // Not finish yet
    // int bit_cnt = 0;
    // int sb_cnt = 0;
    // int arr[5] = {0, 0, 0, 0, 0};
    // for(auto& f: UPFFS){
    //     bit_cnt = bit_cnt + f->d_pins.size();
    //     if(f->d_pins.size() == 1) sb_cnt ++;
    //     arr[f->d_pins.size()]++;
    // }
    // cout << "Total bit num: " << bit_cnt << endl;
    // cout << "Cluster num : " << bit_cnt - sb_cnt << endl;
    // cout << "Non cluster num: " << sb_cnt << endl;

    // for(int i=1; i<5; i++) cout << "Type " << i << ": " << arr[i] << endl;
    
    // for(auto it: INST.ff_umap){
    //     auto f = it.second;
    //     UPFFS.push_back(f);
    //     f->cen_x = (f->fsr.cen_x - f->fsr.
    //     for(int i=0; i<f->d_pins.size(); i++){
    //         f->d_pins[i]->to_new_ff = f;
    //         f->d_pins[i]->new_coox = f->d_pins[i]->coox;
    //         f->d_pins[i]->new_cooy = f->d_pins[i]->cooy;
    //         f->q_pins[i]->to_new_ff = f;
    //         f->q_pins[i]->new_coox = f->q_pins[i]->coox;
    //         f->q_pins[i]->new_cooy = f->q_pins[i]->cooy;
    //     }
    // }
    FFBANK.run();
    COST.evaluate(&UPFFS);

    for(int i=1; i<5; i++){
        cout << "Bit " << i << ": ";
        if(LIB.opt_fftable[i].size() > 0) {
            cout << "Width  " << LIB.opt_fftable[i].front()->size_x << ", ";
            cout << "Height " << LIB.opt_fftable[i].front()->size_y << ", ";
        }
        cout << endl;
    }

    // for(auto f: UPFFS){
    //     opt_area = opt_area + f->type->area;
    //     opt_power = opt_power + f->type->gate_power;
    //     aft_bitnum = aft_bitnum + f->d_pins.size();
    // }


    PM.placeGateInst();
    PM.placeFlipFlopInst( UPFFS, PFFS);
    Output(argv[2], PFFS, INST);

    for(auto f: PFFS){
        opt_area = opt_area + f->type->area;
        opt_power = opt_power + f->type->gate_power;
        aft_bitnum = aft_bitnum + f->d_pins.size();
    }


    double end = clock();

    if(ori_bitnum == aft_bitnum)
        cout << endl << "success - bit num match -" << endl;
    else 
        cout << endl << "error - bit num not match" << endl;

    ori_cost = DIE.Beta*orig_power + DIE.Gamma*orig_area;
    opt_cost = DIE.Beta*opt_power + DIE.Gamma*opt_area;


    cout << endl;
    cout << "Optimize Report >>> " << endl;
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
    
    cout << "Total cost: " << COST.evaluate(&PFFS) << endl;

    cout << endl << "Total execution time: " << (end - start) / 1000000.0  << " s" << '\n';


    return 0;
}



