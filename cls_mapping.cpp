#include "func.h"



bool cmp_x(reg* a, reg* b){
    return a->cen_x < b->cen_x;
}
bool cmp_y(reg* a, reg* b){
    return a->cen_y < b->cen_y;
}

void SortRegList(list<reg*>& temp_list){
    // Calculate the variance in X and Y direction
    double var_x, var_y;
    double mean_x, mean_y;
    double sum;
    double dif;
    
    // X direction
    // Calculate Mean
    sum = 0;
    for(auto r: temp_list){
        sum = r->cen_x + sum;
    }
    mean_x = sum/(double)temp_list.size();
    // Calculate Variance
    sum = 0;
    for(auto r: temp_list){
        dif = r->cen_x - mean_x;
        sum = dif*dif + sum;
    }
    var_x = sum/(double)temp_list.size();
    
    // Y direction
    // Calculate Mean
    sum = 0;
    for(auto r: temp_list){
        sum = r->cen_y + sum;
    }
    mean_y = sum/(double)temp_list.size();
    // Calculate Variance
    sum = 0;
    for(auto r: temp_list){
        dif = r->cen_y - mean_y;
        sum = dif*dif + sum;
    }
    var_y = sum/(double)temp_list.size();

    if(var_x > var_y){
        temp_list.sort(cmp_x);
    }
    else{
        temp_list.sort(cmp_y);
    }

    return;
}

void MapClstoMBFF(lib& LIB, list<cluster*>& KCR, list<ffi*>& MBFFS){
    bool PRINT_INFO = false;
    if(PRINT_INFO) cout << endl << "Mapping cluster to MBFF >>>" << endl;

    int cnt;
    ffi* mbff;
    reg* rp; // reg pointer
    list<reg*> reg_list;
    list<reg*> temp_list;

    MBFFS.clear();

    cnt = 0;
    for(auto& c: KCR){
        temp_list.clear();
        // cout << "C: " << c->member_list.size() << endl;

        if(c->member_list.size() == 0) continue;

        // For each cluster, push each bit of reg into 'temp list'
        for(auto& f: c->member_list){
            for(int b=0; b<f->type->bit_num; b++){
                rp = new reg;
                rp->dpin = f->d_pins[b];
                rp->qpin = f->q_pins[b];
                rp->update_cen();
                temp_list.push_back(rp);
            }
        }    

        SortRegList(temp_list);

        // Assign each single reg in the 'temp list' to new MBFF
        string inst_name;
        for(auto fc: LIB.opt_fftable[c->member_list.size()]){
            inst_name = "";
            inst_name = inst_name + "MA" + to_string(cnt);
            // cout << inst_name << endl;
            mbff = new ffi(inst_name, 0, 0);
            mbff->type = fc;
            mbff->d_pins.reserve(fc->bit_num);
            mbff->q_pins.reserve(fc->bit_num);

            for(int i=0; i<fc->bit_num; i++){
                if(temp_list.size() <= 0) break;
                
                rp = temp_list.front();
                rp->dpin->to_new_ff = mbff;
                rp->qpin->to_new_ff = mbff;
                mbff->d_pins.push_back(rp->dpin);
                mbff->q_pins.push_back(rp->qpin);
                temp_list.pop_front();
                // reg_list.push_back(rp);
            }      
            mbff->new_coor();
            mbff->clk_pin = new pin;
            MBFFS.push_back(mbff);   
            cnt++;     
        }
    }
}