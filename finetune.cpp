#include "func.h"

void FineTune(lib& LIB, list<ffi*>& NCLS, list<ffi*>& MBFFS, list<ffi*>& UPFFS, dieInfo& DIE){
    for(auto& fi: NCLS){
        for(auto& p: fi->d_pins){
            p->to_new_ff = p->to_ff;
            p->new_coox = p->coox;
            p->new_cooy = p->cooy;
        }
        for(auto& p: fi->q_pins){
            p->to_new_ff = p->to_ff;
            p->new_coox = p->coox;
            p->new_cooy = p->cooy;
        }
        UPFFS.push_back(fi);
    }
    for(auto& fi: MBFFS){
        UPFFS.push_back(fi);
    }
    /////////////////////////////////

    return;
}