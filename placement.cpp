#include "placement.h"


// *******************************************************************
// *                        CLASS plcmt_row                          *
// *******************************************************************

// Constructor
plcmt_row::plcmt_row(dieInfo* DIE, double sx, double sy, double sw, double sh, int sn){
    is_tested = false;
    is_visited = false;
    start_x = sx;
    start_y = sy;
    site_w = sw;
    site_h = sh;
    site_num = sn;
    glist.clear();
    space_list.clear();
    space_list.push_back(pair<int, int>(0, site_num-1));
    up_rows.clear();
    down_rows.clear();
    this->DIE = DIE;
    xmax = start_x + site_w*site_num;
    ymax = start_y + site_h;

}


bool plcmt_row::add_gblock(double start, double end){
    int si, ei;

    if(start < start_x) start = start_x;
    if(end > xmax) end = xmax;

    si = (start - start_x)/site_w;
    ei = si + ceil((end - start)/site_w) - 1;

    for(auto rIt=space_list.rbegin(); rIt!=space_list.rend(); rIt++){
        if(si>=rIt->first && si<=rIt->second){
            if(si==rIt->first && ei==rIt->second){
                space_list.erase(--rIt.base());
            }
            else if(si==rIt->first && ei<rIt->second){
                rIt->first = ei+1;
            }
            else if(si>rIt->first && ei<rIt->second){
                int t = rIt->second;
                rIt->second = si-1;
                auto it = rIt.base();
                space_list.insert(it, pair<int, int>(ei+1, t));
            }
            else if(si>rIt->first && ei==rIt->second){
                rIt->second = si-1;
            }
            return;
        }
    }

    return false;
}

void plcmt_row::add_fblock(double start, double end){
    int si = (start - start_x)/site_w;
    int ei = si + ceil((end - start)/site_w) - 1;

    for(auto rIt=space_list.rbegin(); rIt!=space_list.rend(); rIt++){
        if(si>=rIt->first && si<=rIt->second){
            if(si==rIt->first && ei==rIt->second){
                space_list.erase(--rIt.base());
            }
            else if(si==rIt->first && ei<rIt->second){
                rIt->first = ei+1;
            }
            else if(si>rIt->first && ei<rIt->second){
                int t = rIt->second;
                rIt->second = si-1;
                auto it = rIt.base();
                space_list.insert(it, pair<int, int>(ei+1, t));
            }
            else if(si>rIt->first && ei==rIt->second){
                rIt->second = si-1;
            }
            return;
        }
    }

}

bool plcmt_row::check_available(double start, double end, double height){
    int si, ei;

    if(start<start_x || ei>=xmax) return false;

    si = (start - start_x)/site_w;
    ei = si + ceil((end - start)/site_w) - 1;


    for(auto it = space_list.begin(); it!=space_list.end(); it++){
        if(si>=it->first && ei<=it->second) {
            if(height > site_h){
                for(auto& pr: up_rows){
                    if(pr->check_available(start, end, height - site_h) == true) return true;
                }
                return false;
            }
            else{
                return true;
            }
        }
    }

    return false;
}

bool plcmt_row::height_available(double height){
    if(height <= site_h) return true;
    else {
        for(auto& pr: up_rows){
            if(pr->height_available(height - site_h)) return true;
        }
        return false;
    }
}


bool plcmt_row::x_overlapped(double x1, double x2, bool& fit){
    fit = false;
    if((x1 == start_x) && (x2 == (xmax))){
        fit = true;
        return true;
    }
    else{
        if(start_x >= x2){
            return false;
        }
        else if(xmax <= x1){
            return false;
        }
        else{
            return true;
        }
    }
}

bool plcmt_row::y_overlapped(double y1, double y2){

    if(start_y >= y2){
        return false;
    }
    else if(ymax <= y1){
        return false;
    }
    else{
        return true;
    }

}

bool plcmt_row::x_inrange(double x1, double x2){
    if(x1>=start_x && x2<=xmax) return true;
    else return false;
}

bool plcmt_row::y_inrange(double y1, double y2){
    if(y1>=start_y && y2<=ymax) return true;
    else return false;
}

