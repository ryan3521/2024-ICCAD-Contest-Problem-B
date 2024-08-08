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

bool banking::cmp_ff2se(ff2se* a, ff2se* b){
    return a->f->fsr.xmax < b->f->fsr.xmax;
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
    // start: initial minimum w,h request (FSR size constrain)
    LIB->opt_fftable[target_size].front()->calculate_min_FSR();
    double min_fsr_w = LIB->opt_fftable[target_size].front()->fsr_min_w;
    double min_fsr_h = LIB->opt_fftable[target_size].front()->fsr_min_h;
    // end: initial minimum w,h request (FSR size constrain)


    // start: construct x sequence
    list<se*> x_sequence;
    for(auto f: banking_ffs){
        if(f->fsr.can_move == false){
            ncls_ffs.push_back(f);
            continue;
        }
        else if((f->fsr.xmax - f->fsr.xmin) < min_fsr_w){
            ncls_ffs.push_back(f);
            continue;
        }
        else if((f->fsr.ymax - f->fsr.ymin) < min_fsr_h){
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
        if((*it)->type == 1){
            (*it)->to_ff->x_seq_e_it = it;
        }
    }
    // end: construct x sequence

    // start: finding clique candidates in x sequence
    list<ff2se*> x_tracking_list;
    while(x_sequence.size() > 0){
        if(x_sequence.front()->type == 0){
            x_tracking_list.push_front(x_sequence.front()->to_ff);
            x_tracking_list.front()->xtrack_it = x_tracking_list.begin();
            x_sequence.pop_front();
        }
        else{
            ff2se* essential_ff = x_sequence.front()->to_ff;
            // start: construct y sequence
            list<se*> y_sequence;
            for(auto it: x_tracking_list){
                se* se_ptr = NULL;
                
                se_ptr = new se{0, it->f->fsr.ymin, it};
                y_sequence.push_back(se_ptr);
                
                se_ptr = new se{1, it->f->fsr.ymax, it};
                y_sequence.push_back(se_ptr);
            }
            y_sequence.sort(cmp_se);
            // end: construct y sequence

            // start: finding clique candidates in y sequence
            list<ff2se*> y_tracking_list;
            for(auto it: y_sequence){
                if(it->type == 0){
                    y_tracking_list.push_front(it->to_ff);
                    y_tracking_list.front()->ytrack_it = y_tracking_list.begin();
                }
                else if(it->type == 1 && it->to_ff != essential_ff){
                    y_tracking_list.erase(it->to_ff->ytrack_it);
                }
                else if(it->type == 1 && it->to_ff == essential_ff){
                    break;
                }
            } 
            // end: finding clique candidates in y sequence

            // start: determine cluster member (from y_tracking_list)
            if(y_tracking_list.size() < target_size){ 
                x_sequence.pop_front();
                x_tracking_list.erase(essential_ff->xtrack_it);
                ncls_ffs.push_back(essential_ff->f);
            }
            else{
                int size = 0;
                double fsr_xmax, fsr_xmin, fsr_ymax, fsr_ymin;
                list<ff2se*> cluster_members;

                y_tracking_list.sort(cmp_ff2se);
                for(auto it=y_tracking_list.begin(); it!=y_tracking_list.end(); it++){
                    if(it == y_tracking_list.begin()){
                        fsr_xmax = (*it)->f->fsr.xmax;
                        fsr_xmin = (*it)->f->fsr.xmin;
                        fsr_ymax = (*it)->f->fsr.ymax;
                        fsr_ymin = (*it)->f->fsr.ymin;
                        cluster_members.push_back(*it);
                        size++;
                    }
                    else{
                        double new_fsr_xmax, new_fsr_xmin;
                        double new_fsr_ymax, new_fsr_ymin;

                        new_fsr_xmax = ((*it)->f->fsr.xmax < fsr_xmax) ? (*it)->f->fsr.xmax : fsr_xmax;
                        new_fsr_xmin = ((*it)->f->fsr.xmin > fsr_xmin) ? (*it)->f->fsr.xmin : fsr_xmin;
                        new_fsr_ymax = ((*it)->f->fsr.ymax < fsr_ymax) ? (*it)->f->fsr.ymax : fsr_ymax;
                        new_fsr_ymin = ((*it)->f->fsr.ymin > fsr_ymin) ? (*it)->f->fsr.ymin : fsr_ymin;

                        double new_w = new_fsr_xmax - new_fsr_xmin;
                        double new_h = new_fsr_ymax - new_fsr_ymin;
                        if(new_w>=min_fsr_w && new_h>=min_fsr_h){
                            fsr_xmax = new_fsr_xmax;
                            fsr_xmin = new_fsr_xmin;
                            fsr_ymax = new_fsr_ymax;
                            fsr_ymin = new_fsr_ymin;
                            cluster_members.push_back(*it);
                            size++;
                            if(size == target_size){
                                break;
                            }
                        }
                        else{
                            continue;
                        } 
                    }
                }

                if(size == target_size){
                    cls* cls_ptr = new cls;
                    cls_ptr->size = size;
                    for(auto& ff2se_ptr: cluster_members){
                        cls_ptr->pos_slack_members.push_back(ff2se_ptr->f);
                        x_sequence.erase(ff2se_ptr->x_seq_e_it);
                        x_tracking_list.erase(ff2se_ptr->xtrack_it);
                    }
                    clusters.push_back(cls_ptr);
                }
                else{
                    x_sequence.pop_front();
                    x_tracking_list.erase(essential_ff->xtrack_it);
                    ncls_ffs.push_back(essential_ff->f);
                }
            }
            // end: determine cluster member (from y_tracking_list)
        }
    }
    // end: finding clique candidates in x sequence
    
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
            // start: integra
            for(auto& sp: size_priority){
                if(sp.first == 1) break; 
                integra(sp.first);
                banking_ffs.clear();
                for(auto f: ncls_ffs){ banking_ffs.push_back(f);}
                ncls_ffs.clear();
            }
            // end: integra

            // start: kmeans
            // end: kmeans

            
        }

    }
}