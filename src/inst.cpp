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
            else if(pin_ptr->to_gate->is_tracking){
                continue;
            }
            else{
                double gate_cslack = numeric_limits<double>::max();
                pin_ptr->to_gate->is_tracking = true;
                // for(int i=0; i<pin_ptr->to_gate->opins.size(); i++){
                for(auto op: pin_ptr->to_gate->opins){
                    if(op->to_net != NULL){
                        temp_slack = get_critical_slack(op->to_net);
                        if(temp_slack < gate_cslack) gate_cslack = temp_slack;
                    } 
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

double get_min_cs(gatei* g){
    if(g->v == true){
        return g->min_cs;
    }
    if(g->is_tracking == true){
        return numeric_limits<double>::max();
    }


    g->is_tracking = true;
    g->min_cs = g->get_critical_slack();

    for(auto p: g->ipins){
        if(p->to_net == NULL) continue;

        auto sp = p->to_net->ipins.front();
        if(sp->pin_type == 'f'){
            g->v = true;
            return g->min_cs;
        }
    }

    for(auto p: g->ipins){
        if(p->to_net == NULL) continue;

        auto sp = p->to_net->ipins.front();
        double temp_min_cs;
        if(sp->pin_type == 'g'){
            temp_min_cs = get_min_cs(sp->to_gate);
            if(temp_min_cs < g->min_cs) {
                g->min_cs = temp_min_cs;
            }
        }
    }
    g->v = true;

    return g->min_cs;
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
            if(ffptr->q_pins[i]->to_net == NULL) continue;
            // cout << "get critical slack : begin" << endl; 
            ffptr->q_pins[i]->dspd_slk = get_critical_slack(ffptr->q_pins[i]->to_net);
            // cout << "get critical slack : end" << endl; 
            if(ffptr->q_pins[i]->dspd_slk == numeric_limits<double>::max()){
                ffptr->q_pins[i]->dspd_slk = ffptr->d_pins[i]->slack;
            }
            if(ffptr->q_pins[i]->dspd_slk < min_pos_slack/* && ffptr->q_pins[i]->dspd_slk > 0*/){
                no_pos_slack = false;
                min_pos_slack = ffptr->q_pins[i]->dspd_slk;
            }
        }  

        // cout << ffptr->name << ": d pin" << endl;
        for(int i=0; i<ffptr->d_pins.size(); i++){
            if(ffptr->q_pins[i]->to_net == NULL) continue;
            if(ffptr->d_pins[i]->dspd_slk < min_pos_slack/* && ffptr->d_pins[i]->dspd_slk > 0*/){
                no_pos_slack = false;
                min_pos_slack = ffptr->d_pins[i]->dspd_slk;
            }
        } 
    }

    // return redundant slack back to d pins
    for(auto& it: gate_umap){
        auto g = it.second;
        g->v = false;
        g->is_tracking = false;
    }

    for(auto& it: ff_umap){
        for(auto& p: it.second->d_pins){
            if(p->to_net == NULL) continue;
            if(p->to_net->ipins.front()->pin_type == 'g'){
                double min_cs = get_min_cs(p->to_net->ipins.front()->to_gate);
                if(min_cs == numeric_limits<double>::max()){
                    p->dspd_slk = p->slack;
                }
                else if(p->dspd_slk > min_cs){
                    p->dspd_slk = p->dspd_slk + (p->dspd_slk - min_cs);
                    if(p->dspd_slk > p->slack) p->dspd_slk = p->slack;
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
    ffi* new_fi = NULL;
    ffcell* new_type;
    string inst_name;


    for(auto& ori_list: ffs_ori){ 
        list<ffi*>* sing_list = new list<ffi*>;
        ffs_sing.push_back(sing_list);
        for(auto& ori_ff: *ori_list){
            int i=0;
            while(i<ori_ff->d_pins.size()){
                if(ori_ff->d_pins[i]->to_net == NULL) {
                    i++;
                    continue;
                }
                // Note: "NFSB" mean New FF Single Bit
                if(new_fi == NULL){
                    inst_name = "";
                    inst_name = inst_name + "NFSB" + to_string(ff_cnt);
                    ff_cnt++;
                    new_fi = new ffi(inst_name, 0, 0);
                    new_fi->size = 0;
                    new_fi->type = LIB.fftable_cost[LIB.min_ff_size].front();
                    new_fi->clk_pin = new pin;
                    sing_list->push_back(new_fi);
                }

                ori_ff->d_pins[i]->to_new_ff = new_fi;
                ori_ff->q_pins[i]->to_new_ff = new_fi;
                
                new_fi->d_pins.push_back(ori_ff->d_pins[i]);
                new_fi->q_pins.push_back(ori_ff->q_pins[i]);
                new_fi->size++;
                i++;
    
                if(new_fi->size == LIB.min_ff_size){
                    new_fi->update_coor();
                    new_fi = NULL;
                }
            }
            if(new_fi != NULL){
                new_fi->update_coor();
                new_fi = NULL;                
            }
        }
    }
}

bool inst::preference_cmp(pair<int, double> a, pair<int, double> b){
    return a.second > b.second;
}

double inst::TnsTest(bool print, list<pin*>& dpins, list<pin*>& qpins, ffcell* type, double coeff, list<pin*>& optseq_D, list<pin*>& optseq_Q){
    int bit_num = dpins.size();
    double ori_pin_cenx = 0;
    double ori_pin_ceny = 0;
    double sumx = 0;
    double sumy = 0;
    list<pin_pair*> pin_pair_list;

    optseq_D.clear();
    optseq_Q.clear();

    ffi pseudo_ff("PseudoFF", 0, 0);
    pseudo_ff.set_type(type);

    // calculate new ff coor: begin
    // if(print) cout << "calculate new ff coor" << endl;
    for(auto p: dpins){ sumx = sumx + p->coox; sumy = sumy + p->cooy; }    
    for(auto p: qpins){ sumx = sumx + p->coox; sumy = sumy + p->cooy; } 

    ori_pin_cenx = sumx / (double)(bit_num*2);   
    ori_pin_ceny = sumy / (double)(bit_num*2);   

    sumx = 0;
    sumy = 0;
    for(int i=0; i<bit_num; i++){
        sumx = sumx + type->d_pins[i].x_plus;
        sumy = sumy + type->d_pins[i].y_plus;
        sumx = sumx + type->q_pins[i].x_plus;
        sumy = sumy + type->q_pins[i].y_plus;
    }
    double temp_x_plus = sumx / (double)(bit_num*2);   
    double temp_y_plus = sumy / (double)(bit_num*2);   
    pseudo_ff.coox = ori_pin_cenx - temp_x_plus;   
    pseudo_ff.cooy = ori_pin_ceny - temp_y_plus;  
    // calculate new ff coor: end

    // initial pin pair list: begin 
    int pin_idx = 0;
    auto d_itr = dpins.begin();
    auto q_itr = qpins.begin();
    while(d_itr!=dpins.end() && q_itr!=qpins.end()){
        // initial pin pair: begin
        pin_pair* ptr = new pin_pair;
        ptr->idx  = pin_idx;
        ptr->dpin = *d_itr;
        ptr->qpin = *q_itr;
        // initial pin pair: end

        pin_pair_list.push_back(ptr);

        pin_idx++;
        d_itr++;
        q_itr++;
    }
    // initial pin pair list: end


    // intitial preference list: begin
    for(auto pp: pin_pair_list){
        for(int idx=0; idx<bit_num; idx++){
            double dpin_coox = pseudo_ff.coox+type->d_pins[idx].x_plus;
            double dpin_cooy = pseudo_ff.cooy+type->d_pins[idx].y_plus;
            double qpin_coox = pseudo_ff.coox+type->q_pins[idx].x_plus;
            double qpin_cooy = pseudo_ff.cooy+type->q_pins[idx].y_plus;

            double d_slack = pp->dpin->CalTns(dpin_coox, dpin_cooy, true,  type, coeff);
            double q_slack = pp->qpin->CalTns(qpin_coox, qpin_cooy, false, type, coeff);


            double total_slack;
            if(d_slack < 0 && q_slack < 0) total_slack = d_slack + q_slack;
            else if(d_slack < 0) total_slack = d_slack;
            else if(q_slack < 0) total_slack = q_slack;
            else total_slack = d_slack + q_slack;

            if(print && total_slack<0){
                cout << "D pin: " << d_slack << ", Q pin: " << q_slack << endl;
            }
            pp->preference_list.push_back(pair<int, double>(idx, total_slack));
        }
        pp->preference_list.sort(preference_cmp);
        double pos_slack_cnt = 0;
        for(auto pair_: pp->preference_list){
            if(pair_.second > 0) pos_slack_cnt = pos_slack_cnt + 1;
            else break;
        }
        for(auto pair_: pp->preference_list){
            if(pair_.second > 0) pair_.second = pair_.second/pos_slack_cnt;
            else break;
        }
    }
    // intitial preference list: end

    // Stable matching (Gale and shapley): begin (In this stable matching, pin_pair is women, port is men.)
    
    // initial port_pairs (mans): begin
    vector<port_pair> port_pairs;
    port_pairs.resize(bit_num);
    for(auto& pp: port_pairs){
        pp.like_most_pin_pair = NULL;
        pp.choices_list.clear();
    }
    // initial port_pairs (mans): end

    // matching: begin
    // if(print) cout << "matching" << endl;
    int remain_women_num = pin_pair_list.size();
    while(remain_women_num>0){
        // women propose: begin
        // if(print) cout << "proposing, women num = " << remain_women_num << endl;
        for(auto itr=pin_pair_list.begin(); itr!=pin_pair_list.end(); itr++){
            auto& most_like_port = (*itr)->preference_list.front();
            port_pairs[most_like_port.first].choices_list.push_back(pair<double, list<pin_pair*>::iterator>(most_like_port.second, itr));
            (*itr)->preference_list.pop_front();
        }
        // women propose: end

        // men accept or reject: begin
        // if(print) cout << "accpet or reject" << endl;
        for(auto& man: port_pairs){
            bool have_choice = false;
            double max_slack = numeric_limits<double>::lowest();
            list<pin_pair*>::iterator like_most_itr;

            for(auto& ch: man.choices_list){
                if(max_slack < ch.first){
                    max_slack = ch.first;
                    like_most_itr = ch.second;
                    have_choice = true;
                }
            }
            // if(print) cout << "have choice: " << have_choice << endl;
            if(have_choice == false) continue;
            if(man.like_most_pin_pair == NULL){
                man.slack = max_slack;
                man.like_most_pin_pair = *like_most_itr;
                pin_pair_list.erase(like_most_itr);
                remain_women_num--;
            }
            else{
                if(man.slack < max_slack){
                    pin_pair_list.push_back(man.like_most_pin_pair);
                    man.slack = max_slack;
                    man.like_most_pin_pair = *like_most_itr;
                    pin_pair_list.erase(like_most_itr);
                }
            }
            man.choices_list.clear();
        }
        // men accept or reject: end
    }
    // matching: end

    // Stable matching (Gale and shapley): end

    // calculate final best total slack: begin
    double total_slack = 0;
    for(auto& pp: port_pairs){
        optseq_D.push_back(pp.like_most_pin_pair->dpin);
        optseq_Q.push_back(pp.like_most_pin_pair->qpin);
        if(pp.slack < 0){
            total_slack = total_slack + pp.slack;
        }
    } 
    // calculate final best total slack: end

    return total_slack;
}

ffi::ffi(string name, double coox, double cooy){
    this->size = 0;
    this->name = name;
    this->coox = coox;
    this->cooy = cooy;
    d_pins.clear();
    q_pins.clear();
    to_list = NULL;
    index_to_placement_row = -1;
    cost = numeric_limits<double>::max();
    members.clear();
    no_neighbor = true;
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
    this->size = bit_num;
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

void ffi::update_pin_loc(){
    for(int i=0; i<d_pins.size(); i++){
        d_pins[i]->new_coox = coox + type->d_pins[i].x_plus;
        d_pins[i]->new_cooy = cooy + type->d_pins[i].y_plus;
        q_pins[i]->new_coox = coox + type->q_pins[i].x_plus;
        q_pins[i]->new_cooy = cooy + type->q_pins[i].y_plus;
    }
}

void ffi::update_coor(){
    int bit = d_pins.size(); // this bit is the effective bit number, not the same as the "type->bit_num";
    double mx = 0;
    double my = 0;
    double rx = 0; // relative centroid
    double ry = 0; // relative centroid

    // for(int i=0; i<bit; i++){
    //     mx = mx + d_pins[i]->coox + q_pins[i]->coox;
    //     my = my + d_pins[i]->cooy + q_pins[i]->cooy;
    // }
    double num = 0;
    for(auto p: d_pins){
        mx = mx + p->coox;
        my = my + p->cooy;
        num = num + 1;
    }
    for(auto p: q_pins){
        mx = mx + p->coox;
        my = my + p->cooy;
        num = num + 1;
    }

    mx = mx/num;
    cen_x = mx;
    my = my/num;
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
        d_pins[i]->to_new_ff = this;
        d_pins[i]->new_coox = coox + type->d_pins[i].x_plus;
        d_pins[i]->new_cooy = cooy + type->d_pins[i].y_plus;
        
        q_pins[i]->new_name = type->q_pins[i].name;
        q_pins[i]->to_new_ff = this;
        q_pins[i]->new_coox = coox + type->q_pins[i].x_plus;
        q_pins[i]->new_cooy = cooy + type->q_pins[i].y_plus;
    }
    return;
}

void ffi::Set_PseudoBlock_Size(double expand_rate){
    double size = (type->size_x > type->size_y) ? type->size_y : type->size_x;
    double expand_size = size*(expand_rate/100);
    pseudo_block.xmin = coox - expand_size;
    pseudo_block.xmax = coox + type->size_x + expand_size;
    pseudo_block.ymin = cooy - expand_size;
    pseudo_block.ymax = cooy + type->size_y + expand_size;
}


void ffi::CalculateCost(double alpha, double beta, double gamma, double displacement_delay){
    cost = alpha*this->get_timing_cost(coox, cooy, displacement_delay) + beta*type->gate_power + gamma*type->area;
}

double ffi::get_timing_cost(double x, double y, double displacement_delay){
    double cost = 0;
    double slack;
    double rel_x = x - coox;
    double rel_y = y - cooy;



    for(auto& p: d_pins){
        double pin_x = p->new_coox + rel_x;
        double pin_y = p->new_cooy + rel_y;
        slack = p->CalTns(pin_x, pin_y, true, type, displacement_delay);
        if(slack < 0) cost = cost - slack;
    }

    for(auto& p: q_pins){
        double pin_x = p->new_coox + rel_x;
        double pin_y = p->new_cooy + rel_y;
        slack = p->CalTns(pin_x, pin_y, false, type, displacement_delay);
        if(slack < 0) cost = cost - slack;
    }

    return cost;
}


gatei::gatei(string name, double coox, double cooy){
    this->name = name;
    this->coox = coox;
    this->cooy = cooy;
    v = false;
    critical_slack = numeric_limits<double>::max();
    min_cs = numeric_limits<double>::max();
    consume_time = numeric_limits<double>::lowest();
    is_tracking = false;
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

double pin::CalTns(double test_coox, double tes_cooy, bool is_D, ffcell* new_type, double coeff){
    double slack = 0;

    if(to_net==NULL) return 0;

    if(is_D){
        pin* sp = to_net->ipins.front();

        if(sp->pin_type == 'f'){
            double anchor_x = (coox + sp->coox)/2;
            double anchor_y = (cooy + sp->cooy)/2;
            double ori_hpwl = abs(coox - anchor_x) + abs(cooy - anchor_y);
            double new_hpwl = abs(test_coox - anchor_x) + abs(tes_cooy - anchor_y);
            slack = dspd_slk - (new_hpwl - ori_hpwl)*coeff;
        }
        else{
            double ori_hpwl = abs(coox - sp->coox) + abs(cooy - sp->cooy);
            double new_hpwl = abs(test_coox - sp->coox) + abs(tes_cooy - sp->cooy);
            slack = dspd_slk - (new_hpwl - ori_hpwl)*coeff;
        }
    }
    else{
        for(auto tp: to_net->opins){ // tp: target pin
            double temp_slack;
            if(tp->pin_type == 'f'){
                double anchor_x = (coox + tp->coox)/2;
                double anchor_y = (cooy + tp->cooy)/2;
                double ori_hpwl = abs(coox - anchor_x) + abs(cooy - anchor_y);
                double new_hpwl = abs(test_coox - anchor_x) + abs(tes_cooy - anchor_y);
                temp_slack = dspd_slk - (new_hpwl - ori_hpwl)*coeff - (new_type->Qpin_delay - to_ff->type->Qpin_delay);
                if(temp_slack >= dspd_slk) temp_slack = 0;
            }
            else if(tp->pin_type == 'g'){
                double ori_hpwl = abs(coox - tp->coox) + abs(cooy - tp->cooy);
                double new_hpwl = abs(test_coox - tp->coox) + abs(tes_cooy - tp->cooy);
                if(tp->to_gate->get_critical_slack() == numeric_limits<double>::max()){
                    temp_slack = 0;
                }
                else{
                    temp_slack = tp->to_gate->get_critical_slack() - (new_hpwl - ori_hpwl)*coeff - (new_type->Qpin_delay - to_ff->type->Qpin_delay);
                    if(temp_slack >= tp->to_gate->get_critical_slack()){
                        temp_slack = 0;
                    }
                }
            }
            else if(tp->pin_type == 'd'){
                temp_slack = 0;
            }

            if(temp_slack < 0) slack = slack + temp_slack;
        }
    }
    return (slack >= 0)? 1 : slack;
}

se::se(int type, double coor, ffi* to_ff){
    this->type = type;
    this->coor = coor;
    this->to_ff = to_ff;
}