costeva::costeva(dieInfo* DIE, lib* LIB, inst* INST){
    this->DIE = DIE;
    this->LIB = LIB;
    this->ISNT = INST;
}

double costeva::evaluate(list<ffi*>* ff_list){
    this->ffs = ff_list;
    
    // *********************************************************** //
    // Cost = alpha*TNS + beta*Power + gamma*Area + lambda*Density //
    // *********************************************************** //

    double TNS     = calTns();
    double Power   = calPower();
    double Area    = calArea();
    double Density = calDensity();

    double cost = DIE->Alpha*TNS + DIE->Beta*Power + DIE->Gamma*Area + DIE->Lambda*Density;

    return cost;
}

double costeva::calArea(){
    double area = 0;

    for(auto f: *ff_list){
        area = area + f->type->area;
    }

    return area;
}

double costeva::calPower(){
    double power = 0;

    for(auto f: *ff_list){
        power = power + f->type->gate_power;
    }

    return power;
}

double costeva::calTns(){
    
}