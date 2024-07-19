#include "inst.h"

inst::inst(){
    ff_num = 0;
    type_umap.clear();
    ff_umap.clear();
    gate_umap.clear();
}



void inst::add_inst(lib& LIB, string inst_name, string typename_, double coox, double cooy){
    cell* cell_ptr = LIB.get_cell(typename_);
    if(cell_ptr == NULL){
        cout << "Instance: " << inst_name << endl;
        cout << "TypeName: " << typename_ << endl;
        cout << "Error: Can't find match type." << endl;
    }
    ffi* new_ffi;
    gatei* new_gatei;
    
    if(cell_ptr->get_typename() == "FlipFlop"){
        new_ffi = new ffi(inst_name, coox, cooy);
        new_ffi->set_type((ffcell*)cell_ptr);
        new_ffi->initial_PinInfo();
        ff_umap.insert(pair<string, ffi*>(new_ffi->get_name(), new_ffi));
        type_umap.insert(pair<string, bool>(new_ffi->get_name(), 0));
        ff_num = ff_num + new_ffi->type->bit_num;
    }
    else{
        new_gatei = new gatei(inst_name, coox, cooy);
        new_gatei->set_type((gcell*)cell_ptr);
        new_gatei->initial_PinInfo();
        gate_umap.insert(pair<string, gatei*>(new_gatei->get_name(), new_gatei));
        type_umap.insert(pair<string, bool>(new_gatei->get_name(), 1));
    }
}

void inst::set_TSlack(string inst_name, string pin_name, double slack){
    auto it = ff_umap.find(inst_name);

    if(it != ff_umap.end()){
        it->second->set_TSlack(pin_name, slack);
    }
    
    return;
}

void inst::PrintFF(){
    int i=0;

    for(auto& ff: ff_umap){
        i++;

        cout << endl;
        cout << "Inst Name: " << ff.first << endl;
        cout << "Type Name: " << ff.second->get_type()->get_name() << endl;
        cout << "Pos X: "     << ff.second->get_coor().first << endl;  
        cout << "Pos Y: "     << ff.second->get_coor().second << endl;

        for(int j=0; j<ff.second->d_pins.size(); j++){
            cout << "Pin Name: " << ff.second->d_pins[j]->name << " -> Slack = " << ff.second->d_pins[j]->slack << endl;
        }
        cout << endl;
    }
}

void inst::PrintGate(){
    int i=0;

    for(auto& gate: gate_umap){
        i++;

        cout << endl;
        cout << "Inst Name: " << gate.first << endl;
        cout << "Type Name: " << gate.second->type->get_name() << endl;
        cout << "Pos X: "     << gate.second->coox << endl;  
        cout << "Pos Y: "     << gate.second->cooy << endl;
        cout << endl;   
    }
}

 

double get_critical_slack(net* net_ptr){
    // This recursive function will return the minimum slack received from net opins.
    // It will visit all the pins which belong to net_ptr->opins.
    // Also mark the opins connected "GATE" and "FF" if visited.

    double temp_slack;
    double critical_slack = numeric_limits<double>::max();

    for(pin* pin_ptr : net_ptr->opins){
        if(pin_ptr->pin_type == 'g'){
            if(pin_ptr->to_gate->is_visited()){
                temp_slack = pin_ptr->to_gate->get_critical_slack();
            } 
            else{
                double gate_cslack = numeric_limits<double>::max();
                for(int i=0; i<pin_ptr->to_gate->opins.size(); i++){
                    temp_slack = get_critical_slack(pin_ptr->to_gate->opins[i]->to_net);
                    if(temp_slack < gate_cslack) gate_cslack = temp_slack;
                }
                pin_ptr->to_gate->visit(gate_cslack);
                temp_slack = gate_cslack;
            }
            if(temp_slack < critical_slack) critical_slack = temp_slack;
        }
        else if(pin_ptr->pin_type == 'f'){
            if(pin_ptr->dspd_slk < critical_slack) critical_slack = pin_ptr->dspd_slk;
        }
        else if(pin_ptr->pin_type == 'd'){
            // Not need to do any thing.
        }
        else{
            cout << "error: Can not find pin type!" << endl;
            return -1;
        }
    }

    return critical_slack;
}

