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


// Class lib
lib::lib(){
    max_ff_size = 0;
    min_ff_size = numeric_limits<int>::max();
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
        if(((ffcell*)new_cell)->bit_num < min_ff_size) {
            min_ff_size = ((ffcell*)new_cell)->bit_num; 
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

}

