


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
    new_pin->to_ff = this;
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
    my = my/(double)(2*bit);

    for(int i=0; i<bit; i++){
        rx = rx + type->d_pins[i]->x_plus + type->q_pins[i]->x_plus;
        ry = ry + type->d_pins[i]->y_plus + type->q_pins[i]->y_plus;
    }
    rx = rx/(double)(2*bit);
    ry = ry/(double)(2*bit);
    
    coox = ((mx - rx) < 0) ? 0 : (mx - rx);
    cooy = ((my - ry) < 0) ? 0 : (my - ry);

    for(int i=0; i<bit; i++){
        d_pins[i]->new_coox = coox + type->d_pins[i]->x_plus;
        d_pins[i]->new_cooy = cooy + type->d_pins[i]->y_plus;
        
        q_pins[i]->new_coox = coox + type->q_pins[i]->x_plus;
        q_pins[i]->new_cooy = cooy + type->q_pins[i]->y_plus;
    }
    return;
}

void reg::update_cen(){
    cen_x = (dpin->coox + qpin->coox)/2;
    cen_y = (dpin->cooy + qpin->cooy)/2;
}