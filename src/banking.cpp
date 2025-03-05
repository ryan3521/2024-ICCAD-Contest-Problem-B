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
    testList.clear();
    LG->Initialize();
    cout << "Banking ..." << endl; 
    RunBanking(); 
    RenameAllFlipFlops();

    testList.sort(cmp_ff);
    for(auto f: testList){
        // cout << "Timing degradation: " << f->timingDegradation << ",\t Gain: " << f->gain << endl; 
        // cout << "-------------------------------------------------------------------------------------------------------" << endl;
        // for(int i=0; i<f->size; i++){
        //     cout << "D" << i << " degradation: " << f->bitsD_TimingDegradation[i] << "   "; 
        // }
        // cout << endl;
        // cout << "-------------------------------------------------------------------------------------------------------" << endl;
        // for(int i=0; i<f->size; i++){
        //     cout << "Q" << i << " degradation: " << f->bitsQ_TimingDegradation[i] <<  "   "; 
        // }
        // cout << endl;
        // cout << endl;
    }


    return;
}


void banking::RunBanking(){

    bool break_flag = false;

    for(target_size=LIB->max_ff_size; target_size>=LIB->min_ff_size; target_size--){
        for(int loop=0; loop<1; loop++){
            cout << endl << "Bank target size = " << target_size << endl;
            for(auto ff_list: ff_groups){
            
                banking_ffs.clear();
                for(auto f: *ff_list) {
                    if(f->size == 1){
                        banking_ffs.push_back(f);
                        f->isClustered = false;
                    }
                }
                // cout << "Constructing R tree ..." << endl;
                ConstructRtree();


                if(LIB->fftable_cost[target_size].empty()) continue;

                banking_ffs.sort(cmp_ff_y);

                for(auto targetFF: banking_ffs){
                    if(targetFF->isClustered) continue;
                    ffcell* idealType = LIB->fftable_cost[target_size].front();
                    
                    vector<Value> candidates;
                    candidates.reserve(target_size);
                    for(int i=0; i<target_size; i++){
                        // Decide query point
                        double query_x = targetFF->coox + (idealType->d_pins[i].x_plus + idealType->q_pins[i].x_plus)/2;
                        double query_y = targetFF->cooy + (idealType->d_pins[i].y_plus + idealType->q_pins[i].y_plus)/2;
                        Point query_point(query_x, query_y);    
                        
                        // Query
                        vector<Value> nearest_results;
                        rtree.query(bgi::nearest(query_point, 1), std::back_inserter(nearest_results));  
                        if(nearest_results.empty() == false){
                            candidates.push_back(nearest_results.front());
                            
                            // remove single bit ffs from R tree
                            for(const auto& item : nearest_results) {
                                rtree.remove(item);
                            }
                        }
                        else{
                            break;
                        }
                    }
                    // If this cluster have positive gain, generate a new multibit ff        
                    bool success = TestCluster(target_size, candidates);

                }
            } 
            if(target_size == 1){                
                for(auto ff_list: ff_groups){
                    for(auto f: *ff_list) {
                        if(f->size == target_size){
                            LG->AddToBePlacedFF(f);
                        }
                    }
                }
            }
            cout << "Legaizing FFs ..." << endl;
            if(target_size >= 1){
                LG->LegalizeAllBins();
                LG->ClearAllBins();
            }
            if(target_size == 1){
                break;
            }
        }
    }
    
    return;
}

void banking::RenameAllFlipFlops(){
    int cnt = 0;
    string inst_name;

    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
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
            inst_name = inst_name + "NEWFF" + to_string(nameCount);
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
            // nf->members.push_back(nf);
            nameCount++;
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

bool banking::cmp_ff_y(ffi* a, ffi* b){
    return a->cooy < b->cooy;
}

bool banking::cmp_ff_slack(ffi* a, ffi* b){
    return a->d_pins.front()->slack < b->d_pins.front()->slack;
}


bool banking::cmp_ff(ffi* a, ffi* b){
    return a->timingDegradation > b->timingDegradation;
}




void banking::InitialFFsCost(){
    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            f->gain = 0;
        }    
    }
}



void banking::ConstructRtree(){
    rtree.clear();

    for(auto f: banking_ffs){
        rtree.insert(std::make_pair(Rectangle(Point(f->coox, f->cooy), Point(f->coox + f->type->size_x, f->cooy + f->type->size_y)), f));
    }
}