void plcmt_row::add_ff(double start, double end, double height){

    this->add_fblock(start, end);
    
    if(height > site_h){
        for(auto& pr: up_rows){
            if(pr->x_inrange(start, end)){
                pr->add_ff(start, end, height - site_h);
                break;
            }        
        }    
    }

    return;
}


// ds: start idx, de: end idx
bool plcmt_row::seg_mincost(ffi* fi, int ds, int de, int dw, int& best_pos_idx, double& mincost, bool dir){
    bool find_new = false;
    double s; // absolute value, not relative
    double e;
    double cost;

    if(de - ds + 1 < dw) return false;

    if(dir==0){
        s = start_x + ds*site_w;
        e = s + fi->type->size_x;
        for(int i=ds; (i+dw-1)<=de; i++){
            if(this->check_available(s, e, fi->type->size_y) == true){
                cost = abs(fi->coox - s) + abs(fi->cooy - start_y);
                // cost = fi->get_timing_cost(s, start_y, DIE->displacement_delay);
                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i;
                    find_new = true;
                }
                else{
                    return find_new;
                }
            }
            s = s + site_w;
            e = e + site_w;
        }
    }
    else{
        s = start_x + (de-dw+1)*site_w;
        e = s + fi->type->size_x;
        for(int i=de-dw+1; i>=ds; i--){
            if(this->check_available(s, e, fi->type->size_y) == true){
                cost = abs(fi->coox - s) + abs(fi->cooy - start_y);
                // cost = fi->get_timing_cost(s, start_y, DIE->displacement_delay);
                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i;
                    find_new = true;
                }
                else{
                    return find_new;
                }
            }
            s = s - site_w;
            e = e - site_w;
        }
    }
    return find_new;
}

