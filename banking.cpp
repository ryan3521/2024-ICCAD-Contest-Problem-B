#include "banking.h"

banking::banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS){
    this->PM   = PM;
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->PFFS = PFFS;
    SUCCESS = true;
    FAIL    = true;
}

void banking::Initial_Placement(){
    cout << "Gate placing ..." << endl;
    PM->GatePlacement();

    CopyOriginalFFs();
    cout << "FF placing ..." << endl;
    OriginalFFs_Placment();

}

void banking::Run_Placement_Banking(){
    



    return;
}

void banking::run(){
    Initial_Placement();

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

bool banking::cmp_ff_x(ffi* a, ffi* b){
    return a->coox < b->coox;
}

void banking::OriginalFFs_Placment(){
    bool SET_CONSTRAIN = true;
    bool DONT_SET_CONSTRAIN = false;
    double DISPLACE_CONSTRAIN = 20;

    list<ffi*> buffer_list;
    list<ffi*> multi_bit_stack;

    for(auto flist: ff_groups){
        for(auto it=flist->begin(); it!=flist->end(); it++){
            ffi* f = *it;
            placing_ffs.push_back(f);
        }
    }

    placing_ffs.sort(cmp_ff_x);

    for(auto f: placing_ffs){
        if(PM->placeFlipFlop(f, SET_CONSTRAIN, DISPLACE_CONSTRAIN) == SUCCESS){
            if(f->type->bit_num > 1){
                multi_bit_stack.push_back(f);
            }
        }
        else{
            buffer_list.push_back(f);
        }
    }

    cout << "BufferList: " << buffer_list.size() << endl;
}

            




