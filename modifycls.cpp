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
    else{
        for(auto& f: memb_ffs){
            sum_x = sum_x + f->fsr.cen_x;
            sum_y = sum_y + f->fsr.cen_y;
            cnt = cnt + 1;
        }        
        for(auto& f: cand_ffs){
            if((f->fsr.xmin > fsr_xmax || f->fsr.xmax < fsr_xmin || f->fsr.ymin > fsr_ymax || f->fsr.ymax < fsr_ymin) == false){
                sum_x = sum_x + f->fsr.cen_x;
                sum_y = sum_y + f->fsr.cen_y;
                cnt = cnt + 1;
            }
        }
        pos_x = sum_x/cnt;
        pos_y = sum_y/cnt;
    }
    
}

void cls::clear_ffs(){
    size = 0;
    memb_ffs.clear();
    cand_ffs.clear();
}


void cls::add_ff(ffi* f){
    double hpwl_diff;

    if(can_merge(f, hpwl_diff) == true && size < size_limit){
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


banking::banking(inst* INST, lib* LIB, dieInfo* DIE){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
}

void banking::modifyKmeans(){
    int itr_max = 10;

    for(auto& ff_list: INST->ffs_sing){
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
            uncls_ffs.clear();
            // Step 2. Do cluster.
            for(auto& f: *ff_list){
                if(f->fsr.can_move == false){
                    uncls_ffs.push_back(f);
                    continue;
                }
                best_cls = NULL;
                min_hpwl = numeric_limits<double>::max();
                for(auto& c: cluster_list){
                    if(c->can_merge(f, hpwl_cost)){
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
                best_cls->add_ff(f);
            }
            // Step 3. Update loc.
            for(auto& c: cluster_list){
                c->update_loc();
            }
            // Step 4. Calculate cost.
            for(auto& c: cluster_list){

            }
            
            // Step 5. Check insert or delete.

        }

        // Step 6. Merge the unclustered ffs if cost decrease.
    }
}