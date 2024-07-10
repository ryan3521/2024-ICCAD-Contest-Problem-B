#include "cluster.h"


cluster::cluster(double x, double y){
    cen_x = x;
    cen_y = y;
    size  = 0;
}

bool cluster::updateCentroid(){
    double TH = 5; // (%)
    bool move = true;
    double new_cen_x = 0;
    double new_cen_y = 0;
    
    for(ffi* m : member_list){
        new_cen_x = new_cen_x + m->cen_x*(m->type->bit_num);
        new_cen_y = new_cen_y + m->cen_y*(m->type->bit_num);
    }

    new_cen_x = new_cen_x/size;
    new_cen_y = new_cen_y/size;

    if(((new_cen_x+new_cen_y)-(cen_x+cen_y))/(cen_x+cen_y) < (TH/100)) {
        move = false;
    }

    
    cen_x = new_cen_x;
    cen_y = new_cen_y;

    return move;
}

void cluster::clearMemberList(){ member_list.clear(); size = 0;}