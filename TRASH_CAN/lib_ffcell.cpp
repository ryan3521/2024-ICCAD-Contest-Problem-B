#include "lib_ffcell.h"

// Class ff_cell
ffcell::ffcell(string name, int bit_num, double size_x, double size_y, int pin_num){
    this->name = name;
    this->bit_num = bit_num;
    this->size_x = size_x;
    this->size_y = size_y;
    this->pin_num = pin_num;   
    this->d_pins.clear(); 
    this->q_pins.clear(); 
    this->d_pins.reserve(int(pin_num/2)); 
    this->q_pins.reserve(int(pin_num/2)); 
}

void ffcell::add_Dpin(string name, double coox, double cooy){
    pin new_pin;
    
    new_pin.name = name;
    new_pin.coox = coox;
    new_pin.cooy = cooy;

    d_pins.push_back(new_pin);
    return;
}

void ffcell::add_Qpin(string name, double coox, double cooy){
    pin new_pin;
    
    new_pin.name = name;
    new_pin.coox = coox;
    new_pin.cooy = cooy;

    q_pins.push_back(new_pin);
    return;
}

void ffcell::set_Qpin_delay(double Qpin_delay){
    this->Qpin_delay = Qpin_delay;
    return;
}

void ffcell::set_gate_power(double gate_power){
    this->gate_power = gate_power;
    return;
}

string ffcell::get_name(){
    return name;
}

int ffcell::get_bit_num(){
    return bit_num;
}

pair<double, double> ffcell::get_size(){
    pair<double, double> size_pair(size_x, size_y);
    return size_pair;
}

double ffcell::get_Qpin_delay(){
    return Qpin_delay;
}

double ffcell::get_gate_power(){
    return gate_power;
}

void ff_cell::set_CLKpin(string name, double coox, double cooy){
    clk_pin.name = name;
    clk_pin.coox = coox;
    clk_pin.cooy = cooy;
}