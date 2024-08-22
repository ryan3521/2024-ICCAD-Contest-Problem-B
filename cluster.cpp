#include "cluster.h"

cluster::cluster(){
    size = 0;
    is_top = false;
    to_new_cluster = NULL;
    to_top_cluster = NULL;
    cost_per_bit = 0;
    members.clear();
    type = NULL;
    single_bit_ff = NULL;
    is_better_than_new = false;
    mark_remove = false;
    mark_recalculate = false;
    optseq_D.clear();
    optseq_Q.clear();
}

bool cluster::cmp(cluster* a, cluster* b){
    return a->displace_distance > b->displace_distance;
}

void cluster::SortMembers(){
    members.sort(cmp);
}

bool cluster::UpdateCoor_CheckMove(){
    double sum_x = 0;
    double sum_y = 0;

    for(auto m: members){
        sum_x = sum_x + m->coox; 
        sum_y = sum_y + m->cooy; 
    }

    coox = sum_x/(double)size;
    cooy = sum_y/(double)size;

    return (abs(coox-old_coox)>0 || abs(cooy-old_cooy)>0) ? true : false;
}

void cluster::AddMember(cluster* new_member){
    members.push_back(new_member);
    size++;
}

void cluster::Clear(){
    size = 0;
    members.clear();
    old_coox = coox;
    old_cooy = cooy;
}

void cluster::Calculate_BestCost_FFtype(bool print, lib* LIB, inst* INST, dieInfo* DIE){
    ffcell* mincost_ftype = NULL;
    double  mincost = numeric_limits<double>::max(); // = cost per bit (included: tns, area, power)
    list<pin*> dpins;
    list<pin*> qpins;

    for(auto& m: members){
        dpins.push_back(m->single_bit_ff->d_pins.front());
        qpins.push_back(m->single_bit_ff->q_pins.front());
    }

    // initial valid size: begin
    int valid_bit_size = size;
    while(LIB->fftable_cost[valid_bit_size].size() == 0){ valid_bit_size++; }
    // initial valid size: end

    // find best ff type: begin
    for(auto ftype: LIB->fftable_cost[valid_bit_size]){
        double cost = 0;
        double slack, ns;

        slack = INST->TnsTest(print, dpins, qpins, ftype, DIE->displacement_delay, optseq_D, optseq_Q);
        ns = (slack > 0) ? 0 : abs(slack);
        cost = (DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area)/(double)size;
    
        if(cost < mincost){
            mincost = cost;
            mincost_ftype = ftype;
        }
    }
    type = mincost_ftype;
    cost_per_bit = mincost;
    // find best ff type: end

    // if(size == 1) cout << "size 1: " << cost_per_bit << endl;
    // if(size == 4) cout << "size 4: " << cost_per_bit << endl;
    return;
}

bool cluster::TestQuality(bool print){
    if(print) cout << "testing quality: size = " << size << endl;
    if(to_top_cluster == this){ // this top cluster's member size == 1
        if(to_new_cluster == NULL) return true;
        if(cost_per_bit > to_new_cluster->cost_per_bit){
            is_better_than_new = false;
            return false;
        }
        else{
            is_better_than_new = true;
            // tell the new cluster that some(one) of your members are not going to join your new group,
            // the new cluster should re-calculate ff-type and cost-per-bit!
            to_new_cluster->mark_recalculate = true;
            return true;
        }
    }
    else{ // this top cluster's member size > 1
        double old_cost = cost_per_bit*members.size();
        double new_cost = 0;
        for(auto& m: members){
            if(m->to_new_cluster==NULL){
                new_cost = new_cost + m->cost_per_bit;
            }
            else{
                new_cost = new_cost + m->to_new_cluster->cost_per_bit;
            }
        }
        if(old_cost > new_cost){
            is_better_than_new = false;
            if(print) cout << "testing quality finish: false" << endl;
            return false;
            
        }
        else{
            is_better_than_new = true;
            for(auto& m: members){
                if(m->to_new_cluster!=NULL){
                    // tell the new cluster that some(one) of your members are not going to join your new group,
                    // the new cluster should re-calculate ff-type and cost-per-bit!
                    m->to_new_cluster->mark_recalculate = true;
                }
            }
            if(print) cout << "testing quality finish: true" << endl;
            return true;
        }
    }   
}