void inst::SlackDispense(dieInfo& DIE){
    int pcnt;
    bool no_pos_slack; // (no positive slack)
    double slack_sum;
    double min_pos_slack;
    ffi* ffptr;
    // calculate Q pin slack
    for (auto it = ff_umap.begin(); it != ff_umap.end(); it++) {
        // pcnt = 0;
        // slack_sum = 0;
        min_pos_slack = numeric_limits<double>::max();
        no_pos_slack = true;

        ffptr = it->second;

        for(int i=0; i<ffptr->q_pins.size(); i++){
            ffptr->q_pins[i]->dspd_slk = get_critical_slack(ffptr->q_pins[i]->to_net);
            if(ffptr->q_pins[i]->dspd_slk == numeric_limits<double>::max()){
                ffptr->q_pins[i]->dspd_slk = ffptr->d_pins[i]->slack;
            }
            if(ffptr->q_pins[i]->dspd_slk < min_pos_slack/* && ffptr->q_pins[i]->dspd_slk > 0*/){
                no_pos_slack = false;
                min_pos_slack = ffptr->q_pins[i]->dspd_slk;
            }
        }  

        for(int i=0; i<ffptr->d_pins.size(); i++){
            if(ffptr->d_pins[i]->dspd_slk < min_pos_slack/* && ffptr->d_pins[i]->dspd_slk > 0*/){
                no_pos_slack = false;
                min_pos_slack = ffptr->d_pins[i]->dspd_slk;
            }
        } 

        //ffptr->allow_dis = (no_pos_slack==true) ? 0 : min_pos_slack/(DIE.displacement_delay);
        ffptr->allow_dis = min_pos_slack/(DIE.displacement_delay);
    }

    // return redundant slack back to d pins
    for(auto& it: ff_umap){
        for(auto& p: it.second->d_pins){
            if(p->to_net->ipins.front()->pin_type == 'g'){
                if(p->to_net->ipins.front()->to_gate->get_critical_slack() == numeric_limits<double>::max()){
                    p->dspd_slk = p->slack;
                }
                else if(p->dspd_slk > p->to_net->ipins.front()->to_gate->get_critical_slack()){
                    p->dspd_slk = p->dspd_slk + (p->dspd_slk - p->to_net->ipins.front()->to_gate->get_critical_slack());
                }
            }
            else if(p->to_net->ipins.front()->pin_type == 'd'){
                p->dspd_slk = p->slack;
            }
        }
    }
    return;
}

void inst::DebankAllFF(lib& LIB){
    // If ff is multibit ff, debank into single bit ff.
    // The type of all single bit ffs will be the lowest cost one bit ff. 
    int ff_cnt = 0;
    ffi* new_fi;
    ffcell* new_type;
    string inst_name;

    for(auto& ori_list: ffs_ori){ 
        list<ffi*>* sing_list = new list<ffi*>;
        ffs_sing.push_back(sing_list);
        for(auto& ori_ff: *ori_list){
            for(int i=0; i<ori_ff->d_pins.size(); i++){
                inst_name = "";
                inst_name = inst_name + "NFSB" + to_string(ff_cnt);
                new_fi = new ffi(inst_name, 0, 0);
                new_type = LIB.fftable_cost[1].front();

                new_fi->type = new_type;

                ori_ff->d_pins[i]->to_new_ff = new_fi;
                ori_ff->q_pins[i]->to_new_ff = new_fi;
                
                new_fi->d_pins.push_back(ori_ff->d_pins[i]);
                new_fi->q_pins.push_back(ori_ff->q_pins[i]);
    
                new_fi->new_coor();
                new_fi->clk_pin = new pin;

                sing_list->push_back(new_fi);
                ff_cnt++;
            }
        }
    }
}

void inst::ConstructFSR(dieInfo& DIE){
    for(auto& fflist: ffs_sing){
        for(auto& f: *fflist){
            f->calFSR(DIE);
        }
    }
}

ffi::ffi(string name, double coox, double cooy){
    this->name = name;
    this->coox = coox;
    this->cooy = cooy;
    d_pins.clear();
    q_pins.clear();
}

void ffi::set_type(ffcell* type){
    this->type = type;
    cen_x = coox + type->size_x/2;
    cen_y = cooy + type->size_y/2;
}

void ffi::set_TSlack(string d_pin_name, double slack){
    int pin_idx;
    string str = d_pin_name;
    str.erase(0, 1);
    if(str.size() != 0){
        pin_idx = stoi(str);
    }
    else{
        pin_idx = 0;
    }

    d_pins[pin_idx]->slack = slack;
    d_pins[pin_idx]->dspd_slk = slack/2;

    return;    
}


string ffi::get_name(){return name;}

ffcell* ffi::get_type(){return type;}

pair<double, double> ffi::get_coor(){
    pair<double, double> coor_pair(coox, cooy);
    return coor_pair;
}

