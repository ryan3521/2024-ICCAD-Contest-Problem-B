#include "placement.h"

// Constructor
void plcmt_row::plcmt_row(double sx, double sy, double sw, double sh, int sn){
    start_x = sx;
    start_y = sy;
    site_w = sw;
    site_h = sh;
    site_num = sn;
    glist.clear();
}

bool plcmt_row::cmp_g(gatei* a, gatei* b){
    return a->coox < b->coox;
}

void plcmt_row::sort_gate(){glist.sort(cmp_g); return;}

void placement::placement(){
    temp_rows.clear();
    rows.clear();
    rs_list.clear();
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
    temp_rows.sort(row_cmp);
    rows.reserve(temp_rows.size());

    // int idx_cnt = 0;
    // double pre_hight = -1;
    // double cur_hight = 0;
    // row_sec* rs = NULL;
    for(auto& pr: temp_rows){
        // cur_hight = pr->site_h;
        // if(cur_hight != pre_hight){
        //     if(rs == NULL){
        //         rs = new row_sec;
        //     }
        //     else{
        //         rs->end = pr->start_y;
        //         rs = new row_sec;
        //     }
        //     rs->start_idx = idx_cnt;
        //     rs->start = pr->start_y;
        //     rs->hight = cur_hight;
        // }
        rows.push_back(pr);
        // idx_cnt++;
    }
    // rs->end = rs->start + (idx_cnt - rs->start_idx + 1)*rs->hight;


    return;
}

void placement::placeGateInst(inst& INST){
    for(auto& gi: INST.gate_umap){
        double y = gi->cooy;
        double x = gi->coox;
        int idx = (y - rows[0]->start_y)/rows[0]->site_h;

        rows[idx]->glist.push_back(gi);

        double temp = (x - rows[idx]->start_x)/rows[idx]->site_w;
        if(y != rows[idx]->start_y || abs(temp - int(temp)) > 0.0000001){
            cout << "Error: gate instance is not on site." << endl;
        }
    }

    for(auto& r: rows){
        r.sort_gate();
    }
}