bool banking::TestCluster(int targetSize, vector<Value>& nearest_result){
    if(nearest_result.size() != targetSize) return false;

    // Construct a pseudo ff
    string inst_name = "";
    inst_name = inst_name + "NEWFF" + to_string(nameCount);
    ffi* pseudoFF = new ffi(inst_name, 0, 0);
    nameCount++;
    
    pseudoFF->size = targetSize;
    for(auto& value: nearest_result) pseudoFF->members.push_back(value.second);
    
    pseudoFF->type = LIB->fftable_cost[targetSize].front();
    pseudoFF->d_pins.reserve(targetSize);
    pseudoFF->q_pins.reserve(targetSize);
    pseudoFF->members.sort(cmp_ff_y);
    for(auto& f: pseudoFF->members) pseudoFF->d_pins.push_back(f->d_pins.front());
    for(auto& f: pseudoFF->members) pseudoFF->q_pins.push_back(f->q_pins.front());
    pseudoFF->clk_pin = new pin;
    pseudoFF->update_coor();

    double idealCoox = pseudoFF->coox;
    double idealCooy = pseudoFF->cooy;
    // // Place Trial
    LG->FindLegalLocation(pseudoFF);
    pseudoFF->displacement = abs(idealCoox - pseudoFF->coox) + abs(idealCooy - pseudoFF->cooy);
    

    // Calculate new MBFF's gain
    double timingDegradation;
    double totalGain = pseudoFF->CalculateGain(DIE);

    // double timingDegradation = pseudoFF->CalculateTimingDegradation(DIE->displacement_delay);
    // double timingGain = (-1)*(DIE->Alpha)*timingDegradation;
    // double powerGain  = 0;
    // double areaGain   = 0; 


    // totalDegradation = totalDegradation + timingDegradation;

    // for(auto& value: nearest_result){
    //     powerGain = powerGain + value.second->type->gate_power;
    //     areaGain  = areaGain + value.second->type->area;
    // }
    
    // powerGain = (DIE->Beta)*(powerGain - pseudoFF->type->gate_power);
    // areaGain  = (DIE->Gamma)*(areaGain - pseudoFF->type->area);

    // double totalGain = timingGain + powerGain + areaGain;

    // Calculate the gain banked by smaller size's MBFF
    if(targetSize > 1){
        pseudoFF->membersAreaPlusPowerGain = 0;
        for(auto& value: nearest_result){
            pseudoFF->membersAreaPlusPowerGain = pseudoFF->membersAreaPlusPowerGain
             + (DIE->Beta) *(value.second->type->gate_power)
             + (DIE->Gamma)*(value.second->type->area);
        }
        
        
        int remainBitNum = targetSize;
        while(remainBitNum > 0){
            int memberSize = targetSize - 1;
            while(LIB->fftable_cost[memberSize].empty() == true || memberSize > remainBitNum){
                memberSize--;
            }
            
            pseudoFF->membersAreaPlusPowerGain = pseudoFF->membersAreaPlusPowerGain
            - (DIE->Beta) *(LIB->fftable_cost[memberSize].front()->gate_power)
            - (DIE->Gamma)*(LIB->fftable_cost[memberSize].front()->area);
            remainBitNum = remainBitNum - memberSize;
        }
    }
    
    bool formNewMBFF = false;

    if(target_size > 1){
        if(totalGain <= 0){
            formNewMBFF = false;
        }
        else if(pseudoFF->membersAreaPlusPowerGain*1.049 > totalGain){
            formNewMBFF = false;
        }
        else {
            formNewMBFF = true;
        }
    }
    else{
        if(totalGain > 0){
            formNewMBFF = true;
        }
        else{
            formNewMBFF = false;
        }
    }


    if(formNewMBFF == true){
        pseudoFF->gain = totalGain;
        
        // push into the ff group list
        pseudoFF->to_list = nearest_result.front().second->to_list;
        pseudoFF->to_list->push_front(pseudoFF);
        pseudoFF->it_pointer = pseudoFF->to_list->begin();
        
        // remove single bit ffs from ff group list
        for(auto& value: nearest_result){
            value.second->isClustered = true;
            value.second->to_list->erase(value.second->it_pointer);
        }
        if(target_size > 1){
            LG->AddToBePlacedFF(pseudoFF);
        }
        // remove single bit ffs from R tree
        // for(const auto& item : nearest_result) {
        //     rtree.remove(item);
        // }
        

        if(pseudoFF->size == 4){
            testList.push_back(pseudoFF);
        }

        return true;
    }
    else{
        LG->RemoveFFArea(pseudoFF);

        for(auto& value: nearest_result){
            value.second->update_coor();
            timingDegradation = value.second->CalculateTimingDegradation(DIE->displacement_delay);
            totalDegradation = totalDegradation + timingDegradation;
        }

        // add single bit flip flops back into R tree
        for(auto value: nearest_result){
            auto f = value.second;
            rtree.insert(std::make_pair(Rectangle(Point(f->coox, f->cooy), Point(f->coox + f->type->size_x, f->cooy + f->type->size_y)), f));
        }

        return false;
    }
}       
