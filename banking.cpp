#include "banking.h"

banking::banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS){
    this->PM   = PM;
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->PFFS = PFFS;
}

void banking::Initial_Placement(){
    PM->GatePlacement();
    CopyOriginalFFs();

}

void banking::Run_Placement_Banking(){
    



    return;
}

void banking::run(){


    return;
}

void banking::CopyOriginalFFs(){
    string inst_name;
    int newff_cnt = 0;
    list<ffi*>* ff_list;
    for(auto ori_ffs: INST->ffs_ori){
        ff_list = new list<ffi*>;

        for(auto f: *ori_ffs){
            inst_name = "";
            inst_name = inst_name + "NEWFF" + to_string(newff_cnt);
            ffi* nf = new ffi(inst_name, f->coox, f->cooy);

            nf->type = f->type;
            nf->d_pins.reserve(f->d_pins.size());
            nf->q_pins.reserve(f->q_pins.size());
            for(auto p: f->d_pins) nf->d_pins.push_back(p);
            for(auto p: f->q_pins) nf->q_pins.push_back(p);
            nf->clk_pin = new pin;
            nf->update_coor();

            ff_list->push_back(nf);
            nf->to_list = ff_list;
            newff_cnt++;
        }
        ff_groups.push_back(ff_list);
    }

    for(auto flist: ff_groups){
        for(auto it=flist->begin(); it!=flist->end(); it++){
            ffi* f = *it;
            f->it_pointer = it;
        }
    }
}

            




