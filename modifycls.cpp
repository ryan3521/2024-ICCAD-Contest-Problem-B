#include "modifycls.h"

cls::cls(int size_limit, double pos_x, double pos_y){
    this->size_limit = size_limit;
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    size  = 0;
    memb_ffs.clear();
    cand_ffs.clear();
}

void cls::update_loc(){
    // 1. If memb_ffs size is 0, use cand_ffs list to update the location.
    // 2. If memb_ffs size not zero, use memb_ffs and those in cand_ffs which can merge with the feasible region to update location.
    double sum_x = 0;
    double sum_y = 0;
    double cnt   = 0;
    
    if(size == 0){
        for(auto& f: cand_ffs){
            sum_x = sum_x + f->fsr.cen_x;
            sum_y = sum_y + f->fsr.cen_y;
            cnt = cnt + 1;
        }
        if(cnt == 0){
            return;
        }
        else{
            pos_x = sum_x/cnt;
            pos_y = sum_y/cnt;
        }
    }
    else if (size == 1){
        for(auto& f: cand_ffs){
            if((f->fsr.xmin > fsr_xmax || fsr_xmin > f->fsr.xmax || f->fsr.ymin > fsr_ymax || fsr_ymin > f->fsr.ymax) == false){
                if(f->fsr.xmax < fsr_xmax) fsr_xmax = f->fsr.xmax;
                if(f->fsr.xmin > fsr_xmin) fsr_xmin = f->fsr.xmin;
                if(f->fsr.ymax < fsr_ymax) fsr_ymax = f->fsr.ymax;
                if(f->fsr.ymin > fsr_ymin) fsr_ymin = f->fsr.ymin;
            }
        }
        pos_x = (fsr_xmin + fsr_xmax) / 2;
        pos_y = (fsr_ymin + fsr_ymax) / 2;  
    }
    else if (size == size_limit){
        pos_x = (fsr_xmin + fsr_xmax) / 2;
        pos_y = (fsr_ymin + fsr_ymax) / 2;       
    }
    else {
        for(auto& f: cand_ffs){
            if((f->fsr.xmin > fsr_xmax || fsr_xmin > f->fsr.xmax || f->fsr.ymin > fsr_ymax || fsr_ymin > f->fsr.ymax) == false){
                if(f->fsr.xmax < fsr_xmax) fsr_xmax = f->fsr.xmax;
                if(f->fsr.xmin > fsr_xmin) fsr_xmin = f->fsr.xmin;
                if(f->fsr.ymax < fsr_ymax) fsr_ymax = f->fsr.ymax;
                if(f->fsr.ymin > fsr_ymin) fsr_ymin = f->fsr.ymin;
            }
        }
        pos_x = (fsr_xmin + fsr_xmax) / 2;
        pos_y = (fsr_ymin + fsr_ymax) / 2;  
    }

    list<ffi*> temp_list;
    for(auto& f: memb_ffs) temp_list.push_back(f);
    for(auto& f: cand_ffs) temp_list.push_back(f);
    memb_ffs.clear();
    cand_ffs.clear();
    cand_canmerge_ffs.clear();
    cand_cannotmerge_ffs.clear();
    size = 0;

    double hpwl; // useless
    for(auto& f: temp_list){
        if(this->can_merge(f, hpwl) == true && size < size_limit){
            memb_ffs.push_back(f);
            if(size == 0){
                fsr_xmax = f->fsr.xmax;
                fsr_xmin = f->fsr.xmin;
                fsr_ymax = f->fsr.ymax;
                fsr_ymin = f->fsr.ymin;
            }
            else{
                if(f->fsr.xmax < fsr_xmax) fsr_xmax = f->fsr.xmax;
                if(f->fsr.xmin > fsr_xmin) fsr_xmin = f->fsr.xmin;
                if(f->fsr.ymax < fsr_ymax) fsr_ymax = f->fsr.ymax;
                if(f->fsr.ymin > fsr_ymin) fsr_ymin = f->fsr.ymin;
            }
            size++;
        } 
        else if(this->can_merge(f, hpwl) == true && size >= size_limit){
            cand_canmerge_ffs.push_back(f);
        }
        else{
            cand_cannotmerge_ffs.push_back(f);
        }
    }
}

