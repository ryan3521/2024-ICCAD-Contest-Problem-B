#include "func.h"

void FineTune(lib& LIB, list<ffi*>& NCLS, list<ffi*>& MBFFS, list<ffi*>& UPFFS, dieInfo& DIE){
    int cnt;
    string inst_name;
    cnt = MBFFS.size();
    for(auto& fi: NCLS){
        // inst_name = "";
        // inst_name = inst_name + "MA" + to_string(cnt);
        for(auto& p: fi->d_pins){
            p->new_name = p->name;
            p->to_new_ff = p->to_ff;
            p->new_coox = p->coox;
            p->new_cooy = p->cooy;
        }
        for(auto& p: fi->q_pins){
            p->new_name = p->name;
            p->to_new_ff = p->to_ff;
            p->new_coox = p->coox;
            p->new_cooy = p->cooy;
        }
        UPFFS.push_back(fi);
        cnt++;
    }
    for(auto& fi: MBFFS){
        UPFFS.push_back(fi);
    }
    /////////////////////////////////

    return;
}