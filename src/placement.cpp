#include "placement.h"

// *******************************************************************
// *                        CLASS PlacementRow                          *
// *******************************************************************

// Constructor
PlacementRow::PlacementRow(dieInfo* DIE, double sx, double sy, double sw, double sh, int sn){
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
    pivot_valid = false;
    up_rows.clear();
    down_rows.clear();
    this->DIE = DIE;
    xmax = start_x + site_w*site_num;
    ymax = start_y + site_h;

}

void PlacementRow::print_spacelist(){
    cout << endl << "Space List: " << space_list.size() << endl;
    for(auto it = space_list.begin(); it != space_list.end(); it++){
        cout << "[" << it->first << ", " << it->second << "] -> ";
        
    }
    cout << endl;
    cout << endl;
}

void PlacementRow::AddBlockAnyway(double start, double end){
    int start_i, end_i;

    if(start < start_x) start = start_x;
    if(end > xmax) end = xmax;


    start_i = (start - start_x)/site_w;
    end_i   = start_i + ceil((end - start)/site_w) - 1;
    
    auto spaceItr = space_list.begin();
    while(spaceItr != space_list.end()){

        if(spaceItr->first > end_i){
            return;
        }
        else if(start_i > spaceItr->second){
            spaceItr++;
            continue;
        }
        else{
            if(start_i <= spaceItr->first && end_i >= spaceItr->second){
                // space: |OOOOOOO|
                // block: |XXXXXXX|
                spaceItr = space_list.erase(spaceItr);
                continue;
            }
            else if(start_i <= spaceItr->first && end_i < spaceItr->second){
                // space: |OOOOOOO|
                // block: |XXXXX--|
                spaceItr->first = end_i+1;
                return;
            }
            else if(start_i > spaceItr->first && end_i < spaceItr->second){
                // space: |OOOOOOO|
                // block: |--XXX--|
                int t = spaceItr->first;
                spaceItr->first = end_i+1;
                space_list.insert(spaceItr, pair<int, int>(t, start_i-1));
                return;
            }
            else if(start_i > spaceItr->first && end_i >= spaceItr->second){
                // space: |OOOOOOO|
                // block: |--XXXXX|XXX
                spaceItr->second = start_i-1;
                spaceItr++;
                continue;
            }
        }
    }
    return;
}



void PlacementRow::delete_fblock(double start, double end){
    int si = (start - start_x)/site_w;
    int ei = si + ceil((end - start)/site_w) - 1;

    if(space_list.empty()){
        space_list.push_back(pair<int, int>(si, ei));
        return;
    }
    else if(ei < space_list.front().first){
        if(ei == space_list.front().first-1){
            space_list.front().first = si;
        }
        else{
            space_list.push_front(pair<int, int>(si, ei));
        }
        return;
    }
    else if(si > space_list.back().second){
        if(si == space_list.back().second+1){
            space_list.back().second = ei;
        }
        else{
            space_list.push_back(pair<int, int>(si, ei));
        }
        return;
    }

    int front;
    int back;
    auto front_it = space_list.begin();
    for(auto it=space_list.begin(); it!=space_list.end(); it++){
        back = it->first;
        if(it == space_list.begin()){
            front = it->second;
            front_it = it;
            continue;
        }

        if(si>front && ei<back){
            if(si==front+1 && ei==back-1){
                front_it->second = it->second;
                space_list.erase(it);
            }
            else if(si==front+1){
                front_it->second = ei;
            }
            else if(ei==back-1){
                it->first = si;
            }
            else{
                space_list.insert(it, pair<int, int>(si, ei));
            }
            return;
        }
        else{
            front = it->second;
            front_it = it;
        }
    }
}

