#include "cluster.h"

comb::comb(){
    size = 0;
    cost_per_bit = numeric_limits<double>::max();
    members.clear();
    optseq_D.clear();
    optseq_Q.clear();
}

void comb::AddMember(ffi* f){
    members.push_back(f);
    size = size + f->d_pins.size();
}

cluster::cluster(inst* INST, lib* LIB, dieInfo* DIE){
    this->INST = INST;
    this->LIB = LIB;
    this->DIE = DIE;
    
    size = 0;
    members.clear();
    type = NULL;
}

void cluster::AddMember(ffi* new_member){
    related_ffs.push_back(new_member);
    size = size + new_member->d_pins.size();
}

void cluster::Clear(){
    size = 0;
    essential_ff = NULL;
    type = NULL;
    related_ffs.clear();
    members.clear();
    comb_list.clear();
}

bool cluster::cmp_dist(ffi* a, ffi* b){
    return a->dist_to_essential < b->dist_to_essential;
}

bool cluster::cmp_cost(comb* a, comb* b){
    return a->cost_per_bit < b->cost_per_bit;
}

void cluster::ConstructCombs(int target_size){

    comb_list.clear();

    for(auto it = related_ffs.begin(); it!=related_ffs.end(); it=it){
        ffi* f = *it;

        if(essential_ff->d_pins.size() + f->d_pins.size() <= target_size){
            f->dist_to_essential = (f->cen_x - essential_ff->cen_x)*(f->cen_x - essential_ff->cen_x) + (f->cen_y - essential_ff->cen_y)*(f->cen_y - essential_ff->cen_y);
            it++;
        }
        else{
            it = related_ffs.erase(it);
        }
    }

    if(related_ffs.empty()) return;

    related_ffs.sort(cmp_dist);

    comb* new_comb = new comb();
    comb_list.push_back(new_comb);
    new_comb->AddMember(essential_ff);
    while(related_ffs.begin() != related_ffs.end()){
        if(new_comb->size + related_ffs.front()->d_pins.size() <= target_size){
            new_comb->AddMember(related_ffs.front());
            related_ffs.pop_front();
        }
        else{
            new_comb = new comb();
            comb_list.push_back(new_comb);
            new_comb->AddMember(essential_ff);
            new_comb->AddMember(related_ffs.front());
            related_ffs.pop_front();
        }
    }

    for(auto cmb: comb_list) cmb->Calculate_BestCost_FFtype(0, LIB, INST, DIE);
    comb_list.sort(cmp_cost);

}

void comb::Calculate_BestCost_FFtype(bool print, lib* LIB, inst* INST, dieInfo* DIE){
    ffcell* mincost_ftype = NULL;
    double  mincost = numeric_limits<double>::max(); // = cost per bit (included: tns, area, power)
    list<pin*> dpins;
    list<pin*> qpins;

    for(auto& m: members){
        for(auto p: m->d_pins) dpins.push_back(p);
        for(auto p: m->q_pins) qpins.push_back(p);
    }

    // initial valid size: begin
    int valid_bit_size = size;
    while(LIB->fftable_cost[valid_bit_size].size() == 0){ valid_bit_size++; }
    // initial valid size: end

    // find best ff type: begin
    for(auto ftype: LIB->fftable_cost[valid_bit_size]){
        double cost = 0;
        double slack, ns;
        list<pin*> dpins_result;
        list<pin*> qpins_result;

        slack = INST->TnsTest(print, dpins, qpins, ftype, DIE->displacement_delay, dpins_result, qpins_result);
        ns = (slack > 0) ? 0 : abs(slack);
        cost = (DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area)/(double)size;


        if(cost < mincost){
            mincost = cost;
            mincost_ftype = ftype;
            optseq_D.clear();
            optseq_Q.clear();
            for(auto p: dpins_result) optseq_D.push_back(p);
            for(auto p: qpins_result) optseq_Q.push_back(p);
        }
    }
    type = mincost_ftype;
    cost_per_bit = mincost;
    
    // find best ff type: end
    return;
}

bool comb::TestQuality(bool print){
    double newcost = cost_per_bit*size;
    double oldcost = 0;

    for(auto f: members) oldcost = oldcost + f->cost;
    
    if(newcost < oldcost) return true;
    else                  return false;
}

ffi* comb::GetNewFF(){
    ffi* nf = new ffi("temp_name", 0, 0);

    nf->type = type;
    nf->d_pins.reserve(size);
    nf->q_pins.reserve(size);
    for(auto p: optseq_D) nf->d_pins.push_back(p);
    for(auto p: optseq_Q) nf->q_pins.push_back(p);
    nf->clk_pin = new pin;
    nf->update_coor();
    nf->cost = cost_per_bit*size;

    return nf;
}