void ffi::initial_PinInfo(){
    pin* new_pin;
    int bit_num = type->bit_num;
    d_pins.resize(bit_num, NULL);
    q_pins.resize(bit_num, NULL);

    // D pins
    for(int i=0; i<bit_num; i++){
        new_pin = new pin;
        new_pin->name = type->d_pins[i].name;
        new_pin->pin_type = 'f';
        new_pin->to_ff = this;
        new_pin->coox = coox + type->d_pins[i].x_plus;
        new_pin->cooy = cooy + type->d_pins[i].y_plus;
        d_pins[i] = new_pin;
    }

    // Q pins
    for(int i=0; i<bit_num; i++){
        new_pin = new pin;
        new_pin->name = type->q_pins[i].name;
        new_pin->pin_type = 'f';
        new_pin->to_ff = this;
        new_pin->coox = coox + type->q_pins[i].x_plus;
        new_pin->cooy = cooy + type->q_pins[i].y_plus;
        q_pins[i] = new_pin;
    }


    // CLK pin
    clk_pin = new pin;
    clk_pin->name = type->clk_pin.name;
    clk_pin->pin_type = 'f';
    clk_pin->to_ff = this;
    clk_pin->coox = coox + type->clk_pin.x_plus;
    clk_pin->cooy = cooy + type->clk_pin.y_plus;
    return;
}

void ffi::new_coor(){
    int bit = d_pins.size(); // this bit is the effective bit number, not the same as the "type->bit_num";
    double mx = 0;
    double my = 0;
    double rx = 0; // relative centroid
    double ry = 0; // relative centroid

    for(int i=0; i<bit; i++){
        mx = mx + d_pins[i]->coox + q_pins[i]->coox;
        my = my + d_pins[i]->cooy + q_pins[i]->cooy;
    }
    mx = mx/(double)(2*bit);
    cen_x = mx;
    my = my/(double)(2*bit);
    cen_y = my;

    for(int i=0; i<bit; i++){
        rx = rx + type->d_pins[i].x_plus + type->q_pins[i].x_plus;
        ry = ry + type->d_pins[i].y_plus + type->q_pins[i].y_plus;
    }
    rx = rx/(double)(2*bit);
    ry = ry/(double)(2*bit);
    
    coox = ((mx - rx) < 0) ? 0 : (mx - rx);
    cooy = ((my - ry) < 0) ? 0 : (my - ry);

    for(int i=0; i<bit; i++){
        d_pins[i]->new_name = type->d_pins[i].name;
        d_pins[i]->new_coox = coox + type->d_pins[i].x_plus;
        d_pins[i]->new_cooy = cooy + type->d_pins[i].y_plus;
        
        q_pins[i]->new_name = type->q_pins[i].name;
        q_pins[i]->new_coox = coox + type->q_pins[i].x_plus;
        q_pins[i]->new_cooy = cooy + type->q_pins[i].y_plus;
    }
    return;
}

bool ffi::is_too_far(double x, double y, double displacement_delay){
    double rel_x = x - coox;
    double rel_y = y - cooy;

    int neg_cnt = 0;
    int bit_num = d_pins.size();

    for(auto& p: d_pins){
        double pin_x = p->new_coox + rel_x;
        double pin_y = p->new_cooy + rel_y;
        double hpwl = abs(pin_x - p->coox) + abs(pin_y - p->cooy);
        double allow = (p->dspd_slk/displacement_delay)>0 ? (p->dspd_slk/displacement_delay) : 0;
        if(hpwl > allow) neg_cnt++;
    }
    //if(neg_cnt > bit_num) return true;
    if(neg_cnt > bit_num/2) return true;
    else return false;
}

bool ffi::allow_displace(double target_x, double target_y, double displacement_delay){
    double allow_hpwl;
    double dis_hpwl;
    
    // verify D pin
    for(auto& p: d_pins){
        dis_hpwl   = ceil(p->to_net->ipins.front()->coox - target_x) + ceil(p->to_net->ipins.front()->cooy - target_y);
        allow_hpwl = ceil(p->coox - p->to_net->ipins.front()->coox) 
                   + ceil(p->cooy - p->to_net->ipins.front()->cooy); 

        allow_hpwl = allow_hpwl + p->dspd_slk/displacement_delay;

        if(allow_hpwl < dis_hpwl){
            return false;
        }
    }

    // verify Q pin
    for(auto& p: q_pins){
        for(auto& to_p: p->to_net->opins){
            if(to_p->pin_type == 'd'){
                continue;
            }
            else if(to_p->pin_type == 'g'){
                dis_hpwl   = ceil(to_p->coox - target_x) + ceil(to_p->cooy - target_y);
                allow_hpwl = ceil(p->coox - to_p->coox) + ceil(p->cooy - to_p->cooy); 
                allow_hpwl = allow_hpwl + to_p->to_gate->get_critical_slack()/displacement_delay;

                if(allow_hpwl < dis_hpwl){
                    return false;
                }
            }
            else if(to_p->pin_type == 'f'){
                dis_hpwl   = ceil(to_p->coox - target_x) + ceil(to_p->cooy - target_y);
                allow_hpwl = ceil(p->coox - to_p->coox) + ceil(p->cooy - to_p->cooy); 
                allow_hpwl = allow_hpwl + to_p->dspd_slk/displacement_delay;
                if(allow_hpwl < dis_hpwl){
                    return false;
                }
            }
        }
    }
    return true;
}

