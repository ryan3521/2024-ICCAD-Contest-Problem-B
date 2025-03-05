#include "placement.h"

// *******************************************************************
// *                        CLASS PlacementRow                          *
// *******************************************************************

// Constructor
PlacementRow::PlacementRow(double sx, double sy, double sw, double sh, int sn){
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
    upRow = NULL;
    downRow = NULL;
    rightRow = NULL;
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


bool PlacementRow::height_available(double height){
    if(height <= site_h) return true;
    else {
        if(upRow != NULL){
            if(upRow->height_available(height - site_h)) return true;
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
        if(upRow != NULL){
            upRow->add_ff(start, end, height - site_h);    
        }    
    }

    return;
}

void PlacementRow::delete_ff(double start, double end, double height){
    this->delete_fblock(start, end);

    if(height > site_h){
        if(upRow != NULL){
            upRow->delete_ff(start, end, height - site_h); 
        }    
    }

    return;
}

bool PlacementRow::FindSpaceOrJump(double idealcoox, double idealcooy, double globalMincost, double askCoox, double askCooy, double& replyCoox, double width, double height, bool direction, dieInfo& DIE, bool print){
    bool find = false;
    bool haveSpace = false;

    if(direction){
        // if(askCoox == xmax - site_w) return false;

        auto space_itr = space_list.begin();
        while(space_itr != space_list.end()){
            double spaceStart = start_x + space_itr->first*site_w;
            double spaceEnd   = start_x + (space_itr->second+1)*(site_w);
            double spaceWidth = spaceStart - spaceEnd;
            double displacement = abs(askCoox - idealcoox) + abs(askCooy - idealcooy);
            
            if(displacement >= globalMincost){
                return false;
            }

            auto next_itr = space_itr;
            next_itr++;

            if(next_itr == space_list.end()){
                if(spaceEnd == xmax){
                    if(rightRow != NULL){
                        if(rightRow->space_list.front().first == 0 && rightRow->space_list.empty()==false){
                            spaceEnd = rightRow->start_x + (site_w)*(rightRow->space_list.front().second + 1);
                        }
                    }
                    else{
                        spaceEnd = xmax + floor((DIE.die_width - xmax)/site_w)*site_w;
                    }
                }
            }

            haveSpace = false;

            if(askCoox >= spaceEnd){
                space_itr++;
                continue;
            }
            else if(spaceStart > askCoox){
                find = false;
                askCoox = spaceStart;
                continue;
            }
            else if(spaceEnd - askCoox >= width){
                haveSpace = true;            
            }
            
            if(haveSpace){
                if(find){
                    replyCoox = askCoox;
                    return true;
                }
                if(height-site_h > 0){
                    if(upRow == NULL){
                        replyCoox = askCoox;
                        return true;
                    }
                    else if(upRow->FindSpaceOrJump(idealcoox, idealcooy, globalMincost, askCoox, askCooy, replyCoox, width, height - site_h, direction, DIE, print)){
                        if(replyCoox == askCoox){
                            return true;
                        }
                        else{
                            askCoox = replyCoox;
                            find = true;
                            continue;
                        }
                    }
                    else{
                        return false;
                    }
                }
                else{
                    replyCoox = askCoox;
                    return true;
                }
            }
            else{
                space_itr++;
                continue;
            }
        }
        return false;
    }
    else{
        // if(askCoox == start_x) return false;
        if(print) cout << "Find row" << idx << " space, askCoox = " << askCoox << endl;
        auto space_itr = space_list.rbegin();
        while(space_itr != space_list.rend()){
            double spaceStart = start_x + space_itr->first*site_w;
            double spaceEnd   = start_x + (space_itr->second+1)*(site_w);
            double spaceWidth = spaceStart - spaceEnd;
            double displacement = abs(askCoox - idealcoox) + abs(askCooy - idealcooy);
            
            if(displacement >= globalMincost){
                return false;
            }

            if(space_itr == space_list.rbegin()){
                if(spaceEnd == xmax){
                    if(rightRow != NULL){
                        if(rightRow->space_list.front().first == 0 && rightRow->space_list.empty()==false){
                            spaceEnd = rightRow->start_x + (site_w)*(rightRow->space_list.front().second + 1);
                        }
                    }
                    else{
                        spaceEnd = xmax + floor((DIE.die_width - xmax)/site_w)*site_w;
                    }
                }
            }

            auto next_itr = space_itr;
            next_itr++;
            haveSpace = false;

            if(askCoox+width <= spaceStart){
                if(print){
                    cout << "1" << endl;
                }
                space_itr++;
                continue;
            }
            else if(spaceEnd < askCoox+width){
                if(print){
                    cout << "2" << endl;
                }
                find = false;
                askCoox = spaceEnd - ceil(width/site_w)*site_w;
                continue;
            }
            else if(spaceStart <= askCoox){
                if(print){
                    cout << "3: ask Coox = " << askCoox << endl;
                }
                haveSpace = true;
            }


            if(haveSpace){

                if(find){
                    replyCoox = askCoox;
                    if(print) cout << "return true" << endl;
                    return true;
                }
                if(height-site_h > 0){
                    if(upRow == NULL){
                        replyCoox = askCoox;
                        if(print) cout << "return true" << endl;
                        return true;
                    }
                    else if(upRow->FindSpaceOrJump(idealcoox, idealcooy, globalMincost, askCoox, askCooy, replyCoox, width, height - site_h, direction, DIE, print)){
                        if(print) cout << "Back to row" << idx << ", Reply coox = " << replyCoox << endl;
                        if(replyCoox == askCoox){
                            if(print) cout << "return true" << endl;
                            return true;
                        }
                        else{
                            askCoox = replyCoox;
                            find = true;
                            continue;
                        }
                    }
                    else{
                        if(print) cout << "return false" << endl;
                        return false;
                    }
                }
                else{
                    replyCoox = askCoox;
                    return true;
                }
            }
            else{
                space_itr++;
                continue;
            }
        }
        return false;
    }
}


void PlacementRow::PlaceTrial(ffi* f, int& bestRowIndex, int& bestSiteIndex, double& globalMincost, dieInfo& DIE){
    double idealcoox = f->coox;
    double idealcooy = f->cooy;
    double replyCoox;
    double width  = f->type->size_x;
    double height = f->type->size_y;
    bool direction;
    



    // the askCoox should be an on site value
    double askCoox = start_x + floor((idealcoox-start_x)/site_w)*site_w;
    double askCooy = start_y; 

    if(askCoox > xmax - site_w) askCoox = xmax - site_w;
    if(askCoox < start_x) askCoox = start_x;

    // if(start_x == 1308150 && (start_y>=223200 && start_y<247200) && f->name == "NEWFF21213"){
    //     cout << "row index " << idx  << endl;
    //     askCoox = 1332660;
    // }
    
    direction = 0;
    bool print = false;

    // if(start_x == 1308150 && (start_y>=223200 && start_y<247200) && f->name == "NEWFF21213"){
    //     if(idx == 6) print = true;
    // }



    if(this->FindSpaceOrJump(idealcoox, idealcooy, globalMincost, askCoox, askCooy, replyCoox, width, height, direction, DIE, print)){
        globalMincost = abs(idealcoox - replyCoox) + abs(idealcooy - start_y);
        // globalMincost = sqrt(pow((idealcoox - replyCoox),2) + pow((idealcooy - start_y),2));
        bestRowIndex = idx;
        bestSiteIndex = (replyCoox - start_x)/site_w;
        if(replyCoox > xmax) cout << "Error OUT OF RANGE (OVER)" << endl;
        if(replyCoox < start_x) cout << "Error OUT OF RANGE (BELOW)" << endl;
        // if(start_x == 1308150 && (start_y>=223200 && start_y<247200) && f->size == 4){
        //     if(globalMincost != numeric_limits<double>::max()){
        //         cout << f->name << " find: " << f->type->size_x << endl;
        //         cout << fixed << "askedcoox " << askCoox << endl;
        //         cout << fixed << "idealcoox " << idealcoox << endl;
        //         cout << fixed << "replycoox " << replyCoox << endl;
        //     }
        // }
    }
    
    print = false;
    direction = 1;
    if(this->FindSpaceOrJump(idealcoox, idealcooy, globalMincost, askCoox, askCooy, replyCoox, width, height, direction, DIE, print)){
        globalMincost = abs(idealcoox - replyCoox) + abs(idealcooy - start_y);
        // globalMincost = sqrt(pow((idealcoox - replyCoox),2) + pow((idealcooy - start_y),2));
        bestRowIndex = idx;
        bestSiteIndex = (replyCoox - start_x)/site_w;
        if(replyCoox > xmax) cout << "Error OUT OF RANGE (OVER)" << endl;
        if(replyCoox < start_x) cout << "Error OUT OF RANGE (BELOW)" << endl;
        // if(start_x == 1308150 && (start_y>=223200 && start_y<247200) && f->size == 4){
        //     if(globalMincost != numeric_limits<double>::max()){
        //         cout <<f->name << " find" << endl;
    
        //     }
        // }
    }

    return;
}



void PlacementRow::FillGap(double gapWidth){
    int min_w = ceil(gapWidth/site_w);
    int space_w;

    auto itr = space_list.begin();
    while(itr != space_list.end()){
        auto itr_next = itr;
        itr_next++;

        if(itr == space_list.begin() || itr_next == space_list.end()){
            itr++;
            continue;
        }

        space_w = itr->second - itr->first + 1;
        if(space_w < min_w){
            itr = space_list.erase(itr);
        }
        else{
            itr++;
        }
    }
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
    PlacementRow* pr = new PlacementRow(sx, sy, sw, sh, sn);
    temp_rows.push_back(pr);
}

void placement::initial(){
    temp_rows.sort(row_cmp);
    rows.reserve(temp_rows.size());

    PlacementRow* prevRow = NULL;
    for(auto currRow: temp_rows){
        rows.push_back(currRow);
        if(prevRow != NULL){
            prevRow->upRow   = currRow;
            currRow->downRow = prevRow;
        }
        prevRow = currRow;
    }

    for(int i=0; i<rows.size(); i++) { rows[i]->idx = i; }

    return;
}
