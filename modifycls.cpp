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

void banking::modifyKmeans(){
    int itr_max = 10;
    int gcnt = 1;
    for(auto& ff_list: INST->ffs_sing){
        // cout << "G" << gcnt << ": " << ff_list->size() << " ============================================ " << endl;
        gcnt++;
        list<cls*> cluster_list;
        list<ffi*> uncls_ffs;
        cls* cls_ptr;
        int cnt = 0;
        int itr_cnt = 0;
        double old_cost = numeric_limits<double>::max();
        double new_cost;
        
        // Step 1. Initial clusters number and location.
        for(auto& f: *ff_list){
            if(cnt == 0){
                cls_ptr = new cls(LIB->max_ff_size, f->fsr.cen_x, f->fsr.cen_y);
                cluster_list.push_back(cls_ptr);
            }
            if(cnt == LIB->max_ff_size-1){
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

        while(itr_cnt < itr_max){
            // cout << endl;
            // cout << "   Itr " << itr_cnt << endl;
            // cout << "   cls num: " << cluster_list.size() << endl;
            uncls_ffs.clear();
            MultiClsFF* mcff_ptr;
            list<MultiClsFF*> mcff_list;
            mcff_list.clear();
            int multi_cls_cnt;
            // Step 2. Do cluster.
            //cout << "   1. Do cluster" << endl;
            for(auto& f: *ff_list){
                if(f->fsr.can_move == false){
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
                            if(c->size > best_cls->size && c->size < LIB->max_ff_size){
                                best_cls = c;
                            }
                            else if(best_cls->size == LIB->max_ff_size && c->size < LIB->max_ff_size){
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
            for(auto& mcff: mcff_list){
                max_size = mcff->to_cls->size;
                best_cls = NULL;
                if(mcff->to_cls->size == mcff->to_cls->size_limit){
                    if(mcff->to_cls->size > mcff->to_cls->memb_ffs.size()) {
                        mcff->to_cls->size--;
                        mcff->to_cls->add_ff(mcff->f);
                    }
                    else {
                        mcff->to_cls->cand_ffs.push_back(mcff->f);
                    }
                }
                else{
                    for(auto& c: cluster_list){
                        if(c->size == c->size_limit){
                            continue;
                        }
                    
                        if(c->can_merge(mcff->f, hpwl_cost)){
                            if(c->size >= max_size){
                                best_cls = c;
                                max_size = c->size;
                            }
                        }
                    }
                    if(best_cls != NULL){
                        mcff->to_cls->size--;
                        best_cls->add_ff(mcff->f);
                    }
                    else{
                        if(mcff->to_cls->size > mcff->to_cls->memb_ffs.size()) {
                            mcff->to_cls->size--;
                            mcff->to_cls->add_ff(mcff->f);
                        }
                        else {
                            mcff->to_cls->cand_ffs.push_back(mcff->f);
                        }
                    }
                }
                

            }

            // Step 3. Update loc.
            //cout << "   2. Update loc" << endl;
            int t_cnt = 0;
            int b_cnt = 0;
            int a_cnt = 0;
            int mbff_cnt = 0;
            for(auto& c: cluster_list){
                //cout << "memb size (b): " << c->size << ", ";
                //cout << "cand size: " << c->cand_ffs.size() ;
                b_cnt = b_cnt + c->size;
                t_cnt = t_cnt + c->size;
                t_cnt = t_cnt +  c->cand_ffs.size();
                c->update_loc();
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
            //for(int i=1; i<5; i++) cout << "Bit " << i << ": " <<  b_array[i] << endl;

            //cout << "      cost = " << new_cost << endl;

            if(new_cost < old_cost){
                old_cost = new_cost;
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
                }
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
                            if(cnt == LIB->max_ff_size){
                                cnt = 0;
                                new_cls = new cls(LIB->max_ff_size, c->pos_x, c->pos_y); 
                                new_cls_list.push_back(new_cls);
                            }
                        }
                        if(cnt >= 2){
                            new_cls = new cls(LIB->max_ff_size, c->pos_x, c->pos_y); 
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
                        new_cls = new cls(LIB->max_ff_size, pos_x, pos_y);
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

        // Step 6. Merge the unclustered ffs if cost decrease.
        // break;
    }
}

void banking::cls_to_mbff(){
    int cnt = 0;
    string inst_name;
    ffi* mbff;
    ffi* sf; // single bit ff

    for(auto& c: CLS){
        // if(c->memb_ffs.size() == 3){
        //     NCLS.push_back(c->memb_ffs.front());
        //     c->memb_ffs.pop_front();
        // }
        for(auto fc: LIB->opt_fftable[c->memb_ffs.size()]){
            inst_name = "";
            inst_name = inst_name + "NFMB" + to_string(cnt);

            mbff = new ffi(inst_name, 0, 0);
            mbff->type = fc;
            mbff->d_pins.reserve(fc->bit_num);
            mbff->q_pins.reserve(fc->bit_num);

            for(int i=0; i<fc->bit_num; i++){
                if(c->memb_ffs.size() <= 0) break;
                
                sf = c->memb_ffs.front();
                sf->d_pins[0]->to_new_ff = mbff;
                sf->q_pins[0]->to_new_ff = mbff;
                mbff->d_pins.push_back(sf->d_pins[0]);
                mbff->q_pins.push_back(sf->q_pins[0]);
                c->memb_ffs.pop_front();
            }      
            mbff->new_coor();
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