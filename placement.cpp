#include "placement.h"

// *******************************************************************
// *                        CLASS plcmt_row                          *
// *******************************************************************

// Constructor
void plcmt_row::plcmt_row(double sx, double sy, double sw, double sh, int sn){
    is_tested = false;
    is_visited = false;
    start_x = sx;
    start_y = sy;
    site_w = sw;
    site_h = sh;
    site_num = sn;
    glist.clear();
    block_list.clear();
    space_list.clear();
    up_rows.clear();
    down_rows.clear();
    pivot = -1;
}

bool plcmt_row::cmp_g(gatei* a, gatei* b){
    return a->coox < b->coox;
}

void plcmt_row::sort_gate(){glist.sort(cmp_g); return;}


void plcmt_row::add_gblock(double start, double end){
    int s, e;

    s = (start - start_x)/site_w;
    e = (end - start_x)/site_w + 1;

    if(block_list.size() == 0){
        block_list.push_back(pair<int, int>(s, e));
    }
    else{
        for(auto rIt = block_list.rbegin(); rIt != block_list.rend(); rIt++){
            if(s > rIt->second){
                if(s == rIt->second + 1){
                    rIt->second = e;
                }
                else{
                    block_list.insert(rIt.base(), pair<int, int>(s, e));
                    rIt--;
                }

                if(rIt != block_list.rbegin()){
                    //       rIt       rIt2
                    // .... [s, e] -> [s, e]
                    auto rIt2 = rIt-1;
                    if(rIt->second >= rIt2->first){
                        rIt->second >= rIt2->second;
                        block_list.erase(rIt.base());
                    }
                }
                break;
            }
        }
    }
}


void plcmt_row::add_fblock(double start, double end){
    int si = (start - start_x)/site_w;
    int ei = (end - start_x)/site_w;
    int sj;
    int ej;
    int front;
    
    if(si > pivot){
        front = pivot;
        for(auto it = block_list.begin(); it!=block_list.end(); it++){
            sj = front+1;
            ej = it->first-1;
            if(ej>=sj){
                if(si>=sj && si<=ej){
                    if(si>sj){
                        space_list.push_back(pair<int, int>(sj, si-1));
                    }
                    pivot = ei;
                    return;
                }
                else{ // need to add space [sj, ej] into spacelist
                    space_list.push_back(pair<int, int>(sj, ej));
                }
            }
            front = it->second;
            block_list.pop_front();
        }
    }
    else{
        for(auto rIt=space_list.rbegin(); rIt!=space_list.rend(); rIt++){
            if(si>=rIt->first && si<=rIt->second){
                if(si==rIt->first && ei==rIt->second){
                    space_list.erase(--rIt.base());
                }
                else if(si==rIt->first && ei<rIt->second){
                    rIt->first = ei+1;
                }
                else if(si>rIt->first && ei<rIt->second){
                    auto it = rIt.base();
                    space_list.insert(it, pair<int, int>(ei+1, rIt->second));
                    rIt->second = si-1;
                }
                else if(si>rIt->first && ei==rIt->second){
                    rIt->second = si-1;
                }
                return;
            }
        }
    }
}

