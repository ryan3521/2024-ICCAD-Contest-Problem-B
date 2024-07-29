#include "costeva.h"

costeva::costeva(dieInfo* DIE, lib* LIB, inst* INST){
    this->DIE = DIE;
    this->LIB = LIB;
    this->INST = INST;
}

double costeva::evaluate(list<ffi*>* ff_list){
    this->ffs = ff_list;
    
    // *********************************************************** //
    // Cost = alpha*TNS + beta*Power + gamma*Area + lambda*Density //
    // *********************************************************** //

    double TNS     = calTns();
    double Power   = calPower();
    double Area    = calArea();
    double Density = calDensity();

    double cost = DIE->Alpha*TNS + DIE->Beta*Power + DIE->Gamma*Area + DIE->Lambda*Density;

    return cost;
}

double costeva::calArea(){
    double area = 0;

    for(auto f: *ffs){
        area = area + f->type->area;
    }

    return area;
}

double costeva::calPower(){
    double power = 0;

    for(auto f: *ffs){
        power = power + f->type->gate_power;
    }

    return power;
}

double costeva::calTns(){

    for(auto& it: INST->gate_umap){
        auto g = it.second;
        g->v = false;
        g->is_tracking = false;
    }

    for(auto& f: *ffs){
        f->update_pin_loc();
    }

    double tns = 0;
    for(auto& f: *ffs){
        for(int i=0; i<f->d_pins.size(); i++){
            double slack;
            pin* p = f->d_pins[i];
            
            if(p->to_net->ipins.front()->pin_type == 'f'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
                double new_hpwl = abs(sp->new_coox - p->new_coox) + abs(sp->new_cooy - p->new_cooy);
                double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
                double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay) - (new_qpin_delay - ori_qpin_delay);
            }
            else if(p->to_net->ipins.front()->pin_type == 'd'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            }
            else if(p->to_net->ipins.front()->pin_type == 'g'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);

                slack = p->dspd_slk - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                slack = slack + get_ns(sp->to_gate);
            }
            if(slack < 0){
                tns = tns - slack;
            }  
        }
    }

    return tns;
}

double costeva::get_ns(gatei* g){
    if(g->v == true){
        return g->ns;
    }
    
    if(g->is_tracking == true){
        return numeric_limits<double>::max();
    }
    g->is_tracking = true;
    double min_ns = numeric_limits<double>::max();

    for(auto p: g->ipins){
        if(p->to_net == NULL) continue;

        double temp_ns;
        if(p->to_net->ipins.front()->pin_type == 'g'){
            temp_ns = get_ns(p->to_net->ipins.front()->to_gate);
            if(temp_ns < min_ns) min_ns = temp_ns;
        }
        else if(p->to_net->ipins.front()->pin_type == 'f'){
            auto sp = p->to_net->ipins.front(); // sp: source pin
            double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
            double new_hpwl = abs(sp->new_coox - p->coox) + abs(sp->new_cooy - p->cooy);
            double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
            double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();
            temp_ns = g->get_critical_slack() - (new_qpin_delay - ori_qpin_delay) - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            if(temp_ns < min_ns) min_ns = temp_ns;
        }
        else if(p->to_net->ipins.front()->pin_type == 'd'){
            continue;
        }
    }
    g->v = true;
    g->ns = min_ns;
    return min_ns;
}

double costeva::calDensity(){
    return 0;
}