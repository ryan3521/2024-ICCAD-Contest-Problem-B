#include "func.h"



void FineTune(lib& LIB, list<ffi*>& NCLS, list<ffi*>& MBFFS, list<ffi*>& UPFFS, dieInfo& DIE){
    bool PRINT_INFO = false;
    if(PRINT_INFO) cout << endl << "Start fine tune >>>" << endl;
    int cnt;
    string inst_name;
    ffi* new_fi;
    ffcell* new_type;

    cnt = MBFFS.size();
    for(auto& fi: NCLS){
        inst_name = "";
        inst_name = inst_name + "MA" + to_string(cnt);
        new_fi = new ffi(inst_name, 0, 0);
        new_type = LIB.fftable_cost[fi->type->bit_num].front();

        // cout <<new_fi->type->name <<endl;
        new_fi->type = new_type;
        new_fi->d_pins.reserve(fi->d_pins.size());
        new_fi->q_pins.reserve(fi->q_pins.size());

        for(auto& p: fi->d_pins){
            p->to_new_ff = new_fi;
            new_fi->d_pins.push_back(p);
            
        }      
        for(auto& p: fi->q_pins){
            p->to_new_ff = new_fi;
            new_fi->q_pins.push_back(p);
        }      
        new_fi->new_coor();
        new_fi->clk_pin = new pin;

        // for(auto& p: fi->d_pins){
        //     p->new_name = p->name;
        //     p->to_new_ff = p->to_ff;
        //     p->new_coox = p->coox;
        //     p->new_cooy = p->cooy;
        // }
        // for(auto& p: fi->q_pins){
        //     p->new_name = p->name;
        //     p->to_new_ff = p->to_ff;
        //     p->new_coox = p->coox;
        //     p->new_cooy = p->cooy;
        // }
        UPFFS.push_back(new_fi);
        cnt++;
    }
    for(auto& fi: MBFFS){
        UPFFS.push_back(fi);
    }
    /////////////////////////////////

    return;
}