bool plcmt_row::check_available(double start, double end, double height){
    int s, e;

    s = (start - start_x)/site_w;
    e = (end - start_x)/site_w + 1;

    if(s<0 || e>=site_num) return false;

    if(s > pivot){
        int front = pivot;
        for(auto it = block_list.begin(); it!=block_list.end(); it++){
            if(s>front && e<it->first) {
                if(height > site_h){
                    for(auto& pr: up_rows){
                        if(check_available(start, end, height - site_h) == true) return true;
                    }
                    return false;
                }
                else{
                    return true;
                }
            }
            else {front = it->second;}
        }  
        return false;
    }
    else{
        for(auto it = space_list.rbegin(); it!=space_list.rend(); it++){
            if(s>=it->first && e<=it->second) {
                if(height > site_h){
                    for(auto& pr: up_rows){
                        if(check_available(start, end, height - site_h) == true) return true;
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
    if((x1 == start_x) && (x2 == (start_x+site_num*site_w))){
        fit = true;
        return true;
    }
    else{
        if(start_x > x2){
            return false;
        }
        else if((start_x+site_num*site_w) < x1){
            return false;
        }
        else{
            return true;
        }
    }
}

bool plcmt_row::x_inrange(double x1, double x2){
    if(x1>=start_x && x2<=(start_x+site_num*site_w)) return true;
    else return;
}

void plcmt_row::add_gate(double start, double end, double height){

    this->add_gblock(start, end);

    if(height > site_h){
        for(auto& pr: up_rows){
            if(pr->x_inrange(start, end)){
                pr->add_gate(start, end, height - site_h);
                break;
            }        
        }
    }

    return;
}

void plcmt::add_ff(double start, double end, double height){
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
                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i
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
                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i
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
}

double plcmt_row::place_trial(list<plcmt_row*> tested_list, ffi* fi, bool& available, int& best_pos_idx, double global_mincost){ // ax is available x coordinate
    available = false;
    int best_pos_idx;
    double mincost = numeric_limits<double>::max(); // local mincost
    double h = fi->type->size_y;
    double w = fi->type->size_x;
    double sy = fi->cooy;
    double sx = fi->coox;
    int dw = (w/site_w) + 1; // discrete width
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
                        if(ceil(best_pos_idx - dx) < ceil(front+1 - dx)) break;
                    }
                    if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 0) == true){ 
                        available = true;
                        break;
                    }
                }
                else if(dx>front && dx<it->first){ // if the mincost location is inside this space segment
                    int de = (dx + dw - 2)<(it->first-1) ? (dx + dw - 2):(it->first-1);
                    if(this->seg_mincost(fi, front+1, de, dw, best_pos_idx, mincost, 1) == true){
                        available = true;
                    }
                    if(this->seg_mincost(fi, dx, it->first-1, dw, best_pos_idx, mincost, 0) == true){
                        available = true;
                        break;
                    }
                }
                else{
                    if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 1) == true){
                        available = true;
                    }
                }
            }
            front = it->second;
        }

        for(auto rIt = space_list.rbegin(); rIt!=space_list.rend(); rIt++){
            if(available == true){
                if(ceil(best_pos_idx - dx) < ceil(rIt->second-dw+1 - dx)) break;
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
                        if(ceil(best_pos_idx - dx) < ceil(rIt->second-dw+1 - dx)) break;
                    }
                    if(this->seg_mincost(fi, rIt->first, rIt->second, dw, best_pos_idx, mincost, 1) == true){ 
                        available = true;
                        break;
                    }
                }
                else if(dx>=rIt->first && dx<=rIt->second){
                    int de = (dx + dw - 2)<(rIt->second) ? (dx + dw - 2):(rIt->second);
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
                    if(ceil(best_pos_idx - dx) < ceil(front+1 - dx)) break;
                }
                if(this->seg_mincost(fi, front+1, it->first-1, dw, best_pos_idx, mincost, 0) == true){ 
                    available = true;
                    break;
                }
            }
            front = it->second;
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
void placement::placement(){
    temp_rows.clear();
    rows.clear();
    rs_list.clear();
    new_ff_cnt = 0;
}

bool placement::row_cmp(plcmt_row* a, plcmt_row* b){
    if(a->start_y == b->start_y) return a->start_x < b->start_x
    return a->start_y < b->start_y;
}

void placement::addRow(double sx, double sy, double sw, double sh, int sn){
    plcmt_row* pr = new plcmt_row(sx, sy, sw, sh, sn);
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
                        it->first = pr->start_y + pr->site_h;
                        it->second = pr;
                        break;
                    }
                }
            }
        }
        if(is_fit == false){
            hightlevel_list.push_back(pair<double, plcmt_row*>(pr->start_y + pr->site_h, pr));
        }
    }

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
    else if(x < rows[idx]->start) mincost = min_y_diff + (rows[idx]->start - x);
    else mincost = min_y_diff + (x - (rows[idx]->start + rows[idx]->site_num*rows[idx]->site_w));

    // Step 3. Calculate cost for each row which row's abs(start_y-y) < mincost
    int min_y_idx = idx;
    // Find up
    i = min_y_idx;
    while(i<rows.size()){
        y_diff = abs(rows[i]->start_y - y);
        if(y_diff >= mincost) break;
        else{
            if(rows[i]->x_inrange(x, x)) cost = y_diff;
            else if(x < rows[i]->start) cost = y_diff + (rows[i]->start - x);
            else cost = y_diff + (x - (rows[i]->start + rows[i]->site_num*rows[i]->site_w));

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
            else if(x < rows[i]->start) cost = y_diff + (rows[i]->start - x);
            else cost = y_diff + (x - (rows[i]->start + rows[i]->site_num*rows[i]->site_w));

            if(cost < mincost){
                idx = i;
                mincost = cost;
                i--;
            }
        }
    }

    return idx;
}

