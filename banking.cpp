#include "banking.h"

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

bool cluster::UpdateCoor_CheckMove(){
    double sum_x = 0;
    double sum_y = 0;

    for(auto m: members){
        sum_x = sum_x + m->coox; 
        sum_y = sum_y + m->cooy; 
    }

    coox = sum_x/(double)size;
    cooy = sum_y/(double)size;

    return (abs(coox-old_coox)/old_coox>0.9 || abs(cooy-old_cooy)/old_cooy>0.9) ? true : false;
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

    int clk_group_cnt = 0;
    int newff_cnt = 0;
    for(auto ff_list: INST->ffs_sing){
        // cout << endl << "Group " << clk_group_cnt << " FFs Processing ... (" << ff_list->size() << ")" << endl;
        // Initial: begin
        banking_ffs.clear();
        for(auto f: *ff_list) { banking_ffs.push_back(f); }
        Initial_SingleBit_Cls();
        single_bit_clusters.sort(sort_single_bit_cls);
        list<cluster*> kmeans_data_points;
        list<cluster*> top_list;
        kmeans_data_points.clear();
        top_list.clear();
        for(auto& cls: single_bit_clusters) {
            kmeans_data_points.push_back(cls);
            top_list.push_back(cls);
        }
        // Initial: end

        for(int cluster_target_size=LIB->max_ff_size; cluster_target_size>1; cluster_target_size--){
            // cout << endl << "K-means clustering, target size = " << cluster_target_size << endl;
            
            // K-means initial: begin
            // if(cluster_target_size == 2) cout << "K-means initial" << endl;
            list<cluster*> k_clusters;
            k_clusters.clear();
            int cnt = 0;
            cluster* new_cls;
            for(auto& cls: single_bit_clusters){
                if(cnt == 0){
                    new_cls = new cluster;
                    new_cls->size = 0;
                    new_cls->coox = cls->coox;
                    new_cls->cooy = cls->cooy;
                    k_clusters.push_back(new_cls);
                }
                if(cnt == cluster_target_size-1){ cnt=0; }
                else                            { cnt++; }
            }
            // K-means initial: end

            // K-means cluster: begin
            // cout << "do k-means cluster:" << endl;
            // cout << "K = " << k_clusters.size() << endl;
            // cout << "data = " << kmeans_data_points.size() << endl;
            int ITR_LIMIT = 20;
            int itr_cnt = 0;
            while(1){
                // if(cluster_target_size == 2 && clk_group_cnt==5) cout << "itr = " << itr_cnt << endl;
                for(auto& data: kmeans_data_points){
                    double   best_cost = numeric_limits<double>::max();
                    cluster* best_cls = NULL;
                    // Find closest cluster: begin
                    bool full = true;
                    for(auto& k: k_clusters){
                        if(k->size == cluster_target_size) continue;
                        double cost;
                        cost = abs(k->coox - data->coox) + abs(k->cooy - data->cooy); // HPWL
                        //cost = (k->coox - data->coox)*(k->coox - data->coox) + (k->cooy - data->cooy)*(k->cooy - data->cooy); // Euclidean Distance 
                        if(cost < best_cost){
                            best_cost = cost;
                            best_cls = k;
                        }
                        full = false;
                    }
                    if(best_cls == NULL) cout << "error: full = " << full << endl;
                    best_cls->AddMember(data);
                    // Find closest cluster: end
                }

                bool no_move = true;
                // Update Coor: begin
                // if(cluster_target_size == 2) cout << "updatine coor" << endl;
                int move_cnt = 0;
                for(auto& k: k_clusters){
                    if(k->UpdateCoor_CheckMove() == true){
                        no_move = false;
                        move_cnt++;
                    }
                }
                // if(cluster_target_size == 2){
                //     cout << "move num: " << move_cnt << "/" << k_clusters.size() << endl;
                // } 
                // cout << itr_cnt << endl;
                // Update Coor: end

                // Check break: begin
                if(no_move == true || itr_cnt > ITR_LIMIT){
                    break;
                }
                else{
                    for(auto k: k_clusters){
                        k->Clear();
                    }
                }
                // Check break: end
                itr_cnt++;
            }
            // K-means cluster: end

            // Calculate cost per bit for new cluster: begin
            // cout << "Calculating cost-per-bit for new cluster" << endl;
            list<cluster*>::iterator itr1 = k_clusters.begin();
            int cls_cnt = 0;
            while(itr1 != k_clusters.end()){
                cluster* new_cls = *itr1;
                // cout << "Cluster " << cls_cnt << ", size = " << new_cls->size << endl;
                if(new_cls->size == 1){
                    new_cls->members.front()->to_new_cluster = NULL;
                    itr1 = k_clusters.erase(itr1);
                    delete new_cls;
                    continue;
                }
                
                new_cls->mark_recalculate = false;

                for(auto& m: new_cls->members) m->to_new_cluster = new_cls;
                bool print = (cls_cnt == -1) ? true:false;

                new_cls->Calculate_BestCost_FFtype(print, LIB, INST, DIE);

                itr1++;
                cls_cnt++;
            }
            // Calculate cost per bit for new cluster: end

            // Determine new or old cluster stay: begin
            // Initial "is-better-than-new": begin
            // cout << "initial bad top cluster" << endl;
            list<cluster*> bad_old_cluster_list;
            bad_old_cluster_list.clear();
            for(auto& cls: top_list){
                cls->mark_remove = false;
                if(cls->TestQuality(0) == false){
                    bad_old_cluster_list.push_back(cls);
                }
            }
            // Initial "is-better-than-new": end

            int better_cnt = 0;
            for(auto cls: top_list) {
                if(cls->is_better_than_new) better_cnt ++;
            }
            // cout << "Better num: " << better_cnt << endl;
            // cout << "top list : " << top_list.size() << endl;


            // Find out "Good" new cluster: begin
            // cout << "finding out good new cluster" << endl;
            while(1){
                list<cluster*>::iterator itr;
                // if(cluster_target_size==3) cout << "Bad cluster num: " << bad_old_cluster_list.size() << endl;
                // if(cluster_target_size==3) cout << "Good new clusters num: " << k_clusters.size() << endl;
                // Re-calculate cost-per-bit and fftype for new cluster which was marked "re-calculate" : begin
                // if(clk_group_cnt == 1 && cluster_target_size==2)cout << "stage 1" << endl;
                itr = k_clusters.begin();
                while(itr!=k_clusters.end()){
                    const auto new_cls = *itr;

                    if(new_cls->mark_recalculate == true){ // form new top cluster
                        // cout << "mark recalculate true" << endl;
                        new_cls->mark_recalculate = false;
                        list<cluster*>::iterator member_itr = new_cls->members.begin();
                        while(member_itr!=new_cls->members.end()){
                            auto m = *member_itr;
                            if(m->to_top_cluster->is_better_than_new == true){
                                m->to_new_cluster = NULL;
                                member_itr = new_cls->members.erase(member_itr);
                                new_cls->size--;
                            }
                            else{ member_itr++; }
                        }
                        if(new_cls->size == 0){ 
                            delete new_cls;
                            itr = k_clusters.erase(itr);
                            continue;
                        }
                        else if(new_cls->size == 1){
                            new_cls->members.front()->to_new_cluster = NULL;
                            delete new_cls;
                            itr = k_clusters.erase(itr);
                            continue;
                        }
                        else{ 
                            new_cls->Calculate_BestCost_FFtype(0, LIB, INST, DIE);
                        }
                    }
                    // else{cout << "mark recalculate false" << endl;}
                    itr++;
                }
                // Re-calculate cost-per-bit and fftype for new cluster which was marked "re-calculate" : end

                // Re-estimate "is-better-than-new" in bad clusters: begin
                // if(clk_group_cnt == 1 && cluster_target_size==2)cout << "stage 2" << endl;
                bool check_again_new_clusters = false;
                itr = bad_old_cluster_list.begin();
                while(itr!=bad_old_cluster_list.end()){

                    const auto bad_top_cluster = *itr;


                    // int print = false;
                    // if(clk_group_cnt == 1 && cluster_target_size==2 && c==108) print = true;
                    if(bad_top_cluster->TestQuality(0) == true){
                        itr = bad_old_cluster_list.erase(itr);
                        check_again_new_clusters = true;
                        continue;
                    }
                    itr++;
                }
                // if(clk_group_cnt == 1 && cluster_target_size==2)cout << "stage 2 finish" << endl;
                // Re-estimate "is-better-than-new" in bad clusters: end

                // if(clk_group_cnt == 1 && cluster_target_size==2)
                //     cout << "check_again_new_clusters = " << check_again_new_clusters << endl;
                if(check_again_new_clusters == false){
                    break;
                }
            }
            // Find out "Good" new cluster: end

            // Note: After doing "{Find out "Good" new cluster}" step,
            //       the new clusters remain in "k_clusters" list are good new cluster.
            
            // Remove bad cluster from top cluster list: begin
            // if(clk_group_cnt == 1 && cluster_target_size==2)
            //     cout << "removing bad cluster" << endl;
            // cout << "Bad cluster num: " << bad_old_cluster_list.size() << endl;
            // for(auto cls: bad_old_cluster_list) cout << cls->size << " -> ";
            // cout << endl;
            // for(auto cls: bad_old_cluster_list){
            //     if(cls->size == 1){
            //         cout << cls->single_bit_ff->name << " - > ";
            //     }
            //     else{
            //         for(auto m: cls->members) {
            //             cout << m->single_bit_ff->name << " - > ";
            //         }
            //     }
            // }
            // cout << endl;
            // cout << "Good new clusters num: " << k_clusters.size() << endl;
            // for(auto cls: k_clusters) cout << cls->size << " -> ";
            // cout << endl;
            // for(auto cls: k_clusters){
            //     if(cls->size == 1){
            //         cout << cls->single_bit_ff->name << " - > ";
            //     }
            //     else{
            //         for(auto m: cls->members) cout << m->single_bit_ff->name << " - > ";
            //     }
            // }
            // cout << endl;
            for(auto bad_cls: bad_old_cluster_list){
                bad_cls->mark_remove = true;
            }
            list<cluster*>::iterator itr2 = top_list.begin();
            while(itr2!=top_list.end()){
                auto cls = *itr2;
                if(cls->mark_remove == true){
                    if(cls->size == 1){
                        cls->to_top_cluster = cls->to_new_cluster;
                        cls->is_top = false;
                        itr2 = top_list.erase(itr2);
                        continue;
                    }
                    else{
                        for(auto m: cls->members){
                            if(m->to_new_cluster == NULL){
                                m->is_top = true;
                                m->to_top_cluster = m;
                                k_clusters.push_back(m);
                            }
                            else{
                                m->is_top = false;
                                m->to_top_cluster = m->to_new_cluster;
                            }
                        }
                        delete cls;
                        itr2 = top_list.erase(itr2);
                        continue;
                    }
                }
                itr2++;
            }
            // Remove bad cluster from top cluster list: end

            // Add good new clusters into "top_list": begin
            for(auto cls: k_clusters){
                cls->to_new_cluster = NULL;
                cls->is_top = true;
                top_list.push_back(cls);
            }
            // Add good new clusters into "top_list": end

            // Determine new or old cluster stay: end

            // Check top list ff number: begin
            // int cnt_ = 0;
            // for(auto cls: top_list){
            //     cnt_ = cnt_ + cls->size;
            // }
            // if(cnt_ != ff_list->size()){
            //     cout << "ERROR" << endl;
            //     cout << "Top list num: " << cnt_ << endl;
            //     cout << "Actual num: " << ff_list->size() << endl;
            //     return;
            // }
            // Check top list ff number: end
        }


        // Map clusters int "top_list" to mbff and put into "UPFFS": begin
        string inst_name;
        for(auto cls: top_list){
            inst_name = "";
            inst_name = inst_name + "NEWFF" + to_string(newff_cnt);

            if(cls->size == 1){
                cls->single_bit_ff->name = inst_name;
                cls->single_bit_ff->type = cls->type;
                cls->single_bit_ff->new_coor();
                UPFFS->push_back(cls->single_bit_ff);
            }  
            else{
                ffi* mbff = new ffi(inst_name, 0, 0);
                mbff->type = cls->type;
                mbff->d_pins.reserve(cls->size);
                mbff->q_pins.reserve(cls->size);
                for(auto p: cls->optseq_D) mbff->d_pins.push_back(p);
                for(auto p: cls->optseq_Q) mbff->q_pins.push_back(p);
                mbff->clk_pin = new pin;
                mbff->new_coor();
                UPFFS->push_back(mbff);
            }
            newff_cnt++;
        }
        // Map clusters int "top_list" to mbff and put into "UPFFS": end
        clk_group_cnt++;
    }

    return;
}