void cls::clear_ffs(){
    size = 0;
    memb_ffs.clear();
    cand_ffs.clear();
    cand_canmerge_ffs.clear();
    cand_cannotmerge_ffs.clear();
}


void cls::add_ff(ffi* f){
    double hpwl_diff;

    if(can_merge(f, hpwl_diff) == true && size < size_limit){
        if(memb_ffs.size() == 0){
            fsr_xmax = f->fsr.xmax;
            fsr_xmin = f->fsr.xmin;
            fsr_ymax = f->fsr.ymax;
            fsr_ymin = f->fsr.ymin;
        }
        else{
            if(f->fsr.xmax < fsr_xmax) fsr_xmax = f->fsr.xmax;
            if(f->fsr.xmin > fsr_xmin) fsr_xmin = f->fsr.xmin;
            if(f->fsr.ymax < fsr_ymax) fsr_ymax = f->fsr.ymax;
            if(f->fsr.ymin > fsr_ymin) fsr_ymin = f->fsr.ymin;
        }
        size++;
        memb_ffs.push_back(f);
    }
    else{
        cand_ffs.push_back(f);
    }
}

bool cls::can_merge(ffi* f, double& hpwl_diff){
    // IF:   flip flop feasible region cover this cluster's location point 'pos'
    //       return true
    // ELSE: return false and calculate the hpwl from the FSR edge to location point.

    if(f->fsr.xmax >= pos_x && f->fsr.xmin <= pos_x && f->fsr.ymax >= pos_y && f->fsr.ymin <= pos_y){
        hpwl_diff = 0;
        return true;
    }
    else{
        hpwl_diff = 0;
        // -- x -- 
        if(f->fsr.xmax < pos_x){
            hpwl_diff = hpwl_diff + (pos_x - f->fsr.xmax);
        }
        else if(f->fsr.xmin > pos_x){
            hpwl_diff = hpwl_diff + (f->fsr.xmin - pos_x);
        }
        // -- y --
        if(f->fsr.ymax < pos_y){
            hpwl_diff = hpwl_diff + (pos_y - f->fsr.ymax);
        }
        else if(f->fsr.ymin > pos_y){
            hpwl_diff = hpwl_diff + (f->fsr.ymin - pos_y);
        }

        return false;
    }
}


banking::banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->UPFFS = UPFFS;
}

bool cmp_ff(ffi* a, ffi* b){
    return a->fsr.cen_x < b->fsr.cen_x;
}

