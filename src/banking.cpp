#include "banking.h"

banking::banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS){
    this->PM   = PM;
    this->INST = INST;
    this->LIB  = LIB;
    this->DIE  = DIE;
    this->PFFS = PFFS;
    this->LG   = new Legalizer(INST, LIB, DIE, PM);
    SUCCESS = true;
    FAIL    = false;
    cls = new cluster(INST, LIB, DIE);
}

void banking::run(){
    CopyOriginalFFs();
    InitialFFsCost();

    LG->Initialize();
    cout << "Banking ..." << endl; 
    RunBanking(); 
    cout << "Legalizing ..." << endl;
    RunLegalization();
    // PlaceAndDebank();
    RenameAllFlipFlops();
    return;
}

void banking::RunLegalization(){
    cout << "Adding FFs to Bins ..." << endl;
    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            LG->AddToBePlacedFF(f);
        }
    }
    cout << "Legaizing Bins ..." << endl;
    LG->LegalizeAllBins();
}


void banking::PlaceAndDebank(){
    PM->GatePlacement();
    
    vector<list<ffi*>> place_order_array(LIB->max_ff_size+1);
    list<ffi*> place_fail_ffs;

    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            place_order_array[f->size].push_back(f);
        }
    }


    int initial_fail_count = 0;
    int change_fail_count = 0;
    int change_success_count = 0;
    bool set_constrain = true;
    double displace_constrain = 200;
    for(int i=LIB->max_ff_size; i>0; i--){



        place_order_array[i].sort(cmp_ff_x);
        place_fail_ffs.clear(); 


        cout << "Legalizing " << i << " size MBFF" << endl;

        for(auto f: place_order_array[i]){
            if(PM->placeFlipFlop(f, set_constrain, displace_constrain) == FAIL){
                initial_fail_count++;
                if(ChangeTypeAndTry(f) == FAIL){
                    change_fail_count++;
                    if(f->size != LIB->min_ff_size){
                        place_fail_ffs.push_back(f);
                    }
                    else{
                        PM->placeFlipFlop(f, false, displace_constrain);
                    }
                }
                else {
                    change_success_count++;
                }
            }
        }

        for(auto f: place_fail_ffs){
            if(PM->placeFlipFlop(f, false, 600) == FAIL){
                list<ffi*> debank_list;

                Debank(f, debank_list);
                for(auto small_f: debank_list){
                    place_order_array[small_f->size].push_back(small_f);
                }
            }
            else{
                // double cost1 = 0;
                // double cost2 = 0;
                // cost1 = DIE->Alpha*f->get_timing_cost(f->coox, f->cooy, DIE->displacement_delay)
                //         + DIE->Beta*f->type->gate_power + DIE->Gamma*f->type->area;

                // for(auto sf: f->members) cost2 = cost2 + sf->cost;

                // if(cost2 <= cost1){
                //     PM->DeleteFlipFlop(f);
                //     list<ffi*> debank_list;
                //     Debank(f, debank_list);
                //     for(auto small_f: debank_list){
                //         place_order_array[small_f->size].push_back(small_f);
                //     }
                // }
            }
        }
    }
}

void banking::RunBanking(){

    bool break_flag = false;

    for(auto ff_list: ff_groups){
        
        banking_ffs.clear();
        for(auto f: *ff_list) {
            banking_ffs.push_back(f);
            f->isClustered = false;
        }
        cout << "Constructing R tree ..." << endl;
        ConstructRtree();


        for(target_size=LIB->max_ff_size; target_size>=LIB->min_ff_size; target_size--){
            if(LIB->fftable_cost[target_size].empty()) continue;

            banking_ffs.sort(cmp_ff_x);

            for(auto targetFF: banking_ffs){
                if(targetFF->isClustered) continue;

                // cout << "find nearest of " << targetFF->name << endl;

                // Decide query point
                Point query_point(targetFF->coox + targetFF->type->size_x/2, targetFF->cooy + targetFF->type->size_y/2);
                
                // Query
                std::vector<Value> nearest_results;
                rtree.query(bgi::nearest(query_point, target_size), std::back_inserter(nearest_results));  

                // If this cluster have positive gain, generate a new multibit ff        
                bool success = TestCluster(target_size, nearest_results);

                // if(success) {}
                // else cout << "fail" << endl;
            }
        } 

    }

    cout << "Total Timing Degradation: " << totalDegradation << endl;
    return;
}

void banking::RenameAllFlipFlops(){
    int cnt = 0;
    string inst_name;

    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            inst_name = "";
            inst_name = inst_name + "NEWFF" + to_string(cnt);
            f->name = inst_name;
            PFFS->push_back(f);
            cnt++;
        }    
    }
}