void PlacementRow::add_fblock(double start, double end){
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

bool PlacementRow::check_available(double start, double end, double height){
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

bool PlacementRow::height_available(double height){
    if(height <= site_h) return true;
    else {
        for(auto& pr: up_rows){
            if(pr->height_available(height - site_h)) return true;
        }
        return false;
    }
}


bool PlacementRow::x_overlapped(double x1, double x2, bool& fit){
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

bool PlacementRow::y_overlapped(double y1, double y2){

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

bool PlacementRow::x_inrange(double x1, double x2){
    if(x1>=start_x && x2<=xmax) return true;
    else return false;
}

bool PlacementRow::y_inrange(double y1, double y2){
    if(y1>=start_y && y2<=ymax) return true;
    else return false;
}

void PlacementRow::add_ff(double start, double end, double height){
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

void PlacementRow::delete_ff(double start, double end, double height){
    this->delete_fblock(start, end);

    if(height > site_h){
        for(auto& pr: up_rows){
            if(pr->x_inrange(start, end)){
                pr->delete_ff(start, end, height - site_h);
                break;
            }        
        }    
    }

    return;
}


// ds: start idx, de: end idx
bool PlacementRow::seg_mincost(ffi* fi, int ds, int de, int dw, int& best_pos_idx, double& mincost, bool dir, bool set_constrain){
    bool find_new = false;
    double s; // absolute value, not relative
    double e;
    double cost;

    if(de - ds + 1 < dw) return false;

    if(dir==0){
        s = start_x + ds*site_w;
        e = s + fi->type->size_x;
        for(int i=ds; (i+dw-1)<=de; i++){
            if(mincost <= (s - fi->coox) || fi->x_allow_dis <= (s - fi->coox)) break;
            if(this->check_available(s, e, fi->type->size_y) == true){
                if(abs(fi->coox - s) > fi->x_allow_dis) break;
                
                cost = abs(fi->coox - s) + abs(fi->cooy - start_y);

                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i;
                    find_new = true;
                    if(set_constrain == false){
                        fi->x_allow_dis = mincost;
                        fi->y_allow_dis = mincost;
                    }
                    else if(fi->y_allow_dis > mincost && fi->x_allow_dis > mincost){
                        fi->x_allow_dis = mincost;
                        fi->y_allow_dis = mincost;
                    }
                    return find_new;
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
            if(mincost <= (fi->coox - s) || fi->x_allow_dis <= (fi->coox - s)) break;
            if(this->check_available(s, e, fi->type->size_y) == true){
                if(abs(fi->coox - s) > fi->x_allow_dis) break;

                cost = abs(fi->coox - s) + abs(fi->cooy - start_y);

                if(cost < mincost) {
                    mincost = cost;
                    best_pos_idx = i;
                    find_new = true;
                    if(set_constrain == false){
                        fi->x_allow_dis = mincost;
                        fi->y_allow_dis = mincost;
                    }
                    else if(fi->y_allow_dis > mincost && fi->x_allow_dis > mincost){
                        fi->x_allow_dis = mincost;
                        fi->y_allow_dis = mincost;
                    }
                    return find_new;
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

double PlacementRow::place_trial(ffi* fi, bool& available, int& best_pos_idx, double global_mincost, bool set_constrain){ // ax is available x coordinate
    available = false;
    double mincost = global_mincost; // local mincost
    double h = fi->type->size_y;
    double w = fi->type->size_x;
    double sy = fi->cooy;
    double sx = fi->coox;
    int dw = ceil(w/site_w); // discrete width
    // dx is the discrete index which is closest to the sx 
    int dx = (round((sx-start_x)/site_w) < 0) ? 0 : (round((sx-start_x)/site_w) >= site_num) ? (site_num-1) : round((sx-start_x)/site_w); 



    // preliminary examination: begin
    if(this->height_available(h) == false) return mincost;
    if(this->x_inrange(sx, sx+w) == false){
        if(sx < start_x){
            if(abs(sx - start_x) >= fi->x_allow_dis) return mincost;
            if(abs(sx - start_x) + abs(start_y - sy) >= global_mincost) return mincost;
        }
        else{
            if(abs(start_x + site_num*site_w - w - sx) + abs(start_y - sy) >= global_mincost) return mincost;
        }
    }
    // preliminary examination: end


    for(auto It = space_list.begin(); It!=space_list.end(); It++){
        if((It->second - It->first + 1) >= dw){
            if(It->first > dx){
                if(abs((fi->coox) - (start_x+(It->first*site_w))) > fi->x_allow_dis) break;
                if(available == true){
                    if(abs(best_pos_idx - dx) < abs(It->first - dx)) break;
                }
                if(this->seg_mincost(fi, It->first, It->second, dw, best_pos_idx, mincost, 0, set_constrain) == true){ 
                    available = true;
                    break;
                }
            }
            else if(dx>=It->first && dx<=It->second){
                int de = (dx + dw - 1)<=(It->second) ? (dx + dw - 1):(It->second);
                if(this->seg_mincost(fi, It->first, de, dw, best_pos_idx, mincost, 1, set_constrain) == true){
                    available = true;
                }
                if(this->seg_mincost(fi, dx, It->second, dw, best_pos_idx, mincost, 0, set_constrain) == true){
                    available = true;
                    break;
                }
            }
            else{
                if(abs((fi->coox+fi->type->size_x) - (start_x+(It->second+1)*site_w)) > fi->x_allow_dis) continue;    
                if(this->seg_mincost(fi, It->first, It->second, dw, best_pos_idx, mincost, 1, set_constrain) == true){
                    available = true;
                }
            }
        }
    }

    return mincost;
}


double PlacementRow::closest_x(double x){
    if(x < start_x) return start_x;
    else if(x > (start_x + site_num*site_w)) return (start_x + site_num*site_w);
    else return x;
}

void PlacementRow::FillGap(double min_width){
    int min_w = ceil(min_width/site_w);
    int space_w;

    auto itr = space_list.begin();
    while(itr != space_list.end()){
        space_w = itr->second - itr->first + 1;
        if(space_w < min_w){
            itr = space_list.erase(itr);
        }
        else{
            itr++;
        }
    }
}

int PlacementRow::FillDummy(double width){
    int min_w = ceil(width/site_w);
    int space_w;
    int cnt = 0;
    allDummy.clear();

    auto itr = space_list.begin();
    while(itr != space_list.end()){
        space_w = itr->second - itr->first + 1;
        if(space_w < min_w){
            Dummy* dummyPointer = new Dummy;
            dummyPointer->start = itr->first;
            dummyPointer->end   = itr->second;
            allDummy.push_back(dummyPointer);
            itr = space_list.erase(itr);
            cnt++;
        }
        else{
            itr++;
        }
    }
    return cnt;
}

void PlacementRow::ClearDummy(){
    if(allDummy.empty()) return;

    
    if(space_list.empty()){
        space_list.push_back(pair<int, int>(allDummy.front()->start, allDummy.front()->end));
        delete allDummy.front();
        allDummy.pop_front();
        if(allDummy.empty()) return;
    }
    else if(space_list.front().first > allDummy.front()->end){
        space_list.push_front(pair<int, int>(allDummy.front()->start, allDummy.front()->end));
        delete allDummy.front();
        allDummy.pop_front();
        if(allDummy.empty()) return;
    }

    // now the space list won't be empty 
    // and the "first space" in the space list must be front of the "first dummy"

    auto front_it = space_list.begin();
    auto it = space_list.begin();
    while(allDummy.empty() == false){
        if(it == space_list.end()){
            space_list.insert(it, pair<int, int>(allDummy.front()->start, allDummy.front()->end));    
            delete allDummy.front();
            allDummy.pop_front();
            continue;
        }
        
        
        if(front_it->second < allDummy.front()->start && it->first > allDummy.front()->end){
            space_list.insert(front_it, pair<int, int>(allDummy.front()->start, allDummy.front()->end));    
            delete allDummy.front();
            allDummy.pop_front();
            front_it++;
        }
        else{
            front_it = it;
            it++;
        }
    }

    return;
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

bool placement::row_cmp(PlacementRow* a, PlacementRow* b){
    if(a->start_y == b->start_y) return a->start_x < b->start_x;
    return a->start_y < b->start_y;
}

void placement::addRow(double sx, double sy, double sw, double sh, int sn){
    PlacementRow* pr = new PlacementRow(DIE, sx, sy, sw, sh, sn);
    temp_rows.push_back(pr);
}

void placement::initial(){
    bool is_overlapped;
    bool is_fit;
    list<pair<double, PlacementRow*>> hightlevel_list;

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
            hightlevel_list.push_back(pair<double, PlacementRow*>(pr->start_y + pr->site_h, pr));
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
            }
            i++;
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
            }
            i--;
        }
    }

    return idx;
}


void placement::FillDummy(double min_width){

    // Fill the dummy for each row
    int cnt = 0;
    for(auto r: rows){
        cnt = cnt + r->FillDummy(min_width);
    }
    cout << "Fill Dummy: " << cnt << endl;
    return;
}

void placement::ClearDummy(){
    for(auto r: rows){
        r->ClearDummy();
    }
    return;
}


void placement::GatePlacement(){

    for(auto it: INST->gate_umap){
        gatei* g = it.second;
        placeGateInst(g, g->coox, g->cooy, g->type->size_x, g->type->size_y);
    }

    double min_width = numeric_limits<double>::max();
    for(auto& ffcell_list: LIB->fftable_cost){
        for(auto& type: ffcell_list){
            if(type->size_x < min_width){
                min_width = type->size_x;
            }
        }
    }
    for(auto r: rows){
        r->FillGap(min_width);
    }
    return;
}

void placement::placeGateInst(gatei* g, double x, double y, double w, double h){
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
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    return;
                }
                else if(rows[idx]->x_inrange(xmin, xmax)==true){
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(g, xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(g, xmin, rows[idx]->ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                else if(rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(g, xmin, ymin, rows[idx]->start_x - xmin, h);
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(g, rows[idx]->xmax, ymin, xmax - rows[idx]->xmax, h);
                    }
                    return;
                }
                else{
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(g, xmin, ymin, rows[idx]->start_x - xmin, h);
                        xmin = rows[idx]->start_x;
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(g, rows[idx]->xmax, ymin, xmax - rows[idx]->xmax, h);
                        xmax = rows[idx]->xmax;
                    }
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(g, xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(g, xmin, rows[idx]->ymax, w, ymax - rows[idx]->ymax);
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
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    return;
                }
                else if(rows[idx]->x_inrange(xmin, xmax)==true){
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(g, xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(g, xmin, rows[idx]->ymax, w, ymax - rows[idx]->ymax);
                    }
                    return;
                }
                else if(rows[idx]->y_inrange(ymin, ymax)==true){
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(g, xmin, ymin, rows[idx]->start_x - xmin, h);
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(g, rows[idx]->xmax, ymin, xmax - rows[idx]->xmax, h);
                    }
                    return;
                }
                else{
                    rows[idx]->AddBlockAnyway(xmin, xmax);
                    if(xmin < rows[idx]->start_x){
                        placeGateInst(g, xmin, ymin, rows[idx]->start_x - xmin, h);
                        xmin = rows[idx]->start_x;
                    }
                    if(xmax > rows[idx]->xmax){
                        placeGateInst(g, rows[idx]->xmax, ymin, xmax - rows[idx]->xmax, h);
                        xmax = rows[idx]->xmax;
                    }
                    if(ymin < rows[idx]->start_y){
                        placeGateInst(g, xmin, ymin, w, rows[idx]->start_y - ymin);
                    }
                    if(ymax > rows[idx]->ymax){
                        placeGateInst(g, xmin, rows[idx]->ymax, w, ymax - rows[idx]->ymax);
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

void placement::place_formal(ffi* fi, PlacementRow* best_row, int best_pos_idx){
    double start = best_row->start_x + best_pos_idx*best_row->site_w;
    double end = start + fi->type->size_x;
   
    fi->coox = start;
    fi->cooy = best_row->start_y;
    fi->update_pin_loc();
    best_row->add_ff(start, end, fi->type->size_y);
    return;
}

void placement::PlaceBackFlipFlop(ffi* f){
    PlacementRow* best_row = rows[f->index_to_placement_row];
    int best_pos_idx = f->index_to_site;

    double start = best_row->start_x + best_pos_idx*best_row->site_w;
    double end = start + f->type->size_x;
    best_row->add_ff(start, end, f->type->size_y);
}

bool placement::placeFlipFlop(ffi* f, bool set_constrain, double displace_constrain){
    int idx;
    int closest_idx;
    bool available;
    int best_pos_idx;
    double global_mincost = numeric_limits<double>::max();
    double row_mincost;
    PlacementRow* best_row = NULL;
    int best_site_idx;
    int best_row_idx;
    bool place_success = false;

    int search_row_count = 0;


    closest_idx = closest_IDX(f->coox, f->cooy);
    
    if(set_constrain){
        double displace = (f->type->size_x + f->type->size_y) / 2;
        f->x_allow_dis = (displace)*(displace_constrain/100);
        f->y_allow_dis = (displace)*(displace_constrain/100);
    }
    else{
        f->x_allow_dis = numeric_limits<double>::max();
        f->y_allow_dis = numeric_limits<double>::max();
    }
    

    // find up
    idx = closest_idx;
    while(idx < rows.size()){
        if(rows[idx]->start_y - f->cooy > f->y_allow_dis) break;
        else if(f->cooy - rows[idx]->start_y > f->y_allow_dis) { idx++; continue;}

        search_row_count++;
        
        row_mincost = rows[idx]->place_trial(f, available, best_pos_idx, global_mincost, set_constrain);
        if(available && global_mincost>=row_mincost){
            global_mincost = row_mincost;
            best_site_idx = best_pos_idx;
            best_row = rows[idx];
            best_row_idx = idx;
            place_success = true;
            if(set_constrain == false){
                f->x_allow_dis = global_mincost;
                f->y_allow_dis = global_mincost;
            }
            else if(f->y_allow_dis > global_mincost && f->x_allow_dis > global_mincost){
                f->x_allow_dis = global_mincost;
                f->y_allow_dis = global_mincost;
            }
        }
        idx++;
    }

    // find down
    idx = closest_idx - 1;
    while(idx >= 0){
        if(f->cooy - rows[idx]->start_y > f->y_allow_dis) break;
        else if(rows[idx]->start_y - f->cooy > f->y_allow_dis) { idx--; continue;}

        search_row_count++;
        row_mincost = rows[idx]->place_trial(f, available, best_pos_idx, global_mincost, set_constrain);
        if(available && global_mincost>row_mincost){
            global_mincost = row_mincost;
            best_site_idx = best_pos_idx;
            best_row = rows[idx];
            best_row_idx = idx;
            place_success = true;
            if(set_constrain == false){
                f->x_allow_dis = global_mincost;
                f->y_allow_dis = global_mincost;
            }
            else if(f->y_allow_dis > global_mincost && f->x_allow_dis > global_mincost){
                f->x_allow_dis = global_mincost;
                f->y_allow_dis = global_mincost;
            }
            if(global_mincost == 0) break;
        }
        idx--;
    }

    if(place_success){
        place_formal(f, best_row, best_site_idx);
        f->index_to_placement_row = best_row_idx;
        f->index_to_site = best_site_idx;
    }

    return place_success;
}

void placement::DeleteFlipFlop(ffi* f){
    rows[f->index_to_placement_row]->delete_ff(f->coox, f->coox+f->type->size_x, f->type->size_y);
}