double plcmt_row::place_trial(list<plcmt_row*>& tested_list, ffi* fi, bool& available, int& best_pos_idx, double global_mincost){ // ax is available x coordinate
    available = false;
    double mincost = numeric_limits<double>::max(); // local mincost
    double h = fi->type->size_y;
    double w = fi->type->size_x;
    double sy = fi->cooy;
    double sx = fi->coox;
    int dw = ceil(w/site_w); // discrete width
    // dx is the discrete index which is closest to the sx 
    int dx = (round((sx-start_x)/site_w) < 0) ? 0 : (round((sx-start_x)/site_w) >= site_num) ? (site_num-1) : round((sx-start_x)/site_w); 

    
    if(is_tested){
        return mincost;
    }
    else{
        is_tested = true;
        tested_list.push_back(this);
    }


    if(this->height_available(h) == false) return mincost;
    if(this->x_inrange(sx, sx+w) == true){
        if(abs(start_y - sy) >= global_mincost) return mincost;
    }
    else if(sx < start_x){
        if(abs(sx - start_x) + abs(start_y - sy) >= global_mincost) return mincost;
    }
    else{
        if(abs(start_x + site_num*site_w - w - sx) + abs(start_y - sy) >= global_mincost) return mincost;
    }


    if(dx > pivot){
        int front = pivot;
        
        for(auto it = block_list.begin(); it!=block_list.end(); it++){
            if((it->first - front - 1) >= dw){ // if this space section may be available
                if(front+1 > dx){
                    if(available == true){
                        if(abs(best_pos_idx - dx) < abs(front+1 - dx)) break;
                    }
                    if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 0) == true){ 
                        available = true;
                        break;
                    }
                }
                else if(dx>front && dx<it->first){ // if the mincost location is inside this space segment
                    //cout << "in range!" << endl;
                    int de = (dx + dw - 1)<=(it->first-1) ? (dx + dw - 1):(it->first-1);
                    //cout << front+1 << " " << de << endl;
                    if(this->seg_mincost(fi, front+1, de, dw, best_pos_idx, mincost, 1) == true){
                        available = true;
                    }
                    //cout << dx << " " << it->first-1 << endl;
                    if(this->seg_mincost(fi, dx, it->first-1, dw, best_pos_idx, mincost, 0) == true){
                        available = true;
                        break;
                    }
                    //cout << "available: " << available<< endl;
                }
                else{
                    if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 1) == true){
                        available = true;
                    }
                }
            }
            front = it->second;
        }

        if((site_num - front - 1) >= dw){ // if this space section may be available
            if(front+1 > dx){
                if(available == true){
                    if(abs(best_pos_idx - dx) >= abs(front+1 - dx)){
                        if(this->seg_mincost(fi, front+1, site_num-1, dw, best_pos_idx, mincost, 0) == true){ 
                            available = true;
                        }
                    }
                }
                else{
                    if(this->seg_mincost(fi, front+1, site_num-1, dw, best_pos_idx, mincost, 0) == true){ 
                        available = true;
                    }
                }
            }
            else if(dx>front && dx<site_num){ // if the mincost location is inside this space segment
                //cout << "in range!" << endl;
                int de = (dx + dw - 1)<=(site_num-1) ? (dx + dw - 1):(site_num-1);
                //cout << front+1 << " " << de << endl;
                if(this->seg_mincost(fi, front+1, de, dw, best_pos_idx, mincost, 1) == true){
                    available = true;
                }
                //cout << dx << " " << site_num-1 << endl;
                if(this->seg_mincost(fi, dx, site_num-1, dw, best_pos_idx, mincost, 0) == true){
                    available = true;
                }
                //cout << "available: " << available<< endl;
            }
            else{
                if(this->seg_mincost(fi, front+1, site_num-1, dw, best_pos_idx, mincost, 1) == true){
                    available = true;
                }
            }
        }

        for(auto rIt = space_list.rbegin(); rIt!=space_list.rend(); rIt++){
            if(available == true){
                if(abs(best_pos_idx - dx) < abs(rIt->second-dw+1 - dx)) break;
            }
            if(this->seg_mincost(fi, rIt->first, rIt->second, dw, best_pos_idx, mincost, 1) == true){ 
                available = true;
                break;
            }
        }
    }
    else{
        for(auto rIt = space_list.rbegin(); rIt!=space_list.rend(); rIt++){
            if((rIt->second - rIt->first + 1) >= dw){
                if(rIt->second < dx){
                    if(available == true){
                        if(abs(best_pos_idx - dx) < abs(rIt->second-dw+1 - dx)) break;
                    }
                    if(this->seg_mincost(fi, rIt->first, rIt->second, dw, best_pos_idx, mincost, 1) == true){ 
                        available = true;
                        break;
                    }
                }
                else if(dx>=rIt->first && dx<=rIt->second){
                    int de = (dx + dw - 1)<=(rIt->second) ? (dx + dw - 1):(rIt->second);
                    if(this->seg_mincost(fi, rIt->first, de, dw, best_pos_idx, mincost, 1) == true){
                        available = true;
                    }
                    if(this->seg_mincost(fi, dx, rIt->second, dw, best_pos_idx, mincost, 0) == true){
                        available = true;
                        break;
                    }
                }
                else{
                    if(this->seg_mincost(fi, rIt->first, rIt->second, dw, best_pos_idx, mincost, 0) == true){
                        available = true;
                    }
                }
            }
        }

        int front = pivot;       
        for(auto it = block_list.begin(); it!=block_list.end(); it++){
            if((it->first - front - 1) >= dw){ // if this space section may be available
                if(available == true){
                    if(abs(best_pos_idx - dx) < abs(front+1 - dx)) break;
                }
                if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 0) == true){ 
                    available = true;
                    break;
                }
            }
            front = it->second;
        }
        if((site_num - front - 1) >= dw){ // if this space section may be available
            if(available == true){
                if(abs(best_pos_idx - dx) >= abs(front+1 - dx)){
                    if(this->seg_mincost(fi, front+1, site_num-1, dw, best_pos_idx, mincost, 0) == true){ 
                        available = true;
                    }              
                }
            }
            else{
                if(this->seg_mincost(fi, front+1, site_num-1, dw, best_pos_idx, mincost, 0) == true){ 
                    available = true;
                }
            }
        }

    }

    return mincost;
}


double plcmt_row::closest_x(double x){
    if(x < start_x) return start_x;
    else if(x > (start_x + site_num*site_w)) return (start_x + site_num*site_w);
    else return x;
}



// *******************************************************************
// *                        CLASS placement                          *
// *******************************************************************
placement::placement(lib* LIB, inst* INST, dieInfo* DIE){
    temp_rows.clear();
    rows.clear();
    new_ff_cnt = 0;
    this->LIB  = LIB;
    this->INST = INST;
    this->DIE  = DIE;
}

