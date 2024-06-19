#include "netlist.h"

netlist::netlist(){
    this->net_num = 0;
    DI_num = 0;
    new_net = NULL;
    nets.clear();
    Diopins_umap.clear();
}

void netlist::set_net_num(int n){
    net_num = n;
    nets.reserve(net_num);
}

void netlist::add_net(string net_name){
    // cout << "add NET:" << net_name << endl;
    this->new_net = new net(net_name);
    nets.push_back(new_net);
    return;
}

void netlist::add_pin(string str, inst& INST){
    // cout << "\tadd PIN: " << str<< endl;
    int pin_index;
    string inst_name;
    string pin_name;

    bool is_cell_pin = false;


    size_t found =  str.find("/");
    if(found != string::npos){
        is_cell_pin = true;
    }


    if(is_cell_pin){
        ffi* ffi_ptr;
        gatei* gi_ptr;
        inst_name = str.substr(0, found);
        str.erase(0, found+1);
        pin_name = str;

        auto it1 = INST.type_umap.find(inst_name);

        if(it1 == INST.type_umap.end()){
            cout << "Error: Can not find instance: " << inst_name << endl;
            return;
        }
        if(it1->second == 0){ // instance is FF
            
            auto it2 = INST.ff_umap.find(inst_name);
            if(it2 == INST.ff_umap.end()){
                cout << "Error: Can not find FlipFlop: " << inst_name << endl;
                return;
            }

            ffi_ptr = it2->second;
            if(pin_name.find("D") != string::npos){
                // find the corresponding pin idx
                // str.erase(0, 1);
                // if(str.size() == 0){
                //     pin_index = 0;
                // }
                // else{
                //     pin_index = stoi(str);
                // }
                pin_index = ffi_ptr->type->get_PinIDX(pin_name);

                // assign corresponding net pointer to the pin
                ffi_ptr->d_pins[pin_index]->to_net = new_net;

                // D pin is an output net for current net
                // Add this pin to the "opins" vector in current net
                new_net->opins.push_back(ffi_ptr->d_pins[pin_index]);
            }
            else if(pin_name.find("Q") != string::npos){
                // find the corresponding pin idx
                // str.erase(0, 1);
                // if(str.size() == 0){
                //     pin_index = 0;
                // }
                // else{
                //     pin_index = stoi(str);
                // }
                pin_index = ffi_ptr->type->get_PinIDX(pin_name);
                // assign corresponding net pointer to the pin
                ffi_ptr->q_pins[pin_index]->to_net = new_net;

                // Q pin is an input net for current net
                // Add this pin to the "ipins" vector in current net
                new_net->ipins.push_back(ffi_ptr->q_pins[pin_index]);
            }
            else{ // is clk pin
                ffi_ptr->clk_pin->to_net = new_net;
                new_net->opins.push_back(ffi_ptr->clk_pin);
            }
        }
        else{ // instance is Gate
            
            auto it3 = INST.gate_umap.find(inst_name);
            if(it3 == INST.gate_umap.end()){
                cout << "Error: Can not find Gate: " << inst_name << endl;
                return;
            }
            gi_ptr = it3->second;
            // cout << gi_ptr->type->name << endl;
            // cout << pin_name << endl;
            if(pin_name.find("IN") != string::npos){
                // find the corresponding pin idx
                // str.erase(0, 2);
                // if(str.size() == 0){
                //     pin_index = 0;
                // }
                // else{
                //     pin_index = stoi(str);
                // }
                pin_index = gi_ptr->type->get_PinIDX(pin_name);
                // assign corresponding net pointer to the pin
                gi_ptr->ipins[pin_index]->to_net = new_net;

                // IN pin is an output net for current net
                // Add this pin to the "opins" vector in current net
                new_net->opins.push_back(gi_ptr->ipins[pin_index]);
            }
            else{ // is "OUT" pin
                // find the corresponding pin idx
                // str.erase(0, 3);
                // if(str.size() == 0){
                //     pin_index = 0;
                // }
                // else{
                //     pin_index = stoi(str);
                // }
                pin_index = gi_ptr->type->get_PinIDX(pin_name);
                // assign corresponding net pointer to the pin
                gi_ptr->opins[pin_index]->to_net = new_net;

                // OUT pin is an input net for current net
                // Add this pin to the "ipins" vector in current net
                new_net->ipins.push_back(gi_ptr->opins[pin_index]);
            }
        }
    }
    else{ // This pin is belongs to Die IO Pins
        pin* pin_ptr;
        inst_name = str;
        // cout << inst_name << endl;
        auto it4 = Diopins_umap.find(inst_name);

        if(it4 == Diopins_umap.end()){
            cout << "Error: Can not find instance: " << inst_name << endl;
            return;
        }
        pin_ptr = it4->second;
        pin_ptr->to_net = new_net;
        // cout << it4->second->name << endl;
        if(pin_ptr->io == 0){
            new_net->ipins.push_back(pin_ptr);
        }
        else{
            new_net->opins.push_back(pin_ptr);
        }
    }

    return;
}

void netlist::add_Di_pin(string pin_name, double coox, double cooy){
    pin* new_pin = new pin;
    DI_num++;
    new_pin->name = pin_name;
    new_pin->coox = coox;
    new_pin->cooy = cooy;
    new_pin->pin_type = 'd';
    new_pin->io = 0;
    Diopins_umap.insert(pair<string, pin*>(pin_name, new_pin));
    return;
}

void netlist::add_Do_pin(string pin_name, double coox, double cooy){
    pin* new_pin = new pin;

    new_pin->name = pin_name;
    new_pin->coox = coox;
    new_pin->cooy = cooy;
    new_pin->pin_type = 'd';
    new_pin->io = 1;
    Diopins_umap.insert(pair<string, pin*>(pin_name, new_pin));
    return;
}


