#include "lib.h"

// Class cell (Base class)

string cell::get_name(){return name;}

string cell::get_typename(){return type;}

pair<double, double> cell::get_size(){
    pair<double, double> size_pair(size_x, size_y);
    return size_pair;
}

//Class gcell　(Derive class)
gcell::gcell(string name, string type, double size_x, double size_y, int pin_num){
    this->name = name;
    this->type = type;
    this->size_x = size_x;  
    this->size_y = size_y;
    this->area = size_x * size_y;
    this->pin_num = pin_num;
    in_pins.clear();  
    out_pins.clear();  
    in_pins.reserve(int(pin_num/2));
    out_pins.reserve(int(pin_num/2));
    idx_umap.clear();
    in_cnt = 0;
    out_cnt = 0;
}

void gcell::add_pin(string name, double coox, double cooy){
    pin_relpos new_pin;

    new_pin.name = name;
    new_pin.x_plus = coox;
    new_pin.y_plus = cooy;

    if(name.find("IN") != string::npos){
        in_pins.push_back(new_pin);
        idx_umap.insert(pair<string, int>(name, in_cnt));
        in_cnt++;
    }
    else{
        out_pins.push_back(new_pin);
        idx_umap.insert(pair<string, int>(name, out_cnt));
        out_cnt++;
    }
    return;
}

int gcell::get_PinIDX(string name){
    auto it = idx_umap.find(name);
    if(it == idx_umap.end()) return 0;
    else return it->second;
}


// Class ffcell (Derive class) 
ffcell::ffcell(string name, string type, int bit_num, double size_x, double size_y, int pin_num){
    this->name = name;
    this->type = type;
    this->bit_num = bit_num;
    this->size_x = size_x;
    this->size_y = size_y;
    this->area   = size_x*size_y;
    this->pin_num = pin_num;   
    this->d_pins.clear(); 
    this->q_pins.clear(); 
    this->d_pins.reserve(int(pin_num/2)); 
    this->q_pins.reserve(int(pin_num/2)); 
    d_cnt = 0;
    q_cnt = 0;
    idx_umap.clear();
}

void ffcell::add_pin(string name, double coox, double cooy){
    pin_relpos new_pin;

    new_pin.name = name;
    new_pin.x_plus = coox;
    new_pin.y_plus = cooy;

    if(name.find("Q") != string::npos){
        q_pins.push_back(new_pin);
        idx_umap.insert(pair<string, int>(name, q_cnt));
        q_cnt++;
    }
    else{
        d_pins.push_back(new_pin);
        idx_umap.insert(pair<string, int>(name, d_cnt));
        d_cnt++;
    }
    return;
}

int ffcell::get_PinIDX(string name){
    auto it = idx_umap.find(name);
    if(it == idx_umap.end()) return 0;
    else return it->second;
}


void ffcell::set_CLKpin(string name, double coox, double cooy){
    clk_pin.name = name;
    clk_pin.x_plus = coox;
    clk_pin.y_plus = cooy;
}

void ffcell::set_Qpin_delay(double Qpin_delay){
    this->Qpin_delay = Qpin_delay;
    return;
}

void ffcell::set_gate_power(double gate_power){
    this->gate_power = gate_power;
    return;
}

int ffcell::get_bit_num(){return bit_num;}

double ffcell::get_Qpin_delay(){return Qpin_delay;}

double ffcell::get_gate_power(){return gate_power;}


void ffcell::calculate_min_FSR(){
    double xmin = numeric_limits<double>::max();
    double xmax = numeric_limits<double>::min();
    double ymin = numeric_limits<double>::max();
    double ymax = numeric_limits<double>::min();

    for(auto& pr: d_pins){
        if(pr.x_plus > xmax) xmax = pr.x_plus;
        if(pr.x_plus < xmin) xmin = pr.x_plus;
        if(pr.y_plus > ymax) ymax = pr.y_plus;
        if(pr.y_plus < ymin) ymin = pr.y_plus;
    }
    for(auto& pr: q_pins){
        if(pr.x_plus > xmax) xmax = pr.x_plus;
        if(pr.x_plus < xmin) xmin = pr.x_plus;
        if(pr.y_plus > ymax) ymax = pr.y_plus;
        if(pr.y_plus < ymin) ymin = pr.y_plus;
    }

    double up_y, bottom_y;
    double right_x, left_x;
    
    up_y     = ymax - xmin;
    bottom_y = ymin - xmax;
    right_x  = ymax - xmax;
    left_x   = ymin - xmin;

    fsr_min_h = up_y - bottom_y;
    fsr_min_w = right_x - left_x;
    return;
}