void ffi::calFSR(dieInfo& DIE){
    pin* dpin = d_pins[0];
    pin* qpin = q_pins[0];

    for(auto& to_pin: dpin->to_net->ipins){
        double radius;

        // calcualate radius (movable HPWL)
        radius = abs(to_pin->coox - cen_x) + abs(to_pin->cooy - cen_y) + dpin->dspd_slk/DIE.displacement_delay;

        // rotate -45 degree
        // x' = x + y
        // y' = y - x
        
        fsr.xmax = (cen_x + radius) + (cen_y); 
        fsr.xmin = (cen_x) + (cen_y - radius);
        fsr.ymax = (cen_y + radius) - (cen_x);
        fsr.ymin = (cen_y - radius) - (cen_x);
    }
    for(auto& to_pin: qpin->to_net->opins){
        double radius;
        double xmax;
        double xmin;
        double ymax;
        double ymin;

        // calcualate radius (movable HPWL)
        if(to_pin->pin_type == 'g'){
            radius = abs(to_pin->coox - cen_x) + abs(to_pin->cooy - cen_y) + to_pin->to_gate->get_critical_slack()/DIE.displacement_delay;
            if(to_pin->to_gate->get_critical_slack() == numeric_limits<double>::max()) {
                radius = (DIE.die_height > DIE.die_width) ? DIE.die_height : DIE.die_width;
            }
        }
        else if(to_pin->pin_type == 'd'){
            radius = (DIE.die_height > DIE.die_width) ? DIE.die_height : DIE.die_width;
        }
        else if(to_pin->pin_type == 'f'){
            radius = abs(to_pin->coox - cen_x) + abs(to_pin->cooy - cen_y) + to_pin->dspd_slk/DIE.displacement_delay;
        }

        // rotate -45 degree
        // x' = x + y
        // y' = y - x
        
        xmax = (cen_x + radius) + (cen_y); 
        xmin = (cen_x) + (cen_y - radius);
        ymax = (cen_y + radius) - (cen_x);
        ymin = (cen_y - radius) - (cen_x);

        // update FSR
        if(xmin > fsr.xmin) fsr.xmin = xmin;
        if(xmax < fsr.xmax) fsr.xmax = xmax;
        if(ymin > fsr.ymin) fsr.ymin = ymin;
        if(ymax < fsr.ymax) fsr.ymax = ymax;
    }

    if(fsr.xmin > fsr.xmax || fsr.ymin > fsr.ymax){
        fsr.can_move = false;
        fsr.xmax = cen_x + cen_y;
        fsr.xmin = fsr.xmax;
        fsr.ymax = cen_y - cen_x;
        fsr.ymin = fsr.ymax;
    } 
    else{
        fsr.can_move = true;
        fsr.cen_x = (fsr.xmax + fsr.xmin)/2;
        fsr.cen_y = (fsr.ymax + fsr.ymin)/2;
    }
}

void reg::update_cen(){
    cen_x = (dpin->coox + qpin->coox)/2;
    cen_y = (dpin->cooy + qpin->cooy)/2;
}



gatei::gatei(string name, double coox, double cooy){
    this->name = name;
    this->coox = coox;
    this->cooy = cooy;
    v = false;
    critical_slack = numeric_limits<double>::max();
}

void gatei::set_type(gcell* type){
    this->type = type;    
}

string gatei::get_name(){return name;}

gcell* gatei::get_type(){return type;}

void gatei::initial_PinInfo(){
    pin* new_pin;

    ipins.resize(type->in_pins.size(), NULL);
    opins.resize(type->out_pins.size(), NULL);

    // IN pins
    for(int i=0; i<type->in_pins.size(); i++){
        new_pin = new pin;
        new_pin->name = type->in_pins[i].name;
        new_pin->pin_type = 'g';
        new_pin->to_gate = this;
        new_pin->coox = coox + type->in_pins[i].x_plus;
        new_pin->cooy = cooy + type->in_pins[i].y_plus;
        ipins[i] = new_pin;
    }

    // OUT pins
    for(int i=0; i<type->out_pins.size(); i++){
        new_pin = new pin;
        new_pin->name = type->out_pins[i].name;
        new_pin->pin_type = 'g';
        new_pin->to_gate = this;
        new_pin->coox = coox + type->out_pins[i].x_plus;
        new_pin->cooy = cooy + type->out_pins[i].y_plus;
        opins[i] = new_pin;
    }
    return;
}

void gatei::visit(double critical_slack){
    v = true;
    this->critical_slack = critical_slack;
}

bool gatei::is_visited(){return v;}

double gatei::get_critical_slack(){return critical_slack;}