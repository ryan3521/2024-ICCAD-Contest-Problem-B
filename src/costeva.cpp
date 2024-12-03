#include "costeva.h"

bin::bin(double width, double height, double util, double coox, double cooy){
    this->xmax = coox + width;
    this->ymax = cooy + height;
    this->util = util;
    this->coox = coox;
    this->cooy = cooy;
    this->bin_area = width*height;
    this->acc_area = 0;
    this->up_bin = NULL;
    this->next_bin = NULL;
    return;
}

bool bin::add_cell(double coox, double cooy, double sizex, double sizey){
    bool success;
    double cut_x, cut_y;
    double remain_x, remain_y;

    if(coox+sizex <= xmax){
        cut_x = sizex;
        remain_x = 0;
    }
    else{
        cut_x = xmax - coox;
        remain_x = (coox+sizex) - xmax;
    }

    if(remain_x > 0){
        if(next_bin == NULL) return false;
        else success = next_bin->add_cell(xmax, cooy, remain_x, sizey);
        if(success == false) return false;
    }

    if(cooy+sizey <= ymax){
        cut_y = sizey;
        remain_y = 0;
    }
    else{
        cut_y = ymax - cooy;
        remain_y = (cooy+sizey) - ymax;
    }

    if(remain_y > 0){
        if(up_bin == NULL) return false;
        else success = up_bin->add_cell(coox, ymax, remain_x, remain_y);
        if(success == false) return false;
    }

    acc_area = acc_area + cut_x*cut_y;
    return true;
}

bool bin::check_violation(){
    if(acc_area/bin_area <= util/100) return false;
    else return true;
}

costeva::costeva(dieInfo* DIE, lib* LIB, inst* INST, string testcase_filename){
    this->DIE = DIE;
    this->LIB = LIB;
    this->INST = INST;
    filename = "report.txt";
    fout.open(filename, ios::out);

    fout << "Testcase: " << testcase_filename << endl;
    fout << endl;

}

costeva::~costeva(){
    fout.close();
}