void banking::CopyOriginalFFs(){
    string inst_name;
    int newff_cnt = 0;
    list<ffi*>* ff_list;
    for(auto ori_ffs: INST->ffs_sing){
        ff_list = new list<ffi*>;

        for(auto f: *ori_ffs){
            inst_name = "";
            inst_name = inst_name + "NEWFF" + to_string(newff_cnt);
            ffi* nf = new ffi(inst_name, f->coox, f->cooy);

            nf->size = f->size;
            nf->type = f->type;
            nf->d_pins.reserve(f->d_pins.size());
            nf->q_pins.reserve(f->q_pins.size());
            for(auto p: f->d_pins) nf->d_pins.push_back(p);
            for(auto p: f->q_pins) nf->q_pins.push_back(p);
            nf->clk_pin = new pin;
            nf->update_coor();
            // nf->type = LIB->fftable_cost[1].front();
            // nf->update_coor();
            ff_list->push_back(nf);
            nf->to_list = ff_list;
            newff_cnt++;
        }
        ff_groups.push_back(ff_list);

    }


    for(auto flist: ff_groups){
        for(auto it=flist->begin(); it!=flist->end(); it++){
            ffi* f = *it;
            f->it_pointer = it;
        }
    }
}

bool banking::cmp_ff_x(ffi* a, ffi* b){
    return a->coox < b->coox;
}

void banking::OriginalFFs_Placment(){
    bool SET_CONSTRAIN = true;
    bool DONT_SET_CONSTRAIN = false;
    double DISPLACE_CONSTRAIN = 20;

    list<ffi*> buffer_list;
    list<ffi*> multi_bit_stack;

    for(auto flist: ff_groups){
        for(auto it=flist->begin(); it!=flist->end(); it++){
            ffi* f = *it;
            placing_ffs.push_back(f);
        }
    }

    placing_ffs.sort(cmp_ff_x);

    for(auto f: placing_ffs){
        if(PM->placeFlipFlop(f, SET_CONSTRAIN, DISPLACE_CONSTRAIN) == SUCCESS){
            if(f->type->bit_num > 1){
                multi_bit_stack.push_back(f);
            }
        }
        else{
            buffer_list.push_back(f);
        }
    }
}

void banking::InitialFFsCost(){
    // for(auto ff_list: ff_groups){
    //     for(auto f: *ff_list) {
    //         f->CalculateCost(DIE->Alpha,DIE->Beta,DIE->Gamma,DIE->displacement_delay);
    //     }    
    // }
    // double totalDegradation = 0;
    // cout << "Calculating initial timing degradation" << endl;
    // for(auto ff_list: ff_groups){
    //     for(auto f: *ff_list) {
    //         // f->type = LIB->fftable_cost[1].front();
    //         // f->update_coor();
    //         double degradedSlack = f->CalculateTimingDegradation(DIE->displacement_delay);
    //         // cout << degradedSlack << endl;
    //         totalDegradation += degradedSlack;
    //         // if(degradedSlack != 0) break;
    //     }    
    // }
    // cout << "Initial Timing Degradation: " << totalDegradation << endl;
}

void banking::Debank(ffi* big_f, list<ffi*>& debank_list){
    for(auto f: big_f->members){
        f->update_coor();
        big_f->to_list->push_front(f);
        f->it_pointer = big_f->to_list->begin();
        debank_list.push_back(f);
    }

    big_f->to_list->erase(big_f->it_pointer);
    delete big_f;

    return;
}

