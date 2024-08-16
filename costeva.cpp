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

    double oriTns  = calOriTns();
    double TNS     = calTns();
    double Power   = calPower();
    double Area    = calArea();
    double Density = calDensity();

    cout << "Aprox TNS: " << aproxTns() << endl;
    cout << "Ori TNS : " << oriTns << endl;
    cout << "TNS     : " << TNS << endl;
    cout << "Power   : " << Power << endl;
    cout << "Area    : " << Area << endl;
    cout << "Density : " << Density << endl;
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

    // for(auto& f: *ffs){
    //     f->update_pin_loc();
    // }

    double tns = 0;
    for(auto& f: *ffs){
        for(int i=0; i<f->d_pins.size(); i++){
            double slack = 0;
            pin* p = f->d_pins[i];
            
            if(p->to_net->ipins.front()->pin_type == 'f'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
                double new_hpwl = abs(sp->new_coox - p->new_coox) + abs(sp->new_cooy - p->new_cooy);
                double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
                double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay) - (new_qpin_delay - ori_qpin_delay);
                // if(slack < 0){
                //     cout << "neg slack: " << slack << endl;
                //     cout << "given slack: " << p->slack << endl;
                //     cout << "pin ori hpwl: " << ori_hpwl << endl;
                //     cout << "pin new hpwl: " << new_hpwl << endl;
                //     cout << "pin hpwl diff: " << new_hpwl - ori_hpwl << endl;
                //     cout << "q pin delay diff: " << (new_qpin_delay - ori_qpin_delay) << endl;
                //     return 0;
                // }   
            }
            else if(p->to_net->ipins.front()->pin_type == 'd'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                double cen_ori_hpwl = abs(sp->coox - p->to_ff->cen_x) + abs(sp->cooy - p->to_ff->cen_y);
                double cen_new_hpwl = abs(sp->coox - p->to_new_ff->cen_x) + abs(sp->cooy - p->to_new_ff->cen_y);

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);

                // if(slack < 0 && f->type->bit_num != 1){
                //     cout << endl << "***************" << endl;
                //     cout << "neg slack: " << slack << endl;
                //     cout << "ori ff name: " << p->to_ff->name << endl;
                //     cout << "given slack: " << p->slack << endl;
                //     cout << "source pin: " << sp->coox << ", " << sp->cooy << endl;
                //     cout << "target ff ori cen: " << p->to_ff->cen_x << ", " << p->to_ff->cen_y << endl;
                //     cout << "target ff new cen: " << f->cen_x << ", " << f->cen_y << endl;
                //     cout << "target pin ori: " << p->coox << ", " << p->cooy << endl;
                //     cout << "target pin new: " << p->new_coox << ", " << p->new_cooy << endl;
                //     cout << "pin ori hpwl: " << ori_hpwl << endl;
                //     cout << "pin new hpwl: " << new_hpwl << endl;
                //     cout << "pin hpwl diff: " << new_hpwl - ori_hpwl << endl;
                //     cout << "cen ori hpwl: " << cen_ori_hpwl << endl;
                //     cout << "cen new hpwl: " << cen_new_hpwl << endl;
                //     cout << "cen hpwl diff: " << cen_new_hpwl - cen_ori_hpwl << endl;

                //     cout << "fsr cen x: " << f->cen_x + f->cen_y << endl; 
                //     cout << "fsr cen y: " << f->cen_y - f->cen_x << endl; 
                //     cout << endl << "***************" << endl;
                //     return 0;
                // }
                
            }
            else if(p->to_net->ipins.front()->pin_type == 'g'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                double temp_ct  = (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                double cen_ori_hpwl = abs(sp->coox - p->to_ff->cen_x) + abs(sp->cooy - p->to_ff->cen_y);
                double cen_new_hpwl = abs(sp->coox - p->to_new_ff->cen_x) + abs(sp->cooy - p->to_new_ff->cen_y);
                if(get_ct(sp->to_gate) == numeric_limits<double>::lowest()){
                    slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                    // if(slack < 0){
                        // cout << endl << "***************" << endl;
                        // cout << "ff bit num: " << f->type->bit_num << endl;
                        // cout << "source pin coor: " << sp->coox << ", " << sp->cooy << endl;
                        // cout << "ori ff type: " << p->to_ff->type->name << endl;
                        // cout << "new ff type: " << p->to_new_ff->type->name << endl;
                        // cout << "ori ff coor: "  << p->to_ff->coox << ", " << p->to_ff->cooy << endl;
                        // cout << "new ff coor: "  << p->to_new_ff->coox << ", " << p->to_new_ff->cooy << endl;
                        // cout << "ori ff size: " << p->to_ff->type->size_x << ", " << p->to_ff->type->size_y << endl;
                        // cout << "new ff size: " << p->to_new_ff->type->size_x << ", " << p->to_new_ff->type->size_y << endl;
                        // cout << "neg slack: " << slack << endl;
                        // cout << "given slack: " << p->slack << endl;
                        // cout << "pin ori hpwl: " << ori_hpwl << endl;
                        // cout << "pin new hpwl: " << new_hpwl << endl;
                        // cout << "pin hpwl diff: " << new_hpwl - ori_hpwl << endl;
                        // cout << "cen ori hpwl: " << cen_ori_hpwl << endl;
                        // cout << "cen new hpwl: " << cen_new_hpwl << endl;
                        // cout << "cen hpwl diff: " << cen_new_hpwl - cen_ori_hpwl << endl;
                        // cout << "q pin delay diff: " << (new_qpin_delay - ori_qpin_delay) << endl;
                        // cout << endl << "***************" << endl;
                        // return 0;
                    // } 
                }
                else{
                    slack = p->slack - (temp_ct + get_ct(sp->to_gate));
                }
                
            }
            if(slack < 0){
                tns = tns - slack;
            }  
        }
    }

    return tns;
}

