#include "lib_gcell.h"

gcell::gcell(string name, double size_x, double size_y, int pin_num){
    this->name = name;
    this->size_x = size_x;  
    this->size_y = size_y;
    this->pin_num = pin_num;
    in_pins.clear();  
    out_pins.clear();  
    in_pins.reserve(int(pin_num/2));
    out_pins.reserve(int(pin_num/2));
}

void gcell::add_pin(string name, double coox, double cooy){
    pin new_pin;

    new_pin.name = name;
    new_pin.coox = coox;
    new_pin.cooy = cooy;

    if(name.find("IN") != string::npos){
        in_pins.push_back(new_pin);
    }
    else{
        out_pins.push_back(new_pin);
    }
    return;
}