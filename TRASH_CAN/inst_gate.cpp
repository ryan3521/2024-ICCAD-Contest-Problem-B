

gatei::gatei(string name, double coox, double cooy){
    this->name = name;
    this->coox = coox;
    this->cooy = cooy;
    visit = false;
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
    visit = true;
    this->critical_slack = critical_slack;
}

bool gatei::is_visited(){return visit;}

double get_critical_slack(){return critical_slack;}