// Class lib
lib::lib(){
    max_ff_size = 0;
    fftable_cost.clear();
    fftable_c2q.clear();
    cell_umap.clear();
}

void lib::add_cell(cell* new_cell){
    cell_umap.insert(pair<string, cell*>(new_cell->get_name(), new_cell));
    if(new_cell->type == "FlipFlop"){
        if(((ffcell*)new_cell)->bit_num > max_ff_size) {
            max_ff_size = ((ffcell*)new_cell)->bit_num; 
        }
    } 
    return;
}


void lib::set_Qpin_delay(string name, double delay){
    auto it = cell_umap.find(name);

    if(it != cell_umap.end()){
        ((ffcell*)(it->second))->set_Qpin_delay(delay);
    }
    
    return;
}

void lib::set_gate_power(string name, double power){
    auto it = cell_umap.find(name);

    if(it != cell_umap.end()){
        ((ffcell*)(it->second))->set_gate_power(power);
    }

    return;
}

cell* lib::get_cell(string name){
    auto it = cell_umap.find(name);

    if(it != cell_umap.end()){
        return it->second;
    }

    return NULL;
}

void lib::Print_member(){
    int i=0;

    for(auto& c: cell_umap){
        i++;

        cout << endl;
        cout << "Name: " << c.first << " -> Type: " << c.second->get_typename() << endl;
        if(c.second->get_typename() == "FlipFlop"){
            cout << "   Bit Size: " << ((ffcell*)(c.second))->get_bit_num() << endl;
            cout << "   Width: " << c.second->get_size().first << endl;  
            cout << "   Height: "  << c.second->get_size().second << endl;
            cout << "   Qpin Delay: " << ((ffcell*)(c.second))->get_Qpin_delay() << endl;
            cout << "   Gate Power: " << ((ffcell*)(c.second))->get_gate_power() << endl;
        }
        else{
            cout << "   Width: " << c.second->get_size().first << endl;  
            cout << "   Height: "  << c.second->get_size().second << endl;
        }
    }
}

bool lib::cmp_clk2Q(ffcell* a, ffcell* b){
    return a->Qpin_delay < b->Qpin_delay;
}

bool lib::cmp_APcost(ffcell* a, ffcell* b){
    return a->cost_per_bit < b->cost_per_bit;
}

bool lib::cmp_area(ffcell* a, ffcell* b){
    return a->area < b->area;
}

