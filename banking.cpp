#include "banking.h"

cls::cls(){

}






banking::banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->UPFFS = UPFFS;
}

bool cmp_ff_x(ffi* a, ffi* b){
    return a->fsr.cen_x < b->fsr.cen_x;
}



bool cmp_priority(pair<int, double> a, pair<int, double> b){
    if(a.second == b.second){
        return a.first < b.first;
    }
    return a.second < b.second;
}

bool banking::cmp_se(se* a, se* b){
    if(a->coor == b->coor){
        return a->type > b->type;
    }
    return a->coor < b->coor;
}

void banking::initial_size_priority(){
    for(int i=1; i<LIB->mbff_cost.size(); i++){
        size_priority.push_back(pair<int, double>(i, LIB->mbff_cost[i]/(double)i));
    }
    size_priority.sort(cmp_priority);

    int front_bit = -1; 

    auto it = size_priority.begin();
    while(it!=size_priority.end()){

        if(front_bit == -1){
            front_bit = it->first;
        }
        else if(it->first > front_bit){
            it = size_priority.erase(it);
            continue;
        }
        else{
            front_bit = it->first;
        }
        it++;
    }
}

void banking::integra(int target_size){
    // construct x sequence
    list<se*> x_sequence;
    for(auto f: banking_ffs){
        if(f->fsr.can_move == false){
            ncls_ffs.push_back(f);
            continue;
        }

        ff2se* ff2se_ptr = new ff2se{f};
        se* se_ptr = NULL;
        
        se_ptr = new se{0, f->fsr.xmin, ff2se_ptr};
        x_sequence.push_back(se_ptr);
        
        se_ptr = new se{1, f->fsr.xmax, ff2se_ptr};
        x_sequence.push_back(se_ptr);
    }
    x_sequence.sort(cmp_se);
    
    for(list<se*>::iterator it=x_sequence.begin(); it!=x_sequence.end(); it++){
        if((*it)->type == 0){
            (*it)->to_ff->s_it = it;
        }
        else{
            (*it)->to_ff->e_it = it;
        }
    }

    
}

void banking::run(){
    // Steps:
    // 0. initial
    // 1. integra (bank ffs without violating timing slack) 
    // 2. kmeans  (bank ffs with violating timing slack)
    // 3. sort clusters by negative slack contribution
    // 4. destroy clusters which increase the cost

    initial_size_priority();
    for(auto& ff_list: INST->ffs_sing){
        banking_ffs.clear();
        ncls_ffs.clear();
        clusters.clear();
        for(auto f: *ff_list){
            banking_ffs.push_back(f);
        }

    }
}