#include "lib_cell.h"

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
}

void gcell::add_pin(string name, double coox, double cooy){
    pin_relpos new_pin;

    new_pin.name = name;
    new_pin.x_plus = coox;
    new_pin.y_plus = cooy;

    if(name.find("IN") != string::npos){
        in_pins.push_back(new_pin);
    }
    else{
        out_pins.push_back(new_pin);
    }
    return;
}


// Class ffcell (Derive class) 
ffcell::ffcell(string name, string type, int bit_num, double size_x, double size_y, int pin_num){
    this->name = name;
    this->type = type;
    this->bit_num = bit_num;
    this->size_x = size_x;
    this->size_y = size_y;
    this->pin_num = pin_num;   
    this->d_pins.clear(); 
    this->q_pins.clear(); 
    this->d_pins.reserve(int(pin_num/2)); 
    this->q_pins.reserve(int(pin_num/2)); 
}

void ffcell::add_pin(string name, double coox, double cooy){
    pin_relpos new_pin;

    new_pin.name = name;
    new_pin.x_plus = coox;
    new_pin.y_plus = cooy;

    if(name.find("Q") != string::npos){
        q_pins.push_back(new_pin);
    }
    else{
        d_pins.push_back(new_pin);
    }
    return;
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

