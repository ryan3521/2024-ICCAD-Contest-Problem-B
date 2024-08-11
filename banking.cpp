#include "banking.h"




banking::banking(inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* UPFFS){
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->UPFFS = UPFFS;
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


void banking::cls_to_mbff(){
    int cnt = 0;
    string inst_name;
    ffi* mbff;
    ffi* sf; // single bit ff


    for(auto& c: clusters){

        for(auto fc: LIB->opt_fftable[c->size]){
            inst_name = "";
            // Note: "NFMB" mean New FF Multi Bit
            inst_name = inst_name + "NFMB" + to_string(cnt);

            mbff = new ffi(inst_name, 0, 0);
            mbff->type = fc;
            mbff->d_pins.reserve(fc->bit_num);
            mbff->q_pins.reserve(fc->bit_num);

            for(int i=0; i<fc->bit_num; i++){
                if(c->pos_slack_members.size() > 0) {
                    sf = c->pos_slack_members.front();
                    c->pos_slack_members.pop_front();
                }
                else if(c->neg_slack_members.size() > 0){
                    sf = c->neg_slack_members.front();
                    c->neg_slack_members.pop_front();
                }
                else{
                    break;
                }
                
                sf->d_pins[0]->new_name = fc->d_pins[i].name;
                sf->d_pins[0]->to_new_ff = mbff;
                sf->q_pins[0]->new_name = fc->q_pins[i].name;
                sf->q_pins[0]->to_new_ff = mbff;
                mbff->d_pins.push_back(sf->d_pins[0]);
                mbff->q_pins.push_back(sf->q_pins[0]);
            }
            mbff->fsr.can_move = true;    
            mbff->fsr.xmax = c->fsr_xmax;  
            mbff->fsr.xmin = c->fsr_xmin;  
            mbff->fsr.ymax = c->fsr_ymax;  
            mbff->fsr.ymin = c->fsr_ymin;

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

            mbff->coox = (mbff->cen_x - rx);
            mbff->cooy = (mbff->cen_y - ry);
            // ------------------------------------------------
            mbff->clk_pin = new pin;
            UPFFS->push_back(mbff);   
            cnt++;     
        }
    }

    for(auto& f: ncls_ffs){
        UPFFS->push_back(f);
    }
}

void banking::run(){

}