void lib::construct_fftable(dieInfo& DIE){
    ffcell* ff;
    vector<double> cost_table;

    // ################## Initial ##################
    ffmapping_list.clear();
    ffmapping_list.resize(max_ff_size+1, 0);
    mincost_list.resize(max_ff_size+1, -1);
    fftable_cost.resize(max_ff_size+1);
    for(int i=0; i<fftable_cost.size(); i++){
        fftable_cost[i].clear();
    }
    fftable_c2q.resize(max_ff_size+1);
    for(int i=0; i<fftable_c2q.size(); i++){
        fftable_c2q[i].clear();
    }
    fftable_area.resize(max_ff_size+1);
    for(int i=0; i<fftable_area.size(); i++){
        fftable_area[i].clear();
    }
    
    // ==============================================


    // seperate the "FlipFlop" from "cell_umap" into:
    // 1. "fftable_cost"
    // 2. "fftable_c2q"
    // 3. "fftable_area"
    for(auto& c: cell_umap){
        if(c.second->type == "FlipFlop"){
            ff = ((ffcell*)(c.second));
            ff->cost_per_bit = ((DIE.Gamma)*(ff->area) + (DIE.Beta)*(ff->gate_power))/(ff->bit_num);
            fftable_cost[ff->bit_num].push_back(ff);
            fftable_c2q[ff->bit_num].push_back(ff);
            fftable_area[ff->bit_num].push_back(ff);
        }
    }

    // Sort "fftable_cost".
    for(int i=fftable_cost.size()-1; i>0; i--){
        if(fftable_cost[i].size() != 0){
            fftable_cost[i].sort(cmp_APcost);
        }
    }

    // Sort "fftable_c2q".
    for(int i=fftable_c2q.size()-1; i>0; i--){
        if(fftable_c2q[i].size() != 0){
            fftable_c2q[i].sort(cmp_clk2Q);
        }
    }

    // Sort "fftable_area".
    for(int i=fftable_area.size()-1; i>0; i--){
        if(fftable_area[i].size() != 0){
            fftable_area[i].sort(cmp_area);
        }
    }

    // Construct mapping list
    for(int i=fftable_cost.size()-1; i>0; i--){
        if(fftable_cost[i].size() != 0){
            ffmapping_list[i] = -1;
        }
        else{
            if(ffmapping_list[i+1] == -1){
                ffmapping_list[i] = i+1;
            }
            else{
                ffmapping_list[i] = ffmapping_list[i+1];
            }
        }
    }
    // for(int i=0; i<ffmapping_list.size(); i++ ){
    //     cout << ffmapping_list[i] << endl;
    // }

    // calculate the best configuration for each bit
    cost_table.clear();
    cost_table.resize(max_ff_size+1, -1);
    best_choice.resize(max_ff_size+1);
    for(int i=0; i<best_choice.size(); i++){
        best_choice[i].first  = -1;
        best_choice[i].second = -1;
    }
    
    double cost; // useless
    for(int b=1; b<=max_ff_size; b++){
        cost = find_min_cost(b, DIE);
    }

    // back trace "best choice" vector and construct the "opt_fftable"
    // for(int i=0; i<best_choice.size(); i++){
    //     cout << "B" << i << endl;
    //     cout << best_choice[i].first  << " , " << best_choice[i].second << endl;

    // }
    opt_fftable.resize(max_ff_size+1);
    for(int i=0; i<=max_ff_size; i++){
        opt_fftable[i].clear();
    }

    for(int b=1; b<=max_ff_size; b++){
        construct_optlist(b, opt_fftable[b]);
    }

    mbff_cost.resize(opt_fftable.size());
    for(int i=1; i<opt_fftable.size(); i++){
        int cost_sum = 0;
        for(auto& f: opt_fftable[i]){
            cost_sum = cost_sum + f->area*DIE.Gamma + f->gate_power*DIE.Beta; 
        }
        mbff_cost[i] = cost_sum;
    }
}

void lib::construct_optlist(int bit, list<ffcell*>& opt_list){
    if(opt_fftable[bit].size() > 0){
        for(auto ff: opt_fftable[bit]){
            opt_list.push_back(ff);
        }
        return;
    }
    
    if(best_choice[bit].second == -1){
        opt_list.push_back(fftable_cost[best_choice[bit].first].front());
        
        return;
    }
    else{
        construct_optlist(best_choice[bit].first,  opt_list);
        construct_optlist(best_choice[bit].second, opt_list);
        return;
    }
}

double lib::find_min_cost(int bit, dieInfo& DIE){
    if(bit == 0) return 0;
    // The value of max bit will always be the true MBFF size, it won't be the size of pseudo MBFF.
    int max_bit = (ffmapping_list[bit]==-1) ? bit : ffmapping_list[bit];
    // cost = (BETA*f.power + GAMMA*f.area)/bit
    double min_cost; 
    double cur_cost;
    ffcell* f;

    if(mincost_list[bit] != -1){ 
        return mincost_list[bit];
    }

    // mincost initialize: set mincost the cost of max_bit
    // Note: the max_bit ff must be a entity mbff
    f = fftable_cost[max_bit].front();
    min_cost = (f->cost_per_bit)*(f->bit_num)/(double)bit;
    best_choice[bit].first = max_bit;


    // Do recusive search to find the mincost of this bit
    for(int i=1; i<=(bit-i); i++){
        cur_cost = 0;
        cur_cost = (find_min_cost(i, DIE)*i + find_min_cost(bit-i, DIE)*(bit-i))/bit;

        if(cur_cost < min_cost){
            min_cost = cur_cost;
            best_choice[bit].first  = i;
            best_choice[bit].second = bit - i;
        }
    }

    mincost_list[bit] = min_cost;
    return min_cost;
}

void lib::check_opt_result(){
    int bit_sum;
    for(int i=1; i<=max_ff_size; i++){
        bit_sum = 0;
        for(auto ff: opt_fftable[i]){
            bit_sum = bit_sum + ff->bit_num;
        }
        if(bit_sum != i){
            cout << "# OPT Flip Flop Table Wrong !!!" << endl << endl;
            return;
        }
    }
}