bool placement::row_cmp(plcmt_row* a, plcmt_row* b){
    if(a->start_y == b->start_y) return a->start_x < b->start_x;
    return a->start_y < b->start_y;
}

void placement::addRow(double sx, double sy, double sw, double sh, int sn){
    plcmt_row* pr = new plcmt_row(DIE, sx, sy, sw, sh, sn);
    temp_rows.push_back(pr);
}

void placement::initial(){
    bool is_overlapped;
    bool is_fit;
    list<pair<double, plcmt_row*>> hightlevel_list;

    hightlevel_list.clear();
    temp_rows.sort(row_cmp);
    rows.reserve(temp_rows.size());



    for(auto& pr: temp_rows){
        rows.push_back(pr);
        is_fit = false;
        for(auto& it: hightlevel_list){
            if(pr->start_y == it.first){
                is_overlapped = it.second->x_overlapped(pr->start_x, pr->start_x + pr->site_w*pr->site_num, is_fit);
                if(is_overlapped){
                    pr->down_rows.push_back(it.second);
                    it.second->up_rows.push_back(pr);
                    if(is_fit){
                        it.first = pr->start_y + pr->site_h;
                        it.second = pr;
                        break;
                    }
                }
            }
        }
        if(is_fit == false){
            hightlevel_list.push_back(pair<double, plcmt_row*>(pr->start_y + pr->site_h, pr));
        }
    }

    for(int i=0; i<rows.size(); i++) { rows[i]->idx = i; }

    return;
}

int placement::closest_IDX(double x, double y){
    int idx;
    double cost;
    double mincost = numeric_limits<double>::max();;
    double y_diff;
    double min_y_diff = numeric_limits<double>::max();;
    int eva_idx = (y - rows[0]->start_y)/rows[0]->site_h;


    if(eva_idx >= rows.size()) eva_idx = rows.size() - 1;
    else if(eva_idx < 0) eva_idx = 0;

    int i = eva_idx;

    // Step 1. find the row index whith minimal y_diff (== abs(rows[i]->start_y - y))
    // Find up
    while(i<rows.size()){
        y_diff = abs(rows[i]->start_y - y);
        if(y_diff < min_y_diff){
            idx = i;
            i++;
            min_y_diff = y_diff;
        }
        else{
            break;
        }
    }
    // Find down 
    i = eva_idx - 1;
    while(i>=0){
        y_diff = abs(rows[i]->start_y - y);
        if(y_diff < min_y_diff){
            idx = i;
            i--;
            min_y_diff = y_diff;
        }
        else{
            break;
        }
    }

    // Step 2. Calculate mincost
    if(rows[idx]->x_inrange(x, x)) return idx;
    else if(x < rows[idx]->start_x) mincost = min_y_diff + (rows[idx]->start_x - x);
    else mincost = min_y_diff + (x - (rows[idx]->start_x + rows[idx]->site_num*rows[idx]->site_w));

    // Step 3. Calculate cost for each row which row's abs(start_y-y) < mincost
    int min_y_idx = idx;
    // Find up
    i = min_y_idx;
    while(i<rows.size()){
        y_diff = abs(rows[i]->start_y - y);
        if(y_diff >= mincost) break;
        else{
            if(rows[i]->x_inrange(x, x)) cost = y_diff;
            else if(x < rows[i]->start_x) cost = y_diff + (rows[i]->start_x - x);
            else cost = y_diff + (x - (rows[i]->start_x + rows[i]->site_num*rows[i]->site_w));

            if(cost < mincost){
                idx = i;
                mincost = cost;
                i++;
            }
        }
    }
    // Find down
    i = min_y_idx - 1;
    while(i>=0){
        y_diff = abs(rows[i]->start_y - y);
        if(y_diff >= mincost) break;
        else{
            if(rows[i]->x_inrange(x, x)) cost = y_diff;
            else if(x < rows[i]->start_x) cost = y_diff + (rows[i]->start_x - x);
            else cost = y_diff + (x - (rows[i]->start_x + rows[i]->site_num*rows[i]->site_w));

            if(cost < mincost){
                idx = i;
                mincost = cost;
                i--;
            }
        }
    }

    return idx;
}