double costeva::get_ct(gatei* g){
    if(g->v == true){
        return g->consume_time;
    }
    
    if(g->is_tracking == true){
        return numeric_limits<double>::lowest();
    }
    g->is_tracking = true;
    double max_ct = numeric_limits<double>::lowest(); // ct: consume time

    for(auto p: g->ipins){
        if(p->to_net == NULL) continue;

        double temp_ct;
        if(p->to_net->ipins.front()->pin_type == 'g'){
            temp_ct = get_ct(p->to_net->ipins.front()->to_gate);
            if(temp_ct > max_ct) max_ct = temp_ct;
        }
        else if(p->to_net->ipins.front()->pin_type == 'f'){
            auto sp = p->to_net->ipins.front(); // sp: source pin
            double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
            double new_hpwl = abs(sp->new_coox - p->coox) + abs(sp->new_cooy - p->cooy);
            double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
            double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();


            temp_ct = (new_qpin_delay - ori_qpin_delay) + (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            if(temp_ct > max_ct) max_ct = temp_ct;
        }
        else if(p->to_net->ipins.front()->pin_type == 'd'){
            continue;
        }
    }
    g->v = true;
    g->consume_time = max_ct;
    return max_ct;
}

double costeva::calOriTns(){
    int cnt = 0;
    double tns = 0;

    for(auto f: *ffs){
        for(int i=0; i<f->d_pins.size(); i++){
            cnt++;
            if(f->d_pins[i]->slack < 0) tns = tns + f->d_pins[i]->slack;
        }
    }


    return tns;
}


double costeva::aproxTns(){
    for(auto& f: *ffs){
        f->update_pin_loc();
    }

    double tns = 0;
    double temp_slack;
    double ori_hpwl;
    double new_hpwl;

    for(auto f: *ffs){
        for(auto p: f->q_pins){
            for(auto tp: p->to_net->opins){
                if(tp->pin_type == 'g'){
                    ori_hpwl = abs(tp->coox - p->coox) + abs(tp->cooy - p->cooy);
                    new_hpwl = abs(tp->coox - p->new_coox) + abs(tp->cooy - p->new_cooy);
                    temp_slack = tp->to_gate->get_critical_slack() - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                    if(temp_slack < 0) tns = tns - temp_slack;
                }
            }
        }
        for(auto p: f->d_pins){
            auto sp = p->to_net->ipins.front();
            if(sp->pin_type == 'g'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                temp_slack = p->dspd_slk - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                if(temp_slack < 0) tns = tns - temp_slack;
            }
            else if(sp->pin_type == 'd'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                temp_slack = p->dspd_slk - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                if(temp_slack < 0) tns = tns - temp_slack;
            }
            else if(sp->pin_type == 'f'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->new_coox - p->new_coox) + abs(sp->new_cooy - p->new_cooy);
                temp_slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                if(temp_slack < 0) tns = tns - temp_slack;
            }
        }
    }
    return tns;
}

double costeva::calDensity(){
    return 0;
}