void placement::placeGateInst(inst& INST){

    for(auto& gi: INST.gate_umap){
        double y = gi->cooy;
        double x = gi->coox;
        int idx = closest_IDX(x, y);

        double temp = (x - rows[idx]->start_x)/rows[idx]->site_w;
        if(y != rows[idx]->start_y || abs(temp - int(temp)) > 0.0000001){
            cout << "Error: gate instance is not on site." << endl;
            break;
        }
        
        rows[idx]->glist.push_back(gi);
    }


    for(int i=0; i<rows.size(); i++){
        rows[i].sort_gate();
        for(auto& gi: rows[i]->glist){
            rows[i]->add_gate(gi->coox, gi->coox+gi->type->size_x, gi->type->size_y);
        }
        rows[i]->add_block(rows[i]->site_num, rows[i]->site_num);
    }
}

void placement::ff_cmp(ffi* a, ffi* b){
    return a->coox < b->coox;
}



void placement::placeFlipFlopInst(lib& LIB, inst& INST, dieInfo& DIE, list<ffi*>& UPFFS, , list<ffi*>& PFFS){
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
    plcmt_row* pr;
    


    dismantle_list.clear();
    UPFFS.sort(ff_cmp);

    while(UPFFS.size()>0 || dismantle_list.size()>0){
        mincost = numeric_limits<double>::max();
        find = false; 

        // Step 1. Pick one ff from unplaced ff list or dismantle ff list to place
        if(UPFFS.size()>0 && dismantle_list.size()>0){
            if(UPFFS.front()->coox < dismantle_list.front()->coox){
                fi = UPFFS.front();
                UPFFS.pop_front();
            }
            else{
                fi = dismantle_list.front();
                dismantle_list.pop_front();
            }
        }
        else if(UPFFS.size()>0){
            fi = UPFFS.front();
            UPFFS.pop_front();
        }
        else{
            fi = dismantle_list.front();
            dismantle_list.pop_front();
        }

        // Step 2. Calculate the closest placement row for this ff
        idx = closest_IDX(fi->cooy);


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
                        if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, displacement_delay) == false){
                            search_stack.push_back(p);
                            p->is_visited = true;
                        }
                    }
                    else{
                        if(fi->type->bit_num > 1){
                            if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, displacement_delay) == false){
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
                    if(fi->is_too_far(p->closest_x(fi->coox), p->start_y, displacement_delay) == false){
                        search_stack.push_back(p);
                        p->is_visited = true;
                    }
                }
            }
            search_stack.pop_front();
        }

        for(auto& p: search_stack) { p->is_visited = false; }

        // *************************** Stage 2 ***************************
        int up_i = idx + 1;
        int down_i = idx - 1;
        bool search_up = true;
        bool search_down = true;
        plcmt_row* target_pr;
        while(search_up || search_down){
            if(up_i < rows.size()){
                search_up = true;
            }
            else{
                search_up = false;
            }
            if(down_i >= 0){
                search_down = true;
            }
            else{
                search_down = false;
            }

            // check up_i
            if(search_up){
                target_pr = rows[up_i];
            
                if(find){
                    if(abs(target_pr->start_y - fi->coox) < mincost){
                        search_up = false;
                    }
                    else{
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
                            if(fi->is_too_far(fi->coox, target_pr->start_y, displacement_delay) == true){
                                search_up = false;
                            }
                            else{
                                if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, displacement_delay) == false){
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
                up_i++;
            }

            // check down_i
            if(search_down){
                target_pr = rows[down_i];
            
                if(find){
                    if(abs(target_pr->start_y - fi->coox) < mincost){
                        search_down = false;
                    }
                    else{
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
                            if(fi->is_too_far(fi->coox, target_pr->start_y, displacement_delay) == true){
                                search_down = false;
                            }
                            else{
                                if(fi->is_too_far(target_pr->closest_x(fi->coox), target_pr->start_y, displacement_delay) == false){
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
                down_i--;
            }
        }

        for(auto& p: tested_list){ 
            p->is_visited = false;
            p->is_tested  = false; 
        }

        // Step 4. After seaching both side and each with two stages, if the "find" is true, place the ff at the mincost row.
        //         If the result "find" is false:
        //              1. If this ff size is not single bit, dismantle this ff.
        //              2. If this ff size if single bit, than placement legalization fail.

        if(find == false){
            if(fi->type->bit_num == 1){
                cout << "Error: Placement Legalization Fail!" << endl;
                return;
            }
            else{
                // dismantle fi
                mbff_dismantle(fi, dismantle_list, LIB, DIE);
            }
        }
        else{
            place_formal(fi, best_row, best_pos_idx);
            PFFS.push_back(fi);
        }
    }
    return;
}

void placement::place_formal(ffi* fi, plcmt_row* best_row, int best_pos_idx){
    double start = best_row->start_x + best_pos_idx*best_row->site_h;
    double end = start + fi->type->size_x;
    fi->coox = start;
    fi->cooy = best_row->start_y;

    best_row->add_ff(start, end, fi->type->size_y);
    return;
}

void placement::mbff_dismantle(ffi* fi, list<ffi*>& dismantle_list, lib& LIB, dieInfo& DIE){
    int ori_size  = fi->d_pins.size(); // ori: original
    int new_size1 = (double)ori_size/2.0;
    int new_size2 = ori_size - new_size1; 
    int remain_size;
    int pin_cnt = 0;
    bool dismantle = false;
    bool replace = false;
    ffcell* small_type = LIB.fftable_area[fi->type->bit_num].front();
    ffi* new_fi;
    string inst_name;

    if(fi->type->area == LIB.fftable_area.front()->area){
        // It means that this type of MBFF is the smallest size of this bit.
        // No other choice but only dismantle this mbff.
        dismantle = true;
    }
    else{
        double AP_cost = 0; // area power sum

        for(auto& ft: LIB.opt_fftable[new_size1]){
            AP_cost = AP_cost + DIE.Gamma*ft->area + DIE.Beta*ft->gate_power;
        }
        for(auto& ft: LIB.opt_fftable[new_size2]){
            AP_cost = AP_cost + DIE.Gamma*ft->area + DIE.Beta*ft->gate_power;
        }
        AP_cost = AP_cost/(double)ori_size;

        if(AP_cost <= (DIE.Gamma*small_type->area + DIE.Beta*small_type->gate_power)/(double)ori_size){
            dismantle = true;
        }
        else{
            replace = true;
        }
    }

    if(dismantle){
        remain_size = new_size1;
        for(auto& ft: LIB.opt_fftable[new_size1]){
            inst_name = "";
            inst_name = inst_name + "MB" + to_string(new_ff_cnt);
            new_fi = new ffi(inst_name, 0, 0);
            new_fi->type = ft;
            new_fi->d_pins.reserve(ft->bit_num);
            new_fi->q_pins.reserve(ft->bit_num);

            for(int i=0; i<ft->bit_num; i++){
                if(remain_size <= 0) break;
                
                fi->d_pins[pin_cnt]->to_new_ff = ft;
                fi->q_pins[pin_cnt]->to_new_ff = ft;
                new_fi->d_pins.push_back(fi->d_pins[pin_cnt]);
                new_fi->q_pins.push_back(fi->q_pins[pin_cnt]);

                pin_cnt++;
                remain_size--;
            }      
            new_fi->new_coor();
            new_fi->clk_pin = new pin;
            new_fi->clk_pin->name     = fi->clk_pin->name;
            new_fi->clk_pin->to_net   = fi->clk_pin->to_net;
            new_fi->clk_pin->to_ff    = new_fi;
            new_fi->clk_pin->pin_type = 'f';

            dismantle_list.push_front(new_fi);  
            new_ff_cnt++; 
        }

        remain_size = new_size2;
        for(auto& ft: LIB.opt_fftable[new_size1]){
            inst_name = "";
            inst_name = inst_name + "MB" + to_string(new_ff_cnt);
            new_fi = new ffi(inst_name, 0, 0);
            new_fi->type = ft;
            new_fi->d_pins.reserve(ft->bit_num);
            new_fi->q_pins.reserve(ft->bit_num);

            for(int i=0; i<ft->bit_num; i++){
                if(remain_size <= 0) break;
                
                fi->d_pins[pin_cnt]->to_new_ff = ft;
                fi->q_pins[pin_cnt]->to_new_ff = ft;
                new_fi->d_pins.push_back(fi->d_pins[pin_cnt]);
                new_fi->q_pins.push_back(fi->q_pins[pin_cnt]);

                pin_cnt++;
                remain_size--;
            }      
            new_fi->new_coor();
            new_fi->clk_pin = new pin;
            new_fi->clk_pin->name      = fi->clk_pin->name;
            new_fi->clk_pin->to_net    = fi->clk_pin->to_net;
            new_fi->clk_pin->to_new_ff = new_fi;
            new_fi->clk_pin->pin_type  = 'f';

            dismantle_list.push_front(new_fi);   
            new_ff_cnt++; 
        }
    }
    else if(replace){
        inst_name = "";
        inst_name = inst_name + "MB" + to_string(new_ff_cnt);

        fi->type = small_type;
        fi->name = inst_name;

        for(auto& p: fi->d_pins){
            p->to_new_ff = small_type;
        }
        for(auto& p: fi->q_pins){
            p->to_new_ff = small_type;
        }

        fi->new_coor();
        new_ff_cnt++;
    }
    dismantle_list.sort(ff_cmp);
    return;
}