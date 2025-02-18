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
    RenameAllFlipFlops();

    return;
}


void banking::RunBanking(){

    bool break_flag = false;

    for(target_size=LIB->max_ff_size; target_size>=LIB->min_ff_size; target_size--){
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

            banking_ffs.sort(cmp_ff_x);

            for(auto targetFF: banking_ffs){
                if(targetFF->isClustered) continue;


                // Decide query point
                // Point query_point(targetFF->coox + targetFF->type->size_x/2, targetFF->cooy + targetFF->type->size_y/2);
                Point query_point(targetFF->coox, targetFF->cooy);
                
                // Query
                std::vector<Value> nearest_results;
                rtree.query(bgi::nearest(query_point, target_size), std::back_inserter(nearest_results));  

                // If this cluster have positive gain, generate a new multibit ff        
                bool success = TestCluster(target_size, nearest_results);

            }
        } 
        cout << "Adding Target FFs to Bins ..." << endl;
        for(auto ff_list: ff_groups){
            for(auto f: *ff_list) {
                if(f->size == target_size){
                    LG->AddToBePlacedFF(f);
                }
            }
        }
        cout << "Legaizing FFs ..." << endl;

        LG->LegalizeAllBins();
        LG->ClearAllBins();

        // list<ffi*> placedFFs;
        // for(auto ff_list: ff_groups){
        //     for(auto f: *ff_list) {
        //         if(f->size == target_size){
        //             placedFFs.push_back(f);
        //         }
        //     }
        // }
        // placedFFs.sort(cmp_ff_gain);



        // for(auto f: placedFFs){
        //     if(target_size == 4){
        //         if(f->gain < 128) {
        //             LG->DeleteFF(f);
        //             for(auto m: f->members){
        //             m->update_coor();
        //             m->CalculateTimingDegradation(DIE->displacement_delay);
        //             f->to_list->push_front(m);
        //             m->it_pointer = f->to_list->begin();
        //         }

        //         f->to_list->erase(f->it_pointer);
        //         delete f;
        //         }
        //     }
        // }


    }

    cout << "Total Timing Degradation: " << totalDegradation << endl;
    return;
}

void banking::RenameAllFlipFlops(){
    int cnt = 0;
    string inst_name;

    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            // inst_name = "";
            // inst_name = inst_name + "NEWFF" + to_string(cnt);
            // f->name = inst_name;
            PFFS->push_back(f);

            // if(f->name == "NEWFF23338" || f->name == "NEWFF23230"){
            //     cout << endl;
            //     cout << "FF Name: " << f->name << endl;
            //     cout << "X = " << f->coox << ", Y =  " << f->cooy << endl;
            //     cout << "Width = " << f->type->size_x << ", Height = " << f->type->size_y << endl;
            // }

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


bool banking::cmp_ff_gain(ffi* a, ffi* b){
    return a->gain > b->gain;
}




void banking::InitialFFsCost(){
    // double totalDegradation = 0;
    // cout << "Calculating initial timing degradation" << endl;
    for(auto ff_list: ff_groups){
        for(auto f: *ff_list) {
            // f->type = LIB->fftable_cost[1].front();
            // f->update_coor();
            // double degradedSlack = f->CalculateTimingDegradation(DIE->displacement_delay);
            // cout << degradedSlack << endl;
            // totalDegradation += degradedSlack;
            // if(degradedSlack != 0) break;
            f->gain = 0;
        }    
    }
    // cout << "Initial Timing Degradation: " << totalDegradation << endl;
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
    // for(auto& value: nearest_result) pseudoFF->d_pins.push_back(value.second->d_pins.front());
    // for(auto& value: nearest_result) pseudoFF->q_pins.push_back(value.second->q_pins.front());
    pseudoFF->members.sort(cmp_ff_y);
    for(auto& f: pseudoFF->members) pseudoFF->d_pins.push_back(f->d_pins.front());
    for(auto& f: pseudoFF->members) pseudoFF->q_pins.push_back(f->q_pins.front());
    pseudoFF->clk_pin = new pin;
    pseudoFF->update_coor();

    // if(target_size == 2){
    //     cout << pseudoFF->type->name << endl;
    // }
    // Do legaization test and get the ideal location
    // LG->FindAvailableAndUpdatePin(pseudoFF);

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
    
    if(totalGain > 0){
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