bool banking::ChangeTypeAndTry(ffi* oriff){
    bool set_constrain = true;
    double displace_constrain = 200;
    bool print = false;
    ffcell* mincost_ftype = NULL;
    ffcell* ori_fftype = oriff->type;
    list<pin*> best_dpins;
    list<pin*> best_qpins;
    list<pin*> dpins;
    list<pin*> qpins;
    double mincost = numeric_limits<double>::max();
    double ori_size_x = oriff->type->size_x;
    double ori_size_y = oriff->type->size_y;
    double cost_per_bit = 0;
    double dismantle_cost = 0;

    // initial valid size: begin
    int valid_bit_size = oriff->size;
    while(LIB->fftable_cost[valid_bit_size].size() == 0){ valid_bit_size++; }
    // initial valid size: end

    for(auto p: oriff->d_pins) dpins.push_back(p);
    for(auto p: oriff->q_pins) qpins.push_back(p);

    for(auto f: oriff->members) dismantle_cost = dismantle_cost + f->cost;
    
    // find best ff type: begin
    for(auto ftype: LIB->fftable_cost[valid_bit_size]){
        double cost = 0;
        double slack, ns;
        list<pin*> dpins_result;
        list<pin*> qpins_result;

        if(((ftype->size_x < ori_size_x) || (ftype->size_y < ori_size_y) || (ftype->size_x == ori_size_x && ftype->size_y == ori_size_y)) == false){
            continue;
        }

        slack = INST->TnsTest(print, dpins, qpins, ftype, DIE->displacement_delay, dpins_result, qpins_result);
        ns = (slack > 0) ? 0 : abs(slack);
        cost = (DIE->Alpha*ns + DIE->Beta*ftype->gate_power + DIE->Gamma*ftype->area);

        if(cost >= dismantle_cost && oriff->size != LIB->min_ff_size){continue;}
        else if(cost < mincost){
            oriff->type = ftype;
            oriff->d_pins.clear();
            oriff->q_pins.clear();
            for(auto p: dpins_result) oriff->d_pins.push_back(p);
            for(auto p: qpins_result) oriff->q_pins.push_back(p);
            oriff->update_coor();
            if(PM->placeFlipFlop(oriff, set_constrain, displace_constrain) == SUCCESS){
                mincost = cost;
                mincost_ftype = ftype;
                best_dpins.clear();
                best_qpins.clear();
                for(auto p: dpins_result) best_dpins.push_back(p);
                for(auto p: qpins_result) best_qpins.push_back(p);
                PM->DeleteFlipFlop(oriff);
            }
        }
    }
    
    if(mincost != numeric_limits<double>::max()){
        oriff->type = mincost_ftype;
        oriff->cost = mincost;
        oriff->d_pins.clear();
        oriff->q_pins.clear();
        for(auto p: best_dpins) oriff->d_pins.push_back(p);
        for(auto p: best_qpins) oriff->q_pins.push_back(p);
        oriff->update_coor();

        if(PM->placeFlipFlop(oriff, set_constrain, displace_constrain) == SUCCESS){
            return true;
        }
        else{
            cout <<  "error3" << endl;
            return false;
        }
    }
    else{

        oriff->type = ori_fftype;
        oriff->d_pins.clear();
        oriff->q_pins.clear();
        for(auto p: dpins) oriff->d_pins.push_back(p);
        for(auto p: qpins) oriff->q_pins.push_back(p);
        oriff->update_coor();

        return false;
    }
            
    // find best ff type: end
}


void banking::ConstructRtree(){
    rtree.clear();

    for(auto f: banking_ffs){
        rtree.insert(std::make_pair(Rectangle(Point(f->coox, f->cooy), Point(f->coox + f->type->size_x, f->cooy + f->type->size_y)), f));
    }
}


bool banking::TestCluster(int targetSize, vector<Value>& nearest_result){
    if(nearest_result.size() != target_size) return false;

    // Construct a pseudo ff
    ffi* pseudoFF = new ffi("PseudoFF", 0, 0);

    
    pseudoFF->size = targetSize;
    for(auto& value: nearest_result) pseudoFF->members.push_back(value.second);
    pseudoFF->type = LIB->fftable_cost[targetSize].front();
    pseudoFF->d_pins.reserve(targetSize);
    pseudoFF->q_pins.reserve(targetSize);
    for(auto& value: nearest_result) pseudoFF->d_pins.push_back(value.second->d_pins.front());
    for(auto& value: nearest_result) pseudoFF->q_pins.push_back(value.second->q_pins.front());
    pseudoFF->clk_pin = new pin;
    pseudoFF->update_coor();
    pseudoFF->update_pin_loc();


    // Do legaization test and get the ideal location
    LG->FindAvailableAndUpdatePin(pseudoFF);

    // Calculate new MBFF's gain
    double timingDegradation = pseudoFF->CalculateTimingDegradation(DIE->displacement_delay);
    double timingGain = (-1)*(DIE->Alpha)*timingDegradation;
    double powerGain  = 0;
    double areaGain   = 0; 

    // cout << "Timing Degradation: " << timingDegradation << endl;
    totalDegradation = totalDegradation + timingDegradation;

    for(auto& value: nearest_result){
        powerGain = powerGain + value.second->type->gate_power;
        areaGain  = areaGain + value.second->type->area;
    }
    
    powerGain = (DIE->Beta)*(powerGain - pseudoFF->type->gate_power);
    areaGain  = (DIE->Gamma)*(areaGain - pseudoFF->type->area);

    double totalGain = timingGain + powerGain + areaGain;

    // cout << totalGain << endl;
    
    if(totalGain > 0){
        // push into the ff group list
        pseudoFF->to_list = nearest_result.front().second->to_list;
        pseudoFF->to_list->push_front(pseudoFF);
        pseudoFF->it_pointer = pseudoFF->to_list->begin();
        
        // remove single bit ffs from ff group list
        for(auto& value: nearest_result){
            value.second->isClustered = true;
            value.second->to_list->erase(value.second->it_pointer);
        }

        // remove single bit ffs from R tree
          for(const auto& item : nearest_result) {
            rtree.remove(item);
        }
        return true;
    }
    else{
        for(auto& value: nearest_result){
            value.second->update_coor();
            timingDegradation = value.second->CalculateTimingDegradation(DIE->displacement_delay);
            totalDegradation = totalDegradation + timingDegradation;
        }
        return false;
    }
}       
