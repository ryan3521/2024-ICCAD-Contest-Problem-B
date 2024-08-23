#include "banking.h"

banking::banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS){
    this->PM   = PM;
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->PFFS = PFFS;
    SUCCESS = true;
    FAIL    = true;
    cls = new cluster(INST, LIB, DIE);
}

void banking::Initial_Placement(){
    cout << "Gate placing ..." << endl;
    PM->GatePlacement();

    CopyOriginalFFs();
    cout << "Original FF placing ..." << endl;
    OriginalFFs_Placment();

}

void banking::Run_Placement_Banking(){
    InitialFFsCost();
    for(auto ff_list: ff_groups){
        for(target_size=2; target_size<=LIB->max_ff_size; target_size++){
            for(auto f: *ff_list) {
                banking_ffs.clear();
                banking_ffs.push_back(f);
            }
            SetPseudoBlock();
            ConstructXSequence();
            while(FindNewCluster()){
                FindBestCombtoPlace();
            }
        }  
    }

    return;
}

void banking::run(){
    Initial_Placement();
    Run_Placement_Banking();
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

void banking::InitialFFsCost(){
    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            f->CalculateCost(DIE->Alpha,DIE->Beta,DIE->Gamma,DIE->displacement_delay);
        }    
    }
}

void banking::SetPseudoBlock(){
    double base_expand_rate = 10;
    double expand_rate;
    for(auto f: banking_ffs){
        expand_rate = (target_size - f->d_pins.size()) > 0 ? base_expand_rate*(target_size - f->d_pins.size()) : 0;
        f->Set_PseudoBlock_Size(expand_rate);
    }
}

bool banking::cmp_se(se* a, se* b){
    if(a->coor == b->coor){
        int atype = a->type ? 1 : 0;
        int btype = b->type ? 1 : 0;
        return btype > atype;
    }
    return a->coor < b->coor;
}

void banking::ConstructXSequence(){
    se* s_ptr = NULL;
    se* e_ptr = NULL;
    xseq.clear();
    for(auto f: banking_ffs){
        s_ptr = new se(0, f->pseudo_block.xmin, f);
        e_ptr = new se(1, f->pseudo_block.xmax, f);
        xseq.push_back(s_ptr);
        xseq.push_back(e_ptr);
    }
    xseq.sort(cmp_se);
    for(auto it=xseq.begin(); it!=xseq.end(); it++){
        auto se_ptr = *it;
        if(se_ptr->type == 1){
            se_ptr->to_ff->e_it = it;
        }
    }
}

void banking::ConstructYSequence(){
    se* s_ptr = NULL;
    se* e_ptr = NULL;
    yseq.clear();
    for(auto f: tracking_list){
        s_ptr = new se(0, f->pseudo_block.ymin, f);
        e_ptr = new se(1, f->pseudo_block.ymax, f);
        yseq.push_back(s_ptr);
        yseq.push_back(e_ptr);
    }
    yseq.sort(cmp_se);
}


bool banking::FindNewCluster(){
    if(xseq.begin() == xseq.end()) return false;

    while(xseq.begin()!=xseq.end()){
        if(xseq.front()->type == 0){ // type  == "start"
            tracking_list.push_front(xseq.front()->to_ff);
            tracking_list.front()->x_track_list_it = tracking_list.begin();
            delete xseq.front();
            xseq.pop_front();
        }
        else{ // type  == "end"
            essential_ff = xseq.front()->to_ff;
            tracking_list.push_front(essential_ff);
            tracking_list.front()->x_track_list_it = tracking_list.begin();
            delete xseq.front();
            xseq.pop_front();
            break;
        }
    }

    ConstructYSequence();
    FindRelatedFF();

    return true;
}

void banking::FindRelatedFF(){
    list<ffi*> y_tracking_list;

    while(yseq.begin() != yseq.end()){
        if(yseq.front()->type == 0){
            y_tracking_list.push_front(yseq.front()->to_ff);
            y_tracking_list.front()->y_track_list_it = y_tracking_list.begin();
            delete yseq.front();
            yseq.pop_front();
        }
        else if(yseq.front()->to_ff == essential_ff){
            y_tracking_list.erase(yseq.front()->to_ff->y_track_list_it);
            cls->essential_ff = essential_ff;
            cls->related_ffs.clear();
            for(auto f: y_tracking_list) cls->related_ffs.push_back(f);
            delete yseq.front();
            yseq.pop_front();
            break;
        }
        else{
            y_tracking_list.erase(yseq.front()->to_ff->y_track_list_it);
            delete yseq.front();
            yseq.pop_front();
        }
    }

    while(yseq.begin() != yseq.end()){
        delete yseq.front();
        yseq.pop_front();
    }
    return;
}
            

void banking::FindBestCombtoPlace(){
    bool   SET_CONSTRAIN = true;
    double DISPLACE_CONSTRAIN = 50;

    cls->ConstructCombs(target_size);
    while(cls->comb_list.begin() != cls->comb_list.end()){
        target_comb = cls->comb_list.front();
        cls->comb_list.pop_front();
        if(target_comb->TestQuality(0) == SUCCESS){
            for(auto f: target_comb->members) {PM->DeleteFlipFlop(f);}

            ffi* new_ff = target_comb->GetNewFF();
            if(PM->placeFlipFlop(new_ff, SET_CONSTRAIN, DISPLACE_CONSTRAIN) == SUCCESS){
                new_ff->to_list = target_comb->members.front()->to_list;
                new_ff->to_list->push_front(new_ff);
                new_ff->it_pointer = new_ff->to_list->begin();
                for(auto f: target_comb->members){
                    if(f != essential_ff) {
                        delete *(f->e_it);
                        xseq.erase(f->e_it);
                    }
                    tracking_list.erase(f->x_track_list_it);
                    new_ff->to_list->erase(f->it_pointer);
                    delete f;
                }
                break;
            }
            else{
                for(auto f: target_comb->members) {
                    PM->PlaceBackFlipFlop(f);
                }
                delete new_ff;
                delete target_comb;
                continue;
            }
        }
        else{
            delete target_comb;
            continue;
        }
    }
    return;
}