bool cmp_priority(pair<int, double> a, pair<int, double> b){
    if(a.second == b.second){
        return a.first < b.first;
    }
    return a.second < b.second;
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

void banking::modifyKmeans(){
    bool err = false;
    int itr_max = 1000;
    int gcnt = 1;
    int ff_cnt = 0;

    initial_size_priority();


    // cout << "Group num: " << INST->ffs_sing.size() << endl;
    for(auto& ff_list: INST->ffs_sing){
        //cout << "G" << gcnt << ": " << ff_list->size() << " ============================================ " << endl;
        gcnt++;
        list<ffi*> tbp_ffs; // tbp: to be placed
        cls* cls_ptr;

        for(auto& f: *ff_list) tbp_ffs.push_back(f);

        auto prior = size_priority.begin();
        while(prior != size_priority.end()){
            
            int cnt = 0;
            int itr_cnt = 0;
            double old_cost = numeric_limits<double>::max();
            double new_cost;
            list<cls*> cluster_list;
            list<ffi*> uncls_ffs;
            
            // cout << "Prior " << prior->first << "========================" << endl;
            tbp_ffs.sort(cmp_ff);
            // cout << "ff num: " << tbp_ffs.size() << endl;
            // Step 1. Initial clusters number and location.
            for(auto& f: tbp_ffs){
                if(cnt == 0){
                    cls_ptr = new cls(prior->first, f->fsr.cen_x, f->fsr.cen_y);
                    cluster_list.push_back(cls_ptr);
                }
                if(cnt == prior->first-1){
                    cnt = 0;
                }
                else{
                    cnt++;
                }
            }


            // Itr Loop:
            cls* best_cls;
            double min_hpwl;
            double hpwl_cost;
            // Do normal K means to initial clusters position --------------------
            for(int i=0; i<5; i++){
                // cluster
                for(auto& f: tbp_ffs){
                    best_cls = NULL;
                    min_hpwl = numeric_limits<double>::max();
                    for(auto& c: cluster_list){
                        hpwl_cost = abs(f->fsr.cen_x - c->pos_x) + abs(f->fsr.cen_y - c->pos_y);
                        if(c->size != c->size_limit && hpwl_cost < min_hpwl){
                            best_cls = c;
                            min_hpwl = hpwl_cost;
                        }
                    }
                    if(best_cls!=NULL){
                        best_cls->size++;
                        best_cls->memb_ffs.push_back(f);
                    }
                }
                // update loc
                for(auto c: cluster_list){
                    if(c->size == 0) continue;
                    double sum_x = 0;
                    double sum_y = 0;
                    for(auto f: c->memb_ffs){
                        sum_x = sum_x + f->fsr.cen_x;
                        sum_y = sum_y + f->fsr.cen_y;
                    }
                    c->pos_x = sum_x/(double)c->size;
                    c->pos_y = sum_y/(double)c->size;
                    c->size = 0;
                    c->memb_ffs.clear();
                }
            }



            bool same = false;
            while(1){
                // cout << endl;
                // cout << "   Itr " << itr_cnt << endl;
                // cout << "   cls num: " << cluster_list.size() << endl;
                uncls_ffs.clear();
                MultiClsFF* mcff_ptr;
                list<MultiClsFF*> mcff_list;
                mcff_list.clear();
                int multi_cls_cnt;
                // Step 2. Do cluster.
                // cout << "   1. Do cluster" << endl;
                
                for(auto& f: tbp_ffs){
                    if(f->fsr.can_move == false){
                        uncls_ffs.push_back(f);
                        continue;
                    }
                    else if(cluster_list.size() == 0){
                        uncls_ffs.push_back(f);
                        continue;
                    }
                    multi_cls_cnt = 0;
                    best_cls = NULL;
                    min_hpwl = numeric_limits<double>::max();
                    for(auto& c: cluster_list){
                        if(c->can_merge(f, hpwl_cost)){
                            multi_cls_cnt++;
                            if(min_hpwl > hpwl_cost){
                                min_hpwl = hpwl_cost;
                                best_cls = c;
                            }
                            else{
                                if(c->size > best_cls->size && c->size < prior->first){
                                    best_cls = c;
                                }
                                else if(best_cls->size == prior->first && c->size < prior->first){
                                    best_cls = c;
                                }
                            }
                        }
                        else{
                            if(min_hpwl > hpwl_cost){
                                min_hpwl = hpwl_cost;
                                best_cls = c;
                            }
                        }
                    }

                    if(multi_cls_cnt > 1){
                        if(best_cls->size < best_cls->size_limit) best_cls->size++;
                        
                        mcff_ptr = new MultiClsFF;
                        mcff_ptr->f = f;
                        mcff_ptr->to_cls = best_cls;

                        mcff_list.push_back(mcff_ptr);
                    }
                    else{
                        best_cls->add_ff(f);
                    }
                }

                int max_size;
                int best_potential_score;
                bool no_move = false;
                for(auto c: cluster_list){
                    c->potential_cnt  = 0;
                    c->potential_score = 0;
                    c->have_potential = false;
                } 
                
                while(no_move == false){
                    no_move = true;
                    for(auto& mcff: mcff_list){
                        best_potential_score = -1;
                        max_size = mcff->to_cls->size;
                        best_cls = NULL;
                        if(mcff->to_cls->size == mcff->to_cls->size_limit){
                            // if(mcff->to_cls->size > mcff->to_cls->memb_ffs.size()) {
                            //     mcff->to_cls->size--;
                            //     mcff->to_cls->add_ff(mcff->f);
                            // }
                            // else {
                            //     mcff->to_cls->cand_ffs.push_back(mcff->f);
                            // }
                            continue;
                        }
                        else{
                            for(auto& c: cluster_list){
                                if(c->size == c->size_limit){
                                    continue;
                                }
                            
                                if(c->can_merge(mcff->f, hpwl_cost) && c!=mcff->to_cls){
                                    if(c->have_potential){
                                        if(best_potential_score == -1){
                                            best_cls = c;
                                            best_potential_score = c->potential_score;
                                            no_move = false;
                                        }
                                        else if(c->potential_score < best_potential_score){
                                            best_cls = c;
                                            best_potential_score = c->potential_score;
                                            no_move = false;
                                        }
                                    }
                                    else if(c->size >= max_size && best_potential_score==-1){
                                        best_cls = c;
                                        max_size = c->size;
                                        no_move = false;
                                    }
                                    c->potential_cnt++;
                                }
                            }
                            if(best_cls != NULL){
                                mcff->to_cls->size--;
                                mcff->to_cls->potential_cnt++;
                                mcff->to_cls = best_cls;
                                best_cls->potential_cnt--;
                                best_cls->size++;
                                // best_cls->add_ff(mcff->f);
                            }
                            else{
                                // if(mcff->to_cls->size > mcff->to_cls->memb_ffs.size()) {
                                //     mcff->to_cls->size--;
                                //     mcff->to_cls->add_ff(mcff->f);
                                // }
                                // else {
                                //     mcff->to_cls->cand_ffs.push_back(mcff->f);
                                // }
                                continue;
                            }
                        }
                    }
                    for(auto c: cluster_list){
                        if(c->size != c->size_limit && c->size+c->potential_cnt >= c->size_limit){
                            c->have_potential = true;
                            no_move = false;
                            c->potential_score = c->potential_cnt;
                        }
                        else{
                            c->have_potential = false;
                        }
                        c->potential_cnt = 0;
                    }
                }

                for(auto& mcff: mcff_list){
                    if(mcff->to_cls->size > mcff->to_cls->memb_ffs.size()) {
                        mcff->to_cls->size--;
                        mcff->to_cls->add_ff(mcff->f);
                    }
                    else {
                        mcff->to_cls->cand_ffs.push_back(mcff->f);
                    }
                }

                // Step 3. Update loc.
                // cout << "   2. Update loc" << endl;
                int t_cnt = 0;
                int b_cnt = 0;
                int a_cnt = 0;
                int mbff_cnt = 0;
                for(auto& c: cluster_list){
                    //cout << "memb size (b): " << c->size << ", ";
                    //cout << "cand size: " << c->cand_ffs.size() ;
                    b_cnt = b_cnt + c->size;
                    //t_cnt = t_cnt + c->size;
                    //t_cnt = t_cnt +  c->cand_ffs.size();
                    c->update_loc();
                    // t_cnt = t_cnt + c->memb_ffs.size() + c->cand_canmerge_ffs.size() + c->cand_cannotmerge_ffs.size();
                    //cout << "; memb size (a): " << c->size  << ", can merge cand size: " << c->cand_canmerge_ffs.size() << endl;
                    a_cnt = a_cnt + c->size;
                    if(c->size > 1) mbff_cnt ++;
                    // cout << "c size: " << c->size << endl;
                }
                //cout << "before size: " << b_cnt << ", after size: " << a_cnt << endl;
                //cout << "t cnt : " << t_cnt << endl;
                //cout << "mbff number: " << mbff_cnt << endl;

                // Step 4. Calculate cost.
                //cout << "   3. Calculate cost" << endl;
                int b_array[5] = {0, 0, 0, 0, 0};
                int bit_cnt = 0;
                new_cost = 0;
                new_cost = new_cost + LIB->mbff_cost[1]*uncls_ffs.size();
                bit_cnt = bit_cnt + uncls_ffs.size();
                b_array[1] = b_array[1]  + uncls_ffs.size();

                for(auto& c: cluster_list){
                    new_cost = new_cost + LIB->mbff_cost[c->size];
                    bit_cnt = bit_cnt + c->size;

                    b_array[c->size]++;
                    new_cost = new_cost + LIB->mbff_cost[1]*(c->cand_canmerge_ffs.size() + c->cand_cannotmerge_ffs.size());
                    bit_cnt = bit_cnt + c->cand_canmerge_ffs.size() + c->cand_cannotmerge_ffs.size();
                    b_array[1] = b_array[1]  + c->cand_canmerge_ffs.size() + c->cand_cannotmerge_ffs.size();

                }

                // cout << "cost " << new_cost << endl;
                t_cnt = 0;
                if(new_cost < old_cost){
                    old_cost = new_cost;
                    same = false;
                }
                else if(new_cost == old_cost && same==false){
                    same = true;
                }
                else{
                    auto prior_next = prior;
                    int lower_bound = prior->first;

                    prior_next++;
                    while(prior_next != size_priority.end()){
                        if(prior_next->first == lower_bound - 1){
                            lower_bound = prior_next->first;
                            prior_next++;
                        }
                        else{
                            break;
                        }
                    }
                    // cout << "Lower bound: " << lower_bound << endl;
                    //cout << "prior next: " << prior_next->first << endl;
                    tbp_ffs.clear();
                    if(prior_next != size_priority.end()){
                        for(auto& c: cluster_list){
                            if(c->size >= lower_bound){
                                CLS.push_back(c);
                                for(auto& f: c->cand_canmerge_ffs){
                                    tbp_ffs.push_back(f);
                                }
                                for(auto& f: c->cand_cannotmerge_ffs){
                                    tbp_ffs.push_back(f);
                                }
                            }
                            else{
                                for(auto& f: c->memb_ffs){
                                    tbp_ffs.push_back(f);
                                }
                                for(auto& f: c->cand_canmerge_ffs){
                                    tbp_ffs.push_back(f);
                                }
                                for(auto& f: c->cand_cannotmerge_ffs){
                                    tbp_ffs.push_back(f);
                                }
                            }
                            for(auto f: uncls_ffs) tbp_ffs.push_back(f);
                        }
                    }
                    else{
                        for(auto& c: cluster_list){
                            for(auto& f: c->cand_canmerge_ffs){
                                NCLS.push_back(f);
                            }
                            for(auto& f: c->cand_cannotmerge_ffs){
                                NCLS.push_back(f);
                            }
                            CLS.push_back(c);
                            t_cnt = t_cnt + c->memb_ffs.size() + c->cand_canmerge_ffs.size() + c->cand_cannotmerge_ffs.size();
                        }
                        for(auto f: uncls_ffs) NCLS.push_back(f);
                    }
                    prior = prior_next;
                    break;
                }
                
                // Step 5. Check insert or delete. 
                //cout << "   4. Check insert or delete" << endl;
                list<cls*> new_cls_list;
                new_cls_list.clear();
                auto it = cluster_list.begin();
                while(it!=cluster_list.end()){
                    auto c = *it;

                    if(c->size <= 1){
                        if(c->cand_cannotmerge_ffs.size() >= 2){
                            // change this cluster position.
                            double sum_x = 0;
                            double sum_y = 0;
                            double cnt   = c->cand_cannotmerge_ffs.size();

                            for(auto& f: c->cand_canmerge_ffs){
                                sum_x = sum_x + f->fsr.cen_x;
                                sum_y = sum_y + f->fsr.cen_y;
                            }
                            c->pos_x = sum_x / cnt;
                            c->pos_y = sum_y / cnt;
                        }
                        else{
                            // delete this cluster.
                            delete c;
                            it = cluster_list.erase(it);
                            continue;
                        }
                    }    
                    else{
                        if(c->cand_canmerge_ffs.size() >= 2){
                            // add new cluster(s).
                            int cnt = 0;
                            cls* new_cls; 

                            for(auto& f: c->cand_canmerge_ffs){
                                cnt++;
                                if(cnt == prior->first){
                                    cnt = 0;
                                    new_cls = new cls(prior->first, c->pos_x, c->pos_y); 
                                    new_cls_list.push_back(new_cls);
                                }
                            }
                            if(cnt >= 2){
                                new_cls = new cls(prior->first, c->pos_x, c->pos_y); 
                                new_cls_list.push_back(new_cls);
                            }
                        }
                        if(c->cand_cannotmerge_ffs.size() >= 2){
                            // add one new cluster.
                            cls* new_cls;
                            double pos_x;
                            double pos_y;
                            double sum_x = 0;
                            double sum_y = 0;
                            double cnt   = c->cand_cannotmerge_ffs.size();

                            for(auto& f: c->cand_canmerge_ffs){
                                sum_x = sum_x + f->fsr.cen_x;
                                sum_y = sum_y + f->fsr.cen_y;
                            }
                            pos_x = sum_x / cnt;
                            pos_y = sum_y / cnt;
                            new_cls = new cls(prior->first, pos_x, pos_y);
                            new_cls_list.push_back(new_cls);
                        }
                    }
                    it++;
                }

                for(auto& c: new_cls_list){
                    cluster_list.push_back(c);
                }
                for(auto& c: cluster_list){
                    c->clear_ffs();
                }
                itr_cnt++;
            }
        }

        // Step 6. Merge the unclustered ffs if cost decrease.

        //break;
    }

}

void banking::cls_to_mbff(){
    int cnt = 0;
    string inst_name;
    ffi* mbff;
    ffi* sf; // single bit ff


    for(auto& c: CLS){
        for(auto fc: LIB->opt_fftable[c->memb_ffs.size()]){
            inst_name = "";
            // Note: "NFMB" mean New FF Multi Bit
            inst_name = inst_name + "NFMB" + to_string(cnt);

            mbff = new ffi(inst_name, 0, 0);
            mbff->type = fc;
            mbff->d_pins.reserve(fc->bit_num);
            mbff->q_pins.reserve(fc->bit_num);

            for(int i=0; i<fc->bit_num; i++){
                if(c->memb_ffs.size() <= 0) break;
                
                sf = c->memb_ffs.front();
                sf->d_pins[0]->new_name = fc->d_pins[i].name;
                sf->d_pins[0]->to_new_ff = mbff;
                sf->q_pins[0]->new_name = fc->q_pins[i].name;
                sf->q_pins[0]->to_new_ff = mbff;
                mbff->d_pins.push_back(sf->d_pins[0]);
                mbff->q_pins.push_back(sf->q_pins[0]);
                c->memb_ffs.pop_front();
            }      
            // determine new mbff coordinate ------------------
            double cen_x_, cen_y_;
            cen_x_ = (c->fsr_xmax + c->fsr_xmin) / 2;
            cen_y_ = (c->fsr_ymax + c->fsr_ymin) / 2;

            mbff->cen_x = (cen_x_ - cen_y_) / 2;
            mbff->cen_y = (cen_x_ + cen_y_) / 2;

            double rx = 0; // relative centroid
            double ry = 0; // relative centroid
            for(int i=0; i<mbff->d_pins.size(); i++){
                rx = rx + mbff->type->d_pins[i].x_plus + mbff->type->q_pins[i].x_plus;
                ry = ry + mbff->type->d_pins[i].y_plus + mbff->type->q_pins[i].y_plus;
            }
            rx = rx/(double)(2*mbff->d_pins.size());
            ry = ry/(double)(2*mbff->d_pins.size());

            mbff->coox = ((mbff->cen_x - rx) < 0) ? 0 : (mbff->cen_x - rx);
            mbff->cooy = ((mbff->cen_y - ry) < 0) ? 0 : (mbff->cen_y - ry);
            // ------------------------------------------------
            mbff->clk_pin = new pin;
            UPFFS->push_back(mbff);   
            cnt++;     
        }
    }

    for(auto& f: NCLS){
        UPFFS->push_back(f);
    }
}

void banking::run(){
    modifyKmeans();
    cls_to_mbff();
    int bit_cnt = 0;
    int sb_cnt = 0;
    int arr[5] = {0, 0, 0, 0, 0};
    for(auto& f: *UPFFS){
        bit_cnt = bit_cnt + f->d_pins.size();
        if(f->d_pins.size() == 1) sb_cnt ++;
        arr[f->d_pins.size()]++;
    }
    cout << "Total bit num: " << bit_cnt << endl;
    cout << "Cluster num : " << bit_cnt - sb_cnt << endl;
    cout << "Non cluster num: " << sb_cnt << endl;

    for(int i=1; i<5; i++) cout << "Type " << i << ": " << arr[i] << endl;

}