void banking::Initial_SingleBit_Cls(){
    // cout << "Initializing Single Bit Cluster" << endl;
    single_bit_clusters.clear();
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
        // cout << "finding best fftype for ff: " << f->name << endl; 
        for(auto ftype: LIB->fftable_cost[1]){
            double cost = 0;
            double slack, ns;
            list<pin*> optseq_D;
            list<pin*> optseq_Q;
            slack = INST->TnsTest(0, dpins, qpins, ftype, DIE->displacement_delay, optseq_D, optseq_Q);
            ns = (slack > 0) ? 0 : abs(slack);
            cost = DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area;
        
            if(cost < mincost){
                mincost = cost;
                mincost_ftype = ftype;
            }
        }

        cls->type = mincost_ftype;
        cls->single_bit_ff = f;
        cls->cost_per_bit = mincost;
    }
    return;
}

bool banking::sort_single_bit_cls(cluster* a, cluster* b){
    return a->single_bit_ff->cen_x < b->single_bit_ff->cen_x;
}

void banking::PrintResult(){
    int bit_cnt = 0;
    int sb_cnt = 0;
    int arr[5] = {0, 0, 0, 0, 0};
    for(auto& f: *UPFFS){
        bit_cnt = bit_cnt + f->d_pins.size();
        if(f->d_pins.size() == 1) sb_cnt ++;
        arr[f->d_pins.size()]++;
    }
    cout << endl;
    cout << "Cluster Result >>>" << endl;
    cout << "*************************************" << endl;
    cout << "Total bit num: " << bit_cnt << endl;
    cout << "Cluster num : " << bit_cnt - sb_cnt << endl;
    cout << "Non cluster num: " << sb_cnt << endl;
    cout << "-------------------------------------" << endl;
    for(int i=1; i<5; i++) cout << "Type " << i << ": " << arr[i] << endl;
    cout << "*************************************" << endl;
    cout << endl;
}