void costeva::InitialCost(){
    // *********************************************************** //
    // Cost = alpha*TNS + beta*Power + gamma*Area + lambda*Density //
    // *********************************************************** //
    int bit_num = 0;
    int ff_num = INST->ff_umap.size();
    int neg_reg_cnt = 0;
    double area = 0;
    double area_cost = 0;
    double power = 0;
    double power_cost = 0;
    double tns = 0;
    double tns_cost = 0;
    double wns = 0;
    int violated_bins = 0;
    int density_cost = 0;
    double total_cost;

    // initial ff list: begin
    list<ffi*> ff_list;
    for(auto it: INST->ff_umap){
        ffi* f = it.second;
        ff_list.push_back(f);
        bit_num = bit_num + f->d_pins.size();
    }
    ffs = &ff_list;
    // initial ff list: end

    // calculate cost: begin
    area  = calArea();
    area_cost = area*DIE->Gamma;
    power = calPower();
    power_cost = power*DIE->Beta;
    // timing: begin
    for(auto f: *ffs){
        for(auto p: f->d_pins){
            if(p->slack < 0){
                tns = tns + p->slack;
                if(p->slack < wns){
                    wns = p->slack;
                }
                neg_reg_cnt++;
            }
        }
    } 
    tns_cost = -tns*DIE->Alpha;
    // timing: end
    violated_bins = calDensity();
    density_cost = violated_bins*DIE->Lambda;
    total_cost = tns_cost + power_cost + area_cost + density_cost;
    // calculate cost: end

    cout << endl;
    cout << "+================================================================================+" << endl;
    cout << "|                                Initial   Report                                |" << endl;
    cout << "+===========+========================+===========================+===============+" << endl;
    cout << "|           |         Value          |            Cost           |   Percentage  |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  TNS/WNS  |   " << right << setw(10) << tns << "/" << left << setw(10) << wns << "|      " << setw(21) << tns_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (tns_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Power    |       " << left << setw(17) << power << "|      " << setw(21) << power_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (power_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Area     |       " << left << setw(17) << area << "|      " << setw(21) << area_cost << "|    " << left << setw(6) << fixed << setprecision(2)<< (area_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Density  |            " << left << setw(12) << violated_bins << "|      " << setw(21) << density_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (density_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Total    |                 " << left << setw(35) << total_cost << "|      100%     |" << endl;
    cout << "+===========+====================================================================+" << endl;
    cout << "|           |                          Numbers                                   |" << endl;
    cout << "+-----------+--------------------------------------------------------------------|" << endl;
    cout << "|    FFs    |                            " << left << setw(8) << ff_num << "                                |" << endl;
    cout << "+ - - - - - + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|" << endl;
    cout << "|    Bits   |                            " << left << setw(8) << bit_num << "                                |" << endl;
    cout << "+================================================================================+" << endl;
    cout << endl;

    fout << endl;
    fout << "+================================================================================+" << endl;
    fout << "|                                Initial   Report                                |" << endl;
    fout << "+===========+========================+===========================+===============+" << endl;
    fout << "|           |         Value          |            Cost           |   Percentage  |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  TNS/WNS  |   " << right << setw(10) << tns << "/" << left << setw(10) << wns << "|      " << setw(21) << tns_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (tns_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Power    |       " << left << setw(17) << power << "|      " << setw(21) << power_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (power_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Area     |       " << left << setw(17) << area << "|      " << setw(21) << area_cost << "|    " << left << setw(6) << fixed << setprecision(2)<< (area_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Density  |            " << left << setw(12) << violated_bins << "|      " << setw(21) << density_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (density_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Total    |                 " << left << setw(35) << total_cost << "|      100%     |" << endl;
    fout << "+===========+====================================================================+" << endl;
    fout << "|           |                          Numbers                                   |" << endl;
    fout << "+-----------+--------------------------------------------------------------------|" << endl;
    fout << "|    FFs    |                            " << left << setw(8) << ff_num << "                                |" << endl;
    fout << "+ - - - - - + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|" << endl;
    fout << "|    Bits   |                            " << left << setw(8) << bit_num << "                                |" << endl;
    fout << "+================================================================================+" << endl;
    fout << endl;

    return;
}

void costeva::PrintParameter(){
    cout << endl;
    cout << "+-----------+------------------+" << endl;
    cout << "|   Die     |      Value       |" << endl;
    cout << "+-----------+------------------+" << endl;
    cout << "|   Width   |   " << left << setw(15) << DIE->die_width   << "|" << endl;
    cout << "|   Height  |   " << left << setw(15) << DIE->die_height  << "|" << endl;
    cout << "+-----------+------------------+" << endl;
    cout << endl;
    cout << "+----------+------------------+" << endl;
    cout << "|  Weight  |      Value       |" << endl;
    cout << "+----------+------------------+" << endl;
    cout << "|  Alpha   |   " << left << setw(15) << DIE->Alpha  << "|" << endl;
    cout << "|  Beta    |   " << left << setw(15) << DIE->Beta   << "|" << endl;
    cout << "|  Gamma   |   " << left << setw(15) << DIE->Gamma  << "|" << endl;
    cout << "|  Lammda  |   " << left << setw(15) << DIE->Lambda << "|" << endl;
    cout << "+----------+------------------+" << endl;
    cout << endl;
    cout << "+-----------+------------------+" << endl;
    cout << "|   Bin     |      Value       |" << endl;
    cout << "+-----------+------------------+" << endl;
    cout << "|   Width   |   " << left << setw(15) << DIE->bin_width  << "|" << endl;
    cout << "|   Height  |   " << left << setw(15) << DIE->bin_width  << "|" << endl;
    cout << "|   Util    |   " << left << setw(15) << DIE->bin_util   << "|" << endl;
    cout << "+-----------+------------------+" << endl;
    cout << endl;

    fout << "+-----------+------------------+" << endl;
    fout << "|   Die     |      Value       |" << endl;
    fout << "+-----------+------------------+" << endl;
    fout << "|   Width   |   " << left << setw(15) << DIE->die_width   << "|" << endl;
    fout << "|   Height  |   " << left << setw(15) << DIE->die_height  << "|" << endl;
    fout << "+-----------+------------------+" << endl;
    fout << endl;
    fout << "+----------+------------------+" << endl;
    fout << "|  Weight  |      Value       |" << endl;
    fout << "+----------+------------------+" << endl;
    fout << "|  Alpha   |   " << left << setw(15) << DIE->Alpha  << "|" << endl;
    fout << "|  Beta    |   " << left << setw(15) << DIE->Beta   << "|" << endl;
    fout << "|  Gamma   |   " << left << setw(15) << DIE->Gamma  << "|" << endl;
    fout << "|  Lammda  |   " << left << setw(15) << DIE->Lambda << "|" << endl;
    fout << "+----------+------------------+" << endl;
    fout << endl;
    fout << "+-----------+------------------+" << endl;
    fout << "|   Bin     |      Value       |" << endl;
    fout << "+-----------+------------------+" << endl;
    fout << "|   Width   |   " << left << setw(15) << DIE->bin_width  << "|" << endl;
    fout << "|   Height  |   " << left << setw(15) << DIE->bin_width  << "|" << endl;
    fout << "|   Util    |   " << left << setw(15) << DIE->bin_util   << "|" << endl;
    fout << "+-----------+------------------+" << endl;
    fout << endl;
}

void costeva::ResultCost(list<ffi*>* ff_list){
    this->ffs = ff_list;
    
    // *********************************************************** //
    // Cost = alpha*TNS + beta*Power + gamma*Area + lambda*Density //
    // *********************************************************** //
    int bit_num = 0;
    int ff_num = ffs->size();
    int neg_reg_cnt = 0;
    double area = 0;
    double area_cost = 0;
    double power = 0;
    double power_cost = 0;
    double tns = 0;
    double tns_cost = 0;
    double wns = 0;
    int violated_bins = 0;
    int density_cost = 0;
    vector<int> arr(LIB->max_ff_size+1, 0);
    double total_cost;

    for(auto f: *ffs){
        arr[f->d_pins.size()]++;
        bit_num = bit_num + f->d_pins.size();
    }

    // calculate cost: begin
    area  = calArea();
    area_cost = area*DIE->Gamma;
    power = calPower();
    power_cost = power*DIE->Beta;
    tns = calTns(&wns);
    tns_cost = -tns*DIE->Alpha;
    violated_bins = calDensity();
    density_cost = violated_bins*DIE->Lambda;
    total_cost = tns_cost + power_cost + area_cost + density_cost;
    // calculate cost: end

    cout << endl;
    cout << "+================================================================================+" << endl;
    cout << "|                                Result    Report                                |" << endl;
    cout << "+===========+========================+===========================+===============+" << endl;
    cout << "|           |         Value          |            Cost           |   Percentage  |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  TNS/WNS  |   " << right << setw(10) << tns << "/" << left << setw(10) << wns << "|      " << setw(21) << tns_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (tns_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Power    |       " << left << setw(17) << power << "|      " << setw(21) << power_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (power_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Area     |       " << left << setw(17) << area << "|      " << setw(21) << area_cost << "|    " << left << setw(6) << fixed << setprecision(2)<< (area_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Density  |            " << left << setw(12) << violated_bins << "|      " << setw(21) << density_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (density_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    cout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    cout << "|  Total    |                 " << left << setw(35) << total_cost << "|      100%     |" << endl;
    cout << "+===========+====================================================================+" << endl;
    cout << "|           |                          Numbers                                   |" << endl;
    cout << "+-----------+--------------------------------------------------------------------|" << endl;
    cout << "|    FFs    |                            " << left << setw(8) << ff_num << "                                |" << endl;
    cout << "+ - - - - - + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|" << endl;
    cout << "|    Bits   |                            " << left << setw(8) << bit_num << "                                |" << endl;
    cout << "+================================================================================+" << endl;
    cout << endl;

    cout << "+---------------------+" << endl;
    cout << "|  Size  Distribution |" << endl;
    cout << "+---------------------+" << endl;

    for(int i=1; i<=LIB->max_ff_size; i++){
    cout << "|  Size " << right << setw(3) << " |   " << setw(5) << arr[i] << "    |" << endl; 
    }
    cout << "+---------------------+" << endl;
    cout << endl;

    fout << endl;
    fout << "+================================================================================+" << endl;
    fout << "|                                Result    Report                                |" << endl;
    fout << "+===========+========================+===========================+===============+" << endl;
    fout << "|           |         Value          |            Cost           |   Percentage  |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  TNS/WNS  |   " << right << setw(10) << tns << "/" << left << setw(10) << wns << "|      " << setw(21) << tns_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (tns_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Power    |       " << left << setw(17) << power << "|      " << setw(21) << power_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (power_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Area     |       " << left << setw(17) << area << "|      " << setw(21) << area_cost << "|    " << left << setw(6) << fixed << setprecision(2)<< (area_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Density  |            " << left << setw(12) << violated_bins << "|      " << setw(21) << density_cost << "|    " << left << setw(6) << fixed << setprecision(3)<< (density_cost*(100)/total_cost) << defaultfloat << "%    |" << endl;
    fout << "+-----------+------------------------+---------------------------+---------------|" << endl;
    fout << "|  Total    |                 " << left << setw(35) << total_cost << "|      100%     |" << endl;
    fout << "+===========+====================================================================+" << endl;
    fout << "|           |                          Numbers                                   |" << endl;
    fout << "+-----------+--------------------------------------------------------------------|" << endl;
    fout << "|    FFs    |                            " << left << setw(8) << ff_num << "                                |" << endl;
    fout << "+ - - - - - + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|" << endl;
    fout << "|    Bits   |                            " << left << setw(8) << bit_num << "                                |" << endl;
    fout << "+================================================================================+" << endl;
    fout << endl;
}

double costeva::calArea(){
    double area = 0;

    for(auto f: *ffs){
        area = area + f->type->area;
    }

    return area;
}

double costeva::calPower(){
    double power = 0;

    for(auto f: *ffs){
        power = power + f->type->gate_power;
    }

    return power;
}

double costeva::calTns(double* WNS){
    double wns = 0;
    int neg_ff_cnt = 0;
    for(auto& it: INST->gate_umap){
        auto g = it.second;
        g->v = false;
        g->is_tracking = false;
    }

    for(auto& f: *ffs){
        f->update_pin_loc();
    }

    double tns = 0;
    for(auto& f: *ffs){

        double worst_slack = 0;
        for(int i=0; i<f->d_pins.size(); i++){
            double slack = 0;
            pin* p = f->d_pins[i];
            
            if(p->to_net->ipins.front()->pin_type == 'f'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
                double new_hpwl = abs(sp->new_coox - p->new_coox) + abs(sp->new_cooy - p->new_cooy);
                double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
                double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay) - (new_qpin_delay - ori_qpin_delay); 
            }
            else if(p->to_net->ipins.front()->pin_type == 'd'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                double cen_ori_hpwl = abs(sp->coox - p->to_ff->cen_x) + abs(sp->cooy - p->to_ff->cen_y);
                double cen_new_hpwl = abs(sp->coox - p->to_new_ff->cen_x) + abs(sp->cooy - p->to_new_ff->cen_y);

                slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                
            }
            else if(p->to_net->ipins.front()->pin_type == 'g'){
                auto sp = p->to_net->ipins.front(); // sp: source pin
                double ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                double new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                double temp_ct  = (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
                double cen_ori_hpwl = abs(sp->coox - p->to_ff->cen_x) + abs(sp->cooy - p->to_ff->cen_y);
                double cen_new_hpwl = abs(sp->coox - p->to_new_ff->cen_x) + abs(sp->cooy - p->to_new_ff->cen_y);
                if(get_ct(sp->to_gate) == numeric_limits<double>::lowest()){
                    slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);

                }
                else{
                    slack = p->slack - (temp_ct + get_ct(sp->to_gate));
                }

            }
            if(slack < 0){
                tns = tns + slack;
                neg_ff_cnt++;
                if(slack < wns) wns = slack;
            }  
        }
    }
    *WNS =  wns;

    return tns;
}

double costeva::get_ct(gatei* g){
    if(g->v == true){
        return g->consume_time;
    }
    
    if(g->is_tracking == true){
        return numeric_limits<double>::lowest();
    }
    g->is_tracking = true;
    double max_ct = numeric_limits<double>::lowest(); // ct: consume time

    for(auto p: g->ipins){
        if(p->to_net == NULL) continue;

        double temp_ct;
        if(p->to_net->ipins.front()->pin_type == 'g'){
            temp_ct = get_ct(p->to_net->ipins.front()->to_gate);
            if(temp_ct > max_ct) max_ct = temp_ct;
        }
        else if(p->to_net->ipins.front()->pin_type == 'f'){
            auto sp = p->to_net->ipins.front(); // sp: source pin
            double ori_hpwl = abs(sp->coox     - p->coox) + abs(sp->cooy     - p->cooy);
            double new_hpwl = abs(sp->new_coox - p->coox) + abs(sp->new_cooy - p->cooy);
            double ori_qpin_delay = sp->to_ff->type->get_Qpin_delay();
            double new_qpin_delay = sp->to_new_ff->type->get_Qpin_delay();


            temp_ct = (new_qpin_delay - ori_qpin_delay) + (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            if(temp_ct > max_ct) max_ct = temp_ct;
        }
        else if(p->to_net->ipins.front()->pin_type == 'd'){
            continue;
        }
    }
    g->v = true;
    g->consume_time = max_ct;
    return max_ct;
}

double costeva::calOriTns(){
    int cnt = 0;
    double tns = 0;

    for(auto f: *ffs){
        for(int i=0; i<f->d_pins.size(); i++){
            cnt++;
            if(f->d_pins[i]->slack < 0) tns = tns + f->d_pins[i]->slack;
        }
    }


    return tns;
}


double costeva::aproxTns(){
    int neg_d_cnt = 0;

    for(auto& f: *ffs){
        f->update_pin_loc();
    }

    double tns = 0;
    double temp_slack;
    double ori_hpwl;
    double new_hpwl;

    for(auto f: *ffs){
        // for(auto p: f->q_pins){
        //     double q_slack = 0;
        //     for(auto tp: p->to_net->opins){
        //         if(tp->pin_type == 'g'){
        //             ori_hpwl = abs(tp->coox - p->coox) + abs(tp->cooy - p->cooy);
        //             new_hpwl = abs(tp->coox - p->new_coox) + abs(tp->cooy - p->new_cooy);
        //             temp_slack = tp->to_gate->get_critical_slack() - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
        //             if(temp_slack < 0) q_slack = q_slack + temp_slack;
        //             if(temp_slack < 0) tns = tns - temp_slack;
        //         }
        //     }
        // }
        for(auto p: f->d_pins){
            auto sp = p->to_net->ipins.front();
            if(sp->pin_type == 'g'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                temp_slack = p->dspd_slk - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            }
            else if(sp->pin_type == 'd'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->coox - p->new_coox) + abs(sp->cooy - p->new_cooy);
                temp_slack = p->dspd_slk - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            }
            else if(sp->pin_type == 'f'){
                ori_hpwl = abs(sp->coox - p->coox) + abs(sp->cooy - p->cooy);
                new_hpwl = abs(sp->new_coox - p->new_coox) + abs(sp->new_cooy - p->new_cooy);
                temp_slack = p->slack - (new_hpwl - ori_hpwl)*(DIE->displacement_delay);
            }

            if(temp_slack < 0){
                tns = tns - temp_slack;
                neg_d_cnt++;
            } 
        }
    }

    // cout << "neg d count: " << neg_d_cnt << endl;
    return tns;
}

double costeva::aproxOriTns(){
    double tns = 0;
    double temp_slack;
    int qcnt = 0;
    int dcnt = 0;
    for(auto f: *ffs){
        for(auto p: f->q_pins){
            double q_slack = 0;
            if(p->dspd_slk < 0) {
                tns = tns - p->dspd_slk;
                qcnt++;
            }
            // for(auto tp: p->to_net->opins){
            //     if(tp->pin_type == 'g'){
            //         temp_slack = tp->to_gate->get_critical_slack();
            //         if(temp_slack < 0) tns = tns - temp_slack;
            //     }
            //     else if(tp->pin_type == 'f'){
            //         if(p->dspd_slk < 0) tns = tns - p->dspd_slk;
            //     }
            // }
        }
        for(auto p: f->d_pins){
            if(p->dspd_slk < 0) {
                // tns = tns - p->dspd_slk;
                dcnt++;
            }
        }
    }
    cout << "neg d cnt = " << dcnt << endl;
    cout << "neg q cnt = " << qcnt << endl;
    return tns;
}

int costeva::calDensity(){
    int horizon_bin_num  = ceil(DIE->die_width/DIE->bin_width);
    int vertical_bin_num = ceil(DIE->die_height/DIE->bin_height);
    vector<vector<bin*>> bin_array(vertical_bin_num, std::vector<bin*>(horizon_bin_num));

    // initial bin array: begin
    int temp;
    int row_idx;
    int col_idx = 0;
    bin_array.resize(vertical_bin_num);
    for(row_idx=0; row_idx<vertical_bin_num; row_idx++){
        for(col_idx=0; col_idx<horizon_bin_num; col_idx++){
            double bin_width = (DIE->bin_width*col_idx+DIE->bin_width <= DIE->die_width) ? DIE->bin_width : (DIE->die_width - (DIE->bin_width*col_idx));
            double bin_height = (DIE->bin_height*row_idx+DIE->bin_height <= DIE->die_height) ? DIE->bin_height : (DIE->die_height - (DIE->bin_height*row_idx));
            
            bin* nb = new bin(bin_width, bin_height, DIE->bin_util, DIE->bin_width*col_idx, DIE->bin_height*row_idx);
            bin_array[row_idx][col_idx] = nb;
        }
    }

    for(row_idx=0; row_idx<vertical_bin_num; row_idx++){
        for(col_idx=0; col_idx<horizon_bin_num; col_idx++){
            if(col_idx+1<horizon_bin_num) {
                bin_array[row_idx][col_idx]->next_bin = bin_array[row_idx][col_idx+1];
            }
            if(row_idx+1<vertical_bin_num) {
                bin_array[row_idx][col_idx]->up_bin = bin_array[row_idx+1][col_idx];
            }   
        }
    }
    // initial bin array: end

    // add gate cell area to bin: begin
    for(auto it: INST->gate_umap){
        gatei* g = it.second;
        int x_idx = g->coox/DIE->bin_width;
        int y_idx = g->cooy/DIE->bin_height;
        if(bin_array[y_idx][x_idx]->add_cell(g->coox, g->cooy, g->type->size_x, g->type->size_y) == false){
            return -1;
        }
    }
    // add gate cell area to bin: end

    // add ff cell area to bin: begin
    for(auto f: *ffs){
        int x_idx = f->coox/DIE->bin_width;
        int y_idx = f->cooy/DIE->bin_height;
        if(bin_array[y_idx][x_idx]->add_cell(f->coox, f->cooy, f->type->size_x, f->type->size_y) == false){
            return -1;
        }
    }
    // add ff cell area to bin: end

    // count violate bins: begin
    int violate_bin_num = 0;
    for(row_idx=0; row_idx<vertical_bin_num; row_idx++){
        for(col_idx=0; col_idx<horizon_bin_num; col_idx++){
            if(bin_array[row_idx][col_idx]->check_violation() == true){
                violate_bin_num++;
            }
        }
    }
    // count violate bins: end

    // delete: begin
    for(row_idx=0; row_idx<vertical_bin_num; row_idx++){
        for(col_idx=0; col_idx<horizon_bin_num; col_idx++){
            delete bin_array[row_idx][col_idx];
        }
    }
    // delete: end

    return violate_bin_num;
}