void placement::GatePlacement(){

    for(auto it: INST->gate_umap){
        gatei* g = it.second;
        placeGateInst(g->coox, g->cooy, g->type->size_x, g->type->size_y);
    }

    return;
}

void placement::placeGateInst(double x, double y, double w, double h){
        bool fit; // useless
        double xmin = x;
        double xmax = x + w;
        double ymin = y;
        double ymax = y + h;

        int closest_idx = closest_IDX(x, y);
        int idx = closest_idx;
        // find up 
        while(idx < rows.size()){
            if(rows[idx]->start_y > ymax) break;

            if(rows[idx]->x_overlapped(xmin, xmax, fit)==true && rows[idx]->y_overlapped(ymin, ymax)==true){
                if(rows[idx]->x_inrange(xmin, xmax)==true && rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    return;
                }
                else if(rows[idx]->x_inrange(xmin, xmax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(xmin, ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                else if(rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(xmin, ymin, rows[idx]->start_x - xmin, h);
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(xmin, ymin, xmax - rows[idx]->xmax, h);
                    }
                    return;
                }
                else{
                    rows[idx]->add_gblock(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(xmin, ymin, rows[idx]->start_x - xmin, h);
                        xmin = rows[idx]->start_x;
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(xmin, ymin, xmax - rows[idx]->xmax, h);
                        xmax = rows[idx]->xmax;
                    }
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(xmin, ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                break;
            }
            else{
                idx++;
            }
        }

        // find down
        idx = closest_idx - 1;
        while(idx >= 0){
            if(rows[idx]->ymax < ymin) break;

            if(rows[idx]->x_overlapped(xmin, xmax, fit)==true && rows[idx]->y_overlapped(ymin, ymax)==true){
                if(rows[idx]->x_inrange(xmin, xmax)==true && rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    return;
                }
                else if(rows[idx]->x_inrange(xmin, xmax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(xmin, ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                else if(rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->add_gblock(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(xmin, ymin, rows[idx]->start_x - xmin, h);
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(xmin, ymin, xmax - rows[idx]->xmax, h);
                    }
                    return;
                }
                else{
                    rows[idx]->add_gblock(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(xmin, ymin, rows[idx]->start_x - xmin, h);
                        xmin = rows[idx]->start_x;
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(xmin, ymin, xmax - rows[idx]->xmax, h);
                        xmax = rows[idx]->xmax;
                    }
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(xmin, ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                break;
            }
            else{
                idx--;
            }
        }


    return;
}

bool placement::ff_cmp(ffi* a, ffi* b){
    return a->coox < b->coox;
}



void placement::placeFlipFlopInst(list<ffi*>& UPFFS, list<ffi*>& PFFS){
    bool PRINT_INFO = true;
    int idx;
    int pos_idx;
    int best_pos_idx;
    plcmt_row* best_row;
    bool find;      // is true when this ff have space to place
    bool available; // is true when certain placement row have space to place
    double cost;
    double mincost;
    ffi* fi;
    list<ffi*> dismantle_list;
    list<plcmt_row*> tested_list;
    list<plcmt_row*> search_stack;
    int ff_cnt = 0;
 
    double total_time = 0;
    double stage0_time = 0;
    double stage1_time = 0;
    double stage2_time = 0;
    double stage3_time = 0;
    double time_start, time_end;
    double ts, te;
    if(PRINT_INFO) cout << endl << "FFs Placement Legalization >>>" << endl;

    dismantle_list.clear();
    UPFFS.sort(ff_cmp);
    time_start = clock();
    int UPFFS_NUM = UPFFS.size();
    int DISM_NUM = dismantle_list.size();

    while(UPFFS_NUM>0 || dismantle_list.size()>0){
        // cout << ff_cnt << ":" << endl;
        ts = clock();
        mincost = numeric_limits<double>::max();
        find = false; 
        // cout << "\033[2J\033[1;1H";
        // Step 1. Pick one ff from unplaced ff list or dismantle ff list to place
        if(UPFFS_NUM>0 && dismantle_list.size()>0){
            if(UPFFS.front()->coox < dismantle_list.front()->coox){
                fi = UPFFS.front();
                UPFFS.pop_front();
                UPFFS_NUM--;
            }
            else{
                fi = dismantle_list.front();
                dismantle_list.pop_front();
                //DISM_NUM--;
            }
        }
        else if(UPFFS_NUM>0){
            fi = UPFFS.front();
            UPFFS.pop_front();
            UPFFS_NUM--;

        }
        else{
            fi = dismantle_list.front();
            dismantle_list.pop_front();
            //DISM_NUM--;
        }

        te = clock();
        stage0_time = stage0_time + (te - ts);
        ts = clock();

        // Step 2. Calculate the closest placement row for this ff
        idx = closest_IDX(fi->coox, fi->cooy);


        // Step 3. Find the minimal cost around the adjacent area.
        //         In this step, we need to find up and down base on the closest_IDX.
        //         For each search in this two direction, I divide it into two stage:
        //         Stage 1: The search region will be bounded inside the area where is connected to the row with "closest index".
        //         Stage 2: The search region will be the placement rows which index is larger (or small (if in the "Find down direction")) than the "closest index"          
        //         
        //         The search in each stage will have the following limitations:
        //         L1. If the state of "find" is true, and if the searching row's "y_diff" (= start_y - fi->cooy) is larger than mincost, than finish search in this stage.
        //         L2. If the state of "find" is false, in 
        //             Stage 1: stop searching this in this stage if all the upper (or lower) connected rows are beyond the range (is_too_far)
        //             Stage 2: stop searching if the y_diff "is too far", considering x_diff is zero
        //         L3. If the state of "find" is false, and this ff is single bit. The L2 is ineffective, search until "find" is true. 
        tested_list.clear();

        // *************************** Stage 1 ***************************

        
        search_stack.clear();
        search_stack.push_back(rows[idx]);
        rows[idx]->is_visited = true;

        while(search_stack.size() > 0){
            // cout << "PLACE TEST" << endl;
            cost = search_stack.front()->place_trial(tested_list, fi, available, pos_idx, mincost);
            if(available){
                find = true;
                if(cost < mincost){
                    mincost = cost;
                    best_pos_idx = pos_idx;
                    best_row = search_stack.front(); 
                }
            }
            for(auto& p: search_stack.front()->up_rows){
                if(p->is_visited == false){
                    if(find==true){
                        if(abs(p->start_y - fi->cooy) >= mincost){
                            // cout << "row " << p->idx << " larger than mincost " << endl;
                            break;
                        }
                        if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, DIE->displacement_delay) == false){
                            search_stack.push_back(p);
                            p->is_visited = true;
                        }
                        else{
                            // cout << "row " << p->idx << " is too far " << endl;
                        }
                    }
                    else{
                        if(fi->type->bit_num > 1){
                            if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, DIE->displacement_delay) == false){
                                search_stack.push_back(p);
                                p->is_visited = true;
                            }
                        }
                        else{
                            search_stack.push_back(p);
                            p->is_visited = true;
                        }
                    }
                }
            }
            for(auto& p: search_stack.front()->down_rows){
                if(p->is_visited == false){
                    if(find==true){
                        if(abs(p->start_y - fi->cooy) >= mincost){
                            // cout << "row " << p->idx << " larger than mincost " << endl;
                            break;
                        }
                        if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, DIE->displacement_delay) == false){
                            search_stack.push_back(p);
                            p->is_visited = true;
                        }
                        else{
                            // cout << "row " << p->idx << " is too far " << endl;
                        }
                    }
                    else{
                        if(fi->type->bit_num > 1){
                            if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, DIE->displacement_delay) == false){
                                search_stack.push_back(p);
                                p->is_visited = true;
                            }
                        }
                        else{
                            search_stack.push_back(p);
                            p->is_visited = true;
                        }
                    }
                }
            }
            search_stack.pop_front();
        }

        for(auto& p: search_stack) { p->is_visited = false; }
        te = clock();
        stage1_time = stage1_time + (te - ts);
        // *************************** Stage 2 ***************************
        ts = clock();
        int up_i = idx + 1;
        int down_i = idx - 1;
        bool search_up = true;
        bool search_down = true;
        plcmt_row* target_pr;
        while(search_up || search_down){
            if(up_i >= rows.size()){
                search_up = false;
            }

            if(down_i < 0){
                search_down = false;
            }


            // check up_i
            if(search_up){
                target_pr = rows[up_i];
            
                if(find){
                    if(abs(target_pr->start_y - fi->cooy) >= mincost){
                        search_up = false;
                    }
                    else{
                        if(target_pr->is_tested == false){
                            if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, DIE->displacement_delay) == false){
                                cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                                if(available){
                                    find = true;
                                    if(cost < mincost){
                                        mincost = cost;
                                        best_pos_idx = pos_idx;
                                        best_row = target_pr; 
                                    }
                                }
                            }
                        }
                    }
                }
                else{
                    if(fi->type->bit_num == 1){
                        if(target_pr->is_tested == false){
                            cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                            if(available){
                                find = true;
                                if(cost < mincost){
                                    mincost = cost;
                                    best_pos_idx = pos_idx;
                                    best_row = target_pr; 
                                }
                            }
                        }
                    }
                    else{
                        if(target_pr->is_tested == false){
                            if(fi->is_too_far(fi->coox, target_pr->start_y, DIE->displacement_delay) == true){
                                search_up = false;
                            }
                            else{
                                if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, DIE->displacement_delay) == false){
                                    cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                                    if(available){
                                        find = true;
                                        if(cost < mincost){
                                            mincost = cost;
                                            best_pos_idx = pos_idx;
                                            best_row = target_pr; 
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                up_i++;
            }

            // check down_i
            if(search_down){
                target_pr = rows[down_i];
            
                if(find){
                    if(abs(target_pr->start_y - fi->cooy) >= mincost){
                        search_down = false;
                    }
                    else{
                        if(target_pr->is_tested == false){
                            if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, DIE->displacement_delay) == false){
                                cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                                if(available){
                                    find = true;
                                    if(cost < mincost){
                                        mincost = cost;
                                        best_pos_idx = pos_idx;
                                        best_row = target_pr; 
                                    }
                                }
                            }
                        }
                    }
                }
                else{
                    if(fi->type->bit_num == 1){
                        if(target_pr->is_tested == false){
                            cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                            if(available){
                                find = true;
                                if(cost < mincost){
                                    mincost = cost;
                                    best_pos_idx = pos_idx;
                                    best_row = target_pr; 
                                }
                            }
                        }
                    }
                    else{
                        if(target_pr->is_tested == false){
                            if(fi->is_too_far(fi->coox, target_pr->start_y, DIE->displacement_delay) == true){
                                search_down = false;
                            }
                            else{
                                if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, DIE->displacement_delay) == false){
                                    cost = target_pr->place_trial(tested_list, fi, available, pos_idx, mincost);
                                    if(available){
                                        find = true;
                                        if(cost < mincost){
                                            mincost = cost;
                                            best_pos_idx = pos_idx;
                                            best_row = target_pr; 
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                down_i--;
            }
        }

        for(auto& p: tested_list){ 
            p->is_visited = false;
            p->is_tested  = false; 
        }
        te = clock();
        stage2_time = stage2_time + (te - ts);
        // Step 4. After seaching both side and each with two stages, if the "find" is true, place the ff at the mincost row.
        //         If the result "find" is false:
        //              1. If this ff size is not single bit, dismantle this ff.
        //              2. If this ff size if single bit, than placement legalization fail.
        ts = clock();
        if(find == false){
            if(fi->type->bit_num == 1 && PRINT_INFO == true){

                rows[idx]->print_blocklist();
                return;
            }
            else{
                // dismantle fi
                // cout << ff_cnt << " dismantle" << endl;

            }
        }
        else{
            place_formal(fi, best_row, best_pos_idx);


            PFFS.push_back(fi);
            ff_cnt++;
            te = clock();
            stage3_time = stage3_time + (te - ts);
        }
    }
    time_end = clock();
    total_time = time_end - time_start;


    return;
}

void placement::place_formal(ffi* fi, plcmt_row* best_row, int best_pos_idx){
    double start = best_row->start_x + best_pos_idx*best_row->site_w;
    double end = start + fi->type->size_x;
   
    fi->coox = start;
    fi->cooy = best_row->start_y;
    fi->update_pin_loc();
    best_row->add_ff(start, end, fi->type->size_y);
    return;
}
