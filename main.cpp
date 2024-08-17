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
    costeva COST(&DIE, &LIB, &INST);
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


    // Test TNS: begin
    // double ori_total = 0;
    // for(auto it: INST.ff_umap){
    //     ffi* f = it.second;
    //     list<pin*> dpins;
    //     list<pin*> qpins;
    //     list<pin*> optseq_D;
    //     list<pin*> optseq_Q;
    //     ffcell* type = f->type;
    //     double coeff = DIE.displacement_delay;
    //     double test_result;
    //     double ori_slack = 0;
    //     double ori_dslack = f->d_pins.front()->dspd_slk;
    //     double ori_qslack = 0;

        // for(auto p: f->q_pins.front()->to_net->opins){
        //     double temp_slack;
        //     if(p->pin_type == 'f'){
        //         temp_slack = f->q_pins.front()->dspd_slk;
        //     }
        //     else if(p->pin_type == 'g'){
        //         temp_slack = p->to_gate->get_critical_slack();
        //     }
        //     else if(p->pin_type == 'd'){
        //         temp_slack = 0; 
        //     }
        //     if(temp_slack < 0) ori_qslack = ori_qslack + temp_slack;
        // }

        // if(ori_qslack<0 && ori_dslack<0) ori_slack = ori_dslack + ori_qslack;
        // else if(ori_dslack < 0) ori_slack = ori_dslack;
        // else if(ori_qslack < 0) ori_slack = ori_qslack;
        // else ori_slack = ori_dslack + ori_qslack;

        

        // if(ori_dslack < 0) ori_total = ori_total + ori_dslack;


        // dpins.push_back(f->d_pins.front());
        // qpins.push_back(f->q_pins.front());
        // test_result = INST.TnsTest(0, dpins, qpins, type, coeff, optseq_D, optseq_Q);

        // if((test_result - ori_slack) > 0.000001){
        //     cout << endl;
        //     cout << "Ori = " << ori_slack << ", Test result: " << test_result << endl; 
        //     cout << "****************************************" << endl;
        //     cout << "Original Slack: " << endl;
        //     cout << "Slack: " << f->d_pins.front()->slack << endl;
        //     cout << "DSPD d pin: " << f->d_pins.front()->dspd_slk << endl;
        //     cout << "DSPD q pin: ";
        //     for(auto p: f->q_pins.front()->to_net->opins){
        //         if(p->pin_type == 'f'){
        //             cout << "f:" << f->q_pins.front()->dspd_slk << " -> ";
        //         }
        //         else if(p->pin_type == 'g'){
        //             cout << "g:" << p->to_gate->get_critical_slack() << " -> ";
        //         }
        //         else if(p->pin_type == 'd'){
        //             cout << "d:0 -> "; 
        //         }
        //     }
        //     cout <<  "end" << endl;
        //     cout << "****************************************" << endl;
        //     cout << endl;
        //     return 0;
        // }
    // }
    // cout << "Origianl totoal slack: " << ori_total << endl;

    // Test TNS: end








    for(auto it: INST.ff_umap){
        auto f = it.second;
        orig_area = orig_area + f->type->area;
        orig_power = orig_power + f->type->gate_power;
        ori_bitnum = ori_bitnum + f->d_pins.size();
    }

    FFBANK.run();
    FFBANK.PrintResult();

    // for(auto f: UPFFS){
    //     opt_area = opt_area + f->type->area;
    //     opt_power = opt_power + f->type->gate_power;
    //     aft_bitnum = aft_bitnum + f->d_pins.size();
    // }

    // for(auto f: UPFFS){
    //     list<pin*> dpins;
    //     list<pin*> qpins;
    //     list<pin*> optseq_D;
    //     list<pin*> optseq_Q;
    //     ffcell* type;
    //     double coeff = DIE.displacement_delay;

    //     if(/*f->type->bit_num == 1 && f->d_pins.front()->to_ff->type != f->type &&*/ f->name == "NEWFF18020"){
    //         dpins.push_back(f->d_pins.front());
    //         qpins.push_back(f->q_pins.front());

    //         cout << "---------------------------" << endl;
    //         cout << f->name << endl;
    //         cout << "original type: " << endl;
    //         type = f->d_pins.front()->to_ff->type;
    //         INST.TnsTest(1, dpins, qpins, type, coeff, optseq_D, optseq_Q);
    //         cout << "new type: " << endl;
    //         type = f->type;
    //         INST.TnsTest(1, dpins, qpins, type, coeff, optseq_D, optseq_Q);

    //         cout << "---------------------------" << endl;
    //     }
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

    // DrawFFs(DIE, LIB, INST, UPFFS);
    return 0;
}



