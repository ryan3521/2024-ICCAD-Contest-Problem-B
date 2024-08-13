#include "banking.h"

bool cluster::UpdateCoor_CheckMove(){
    double sum_x = 0;
    double sum_y = 0;

    for(auto m: members){
        sum_x = sum_x + m->coox; 
        sum_y = sum_y + m->cooy; 
    }

    coox = sum_x/(double)size;
    cooy = sum_y/(double)size;

    return (coox!=old_coox || cooy!=old_cooy) ? true : false;
}

void cluster::AddMember(cluster* new_member){
    size++;
    members.push_back(new_member);
}

void cluster::Clear(){
    size = 0;
    members.clear();
    old_coox = coox;
    old_cooy = cooy;
}


banking::banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->UPFFS = UPFFS;
}


void banking::run(){
    // Steps:
    // 1. initial: for each single bit ff, choose the ff-type such that minimum the cost.
    // 2. for each size of mbff, fist run k-means to form new clusters,
    //    for each new clusters, choose the lowest cost ff type from LIB,
    //    compare cost per bit with the original one which the bit belongs to, if the new mbff cost per bit is lower than the original one, 
    //    "and" also the original cluster member bit have over half of the member bits find new lower cost cluster, than destroy the orignal cluster,
    //    and retain the new cluster, otherwise, destroy the new cluster.

    for(auto ff_list: INST->ffs_sing){
        // Initial: begin
        banking_ffs.clear();
        for(auto f: *ff_list) { banking_ffs.push_back(f); }
        Initial_SingleBit_Cls();
        single_bit_clusters.sort(sort_single_bit_cls);
        list<cluster*> kmeans_data_points;
        list<cluster*> top_list;
        for(auto& cls: single_bit_clusters) {
            kmeans_data_points.push_back(cls);
            top_list.push_back(cls);
        }
        // Initial: end

        for(int cluster_target_size=LIB->max_ff_size; cluster_target_size>1; cluster_target_size--){
            // K-means initial: begin
            list<cluster*> k_clusters;
            int cnt = 0;
            cluster* new_cls;
            for(auto& cls: single_bit_clusters){
                if(cnt == 0){
                    new_cls = new cluster;
                    new_cls->coox = cls->coox;
                    new_cls->cooy = cls->cooy;
                    k_clusters.push_back(new_cls);
                }
                if(cnt == cluster_target_size-1){ cnt=0; }
                else                            { cnt++; }
            }
            // K-means initial: end

            // K-means cluster: begin
            while(1){
                for(auto& data: kmeans_data_points){
                    double   best_cost = numeric_limits<double>::max();
                    cluster* best_cls = NULL;
                    // Find closest cluster: begin
                    for(auto& k: k_clusters){
                        if(k->size == cluster_target_size) continue;
                        double cost;
                        cost = abs(k->coox - data->coox) + abs(k->cooy - data->cooy); // HPWL
                        //cost = (k->coox - data->coox)*(k->coox - data->coox) + (k->cooy - data->cooy)*(k->cooy - data->cooy); // Euclidean Distance 
                        if(cost < best_cost){
                            best_cost = cost;
                            best_cls = k;
                        }
                    }
                    best_cls->AddMember(data);
                    // Find closest cluster: end
                }

                bool no_move = true;
                // Update Coor: begin
                for(auto& k: k_clusters){
                    if(k->UpdateCoor_CheckMove() == true){
                        no_move = false;
                    }
                }
                // Update Coor: end

                // Check break: begin
                if(no_move == true){
                    break;
                }
                else{
                    for(auto k: k_clusters){
                        k->Clear();
                    }
                }
                // Check break: end
            }
            // K-means cluster: end

            // Calculate cost per bit for new cluster: begin
            list<cluster*>::iterator itr = k_clusters.begin();
            while(itr != k_clusters.end()){
                cluster* new_cls = *itr;
                ffcell* mincost_ftype = NULL;
                double  mincost = numeric_limits<double>::max(); // = cost per bit (included: tns, area, power)

                if(new_cls->size == 1){
                    itr = k_clusters.erase(itr);
                    delete new_cls;
                    continue;
                }

                for(auto& m: new_cls->members) m->to_new_cluster = new_cls;

                list<pin*> dpins;
                list<pin*> qpins;
                for(auto& m: new_cls->members){
                    dpins.push_back(m->single_bit_ff->d_pins.front());
                    qpins.push_back(m->single_bit_ff->q_pins.front());
                }

                // initial valid size: begin
                int valid_bit_size = new_cls->size;
                while(LIB->fftable_cost[valid_bit_size].size() == 0){ valid_bit_size++; }
                // initial valid size: end

                // find best ff type: begin
                for(auto ftype: LIB->fftable_cost[valid_bit_size]){
                    double cost = 0;
                    double slack, ns;
                    slack = INST->TnsTest(dpins, qpins, ftype, DIE->displacement_delay);
                    ns = (slack > 0) ? 0 : abs(slack);
                    cost = (DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area)/(double)new_cls->size;
                
                    if(cost < mincost){
                        mincost = cost;
                        mincost_ftype = ftype;
                    }
                }
                new_cls->type = mincost_ftype;
                new_cls->cost_per_bit = mincost;
                // find best ff type: end

                itr++;
            }
            // Calculate cost per bit for new cluster: end

            // Determine new or old cluster stay: begin
            
            // Determine new or old cluster stay: end
        }
    }



    return;
}

void banking::Initial_SingleBit_Cls(){
    for(auto f: banking_ffs){
        cluster* cls = new cluster;
        cls->size = 1;
        cls->is_top = true;
        cls->to_top_cluster = cls;
        cls->to_new_cluster = NULL;
        cls->old_coox = f->cen_x;
        cls->old_cooy = f->cen_y;
        cls->coox = f->cen_x;
        cls->cooy = f->cen_y;
        single_bit_clusters.push_back(cls);

        list<pin*> dpins;
        list<pin*> qpins;
        dpins.push_back(f->d_pins[0]);
        qpins.push_back(f->q_pins[0]);

        ffcell* mincost_ftype = NULL;
        double  mincost = numeric_limits<double>::max(); // = cost per bit (included: tns, area, power)

        for(auto ftype: LIB->fftable_cost[1]){
            double cost = 0;
            double slack, ns;
            slack = INST->TnsTest(dpins, qpins, ftype, DIE->displacement_delay);
            ns = (slack > 0) ? 0 : abs(slack);
            cost = DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area;
        
            if(cost < mincost){
                mincost = cost;
                mincost_ftype = ftype;
            }
        }

        cls->type = mincost_ftype;
        cls->single_bit_ff = f;
    }
    return;
}

bool banking::sort_single_bit_cls(cluster* a, cluster* b){
    return a->single_bit_ff->cen_x < b->single_bit_ff->cen_x;
}