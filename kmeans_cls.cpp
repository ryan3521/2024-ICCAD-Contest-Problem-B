#include "func.h"

bool cmp_cx(cluster* a, cluster* b){
    return a->cen_x < b->cen_x;
}

bool cmp_ffx(ffi* a, ffi* b){
    return a->coox < b->coox;
}

void InitialCenter(lib& LIB, inst& INST, list<ffi*>& ff_list , vector<cluster*>& kcr){
    int cnt = 0;
    cluster* cptr; // cluster pointer
    
    ff_list.clear();
    for(auto& it: INST.ff_umap){
        ff_list.push_back(it.second);
    }
    ff_list.sort(cmp_ffx);

    kcr.clear();
    kcr.reserve(ceil(ff_list.size()/LIB.max_ff_size));

    for(auto& fi: ff_list){
        if(cnt == LIB.max_ff_size-1){
            cnt = 0;
            cptr = new cluster(fi->cen_x, fi->cen_y);
            kcr.push_back(cptr);
        }
        else{
            cnt++;
        }
    }



    return;
}

void DoCluster(double dis_delay, lib& LIB, inst& INST, vector<int>& map_to_cluster, list<ffi*>& ff_list, vector<cluster*>& kcr, list<ffi*>& NCLS, int itr){
    cluster* closest_c; // closest cluster
    cluster* c;
    double min_hpwl;
    bool find;
    double hpwl_diff;
    bool find_up = true;
    bool find_down = true;
    int closest_idx;
    int ff_cnt = 0;
    int up_i;
    int down_i;

    for(const auto& c : kcr){ c->clearMemberList(); }
    NCLS.clear();
    sort.(kcr.begin(), kcr.end(), cmp_cx);

    if(itr == 0){
        // initial "ff map to the closest cluster"
        for(int i=0; i<map_to_cluster.size(); i++){
            map_to_cluster[i] = (double)i/(double)LIB.max_ff_size;
        }
    }


    for(const auto& f : ff_list){
        find      = false;
        closest_c = NULL;
        find_up   = true;
        find_down = true;
        closest_idx = map_to_cluster[ff_cnt];
        up_i = closest_idx;
        down_i = closest_idx - 1;

        while(find_up==true || find_down==true){
            if(down_i < 0) find_down = false;
            if(up_i >= kcr.size()) find_up = false;

            // Find UP
            c = kcr[up_i];
            if(find_up){
                if(find == true){
                    if(min_hpwl <= c->cen_x - f->cen_x){
                        find_up = false;
                    }
                }
            }
            if(find_up){
                if(f->allow_displace(c->cen_x, f->cen_y, dis_delay)==false && c->cen_x >= f->cen_x){
                    find_up = false;
                }
            }
            if(find_up){
                hpwl_diff = abs(c->cen_x - f->cen_x) + abs(c->cen_y - f->cen_y);
                if(f->allow_displace(c->cen_x, c->cen_y, dis_delay)==true && ((c->size)+(f->type->bit_num) <= LIB.max_ff_size)){
                    if(find == false){
                        find = true;
                        min_hpwl = hpwl_diff;
                        closest_c = c;
                    }
                    else{
                        if(hpwl_diff < min_hpwl){
                            min_hpwl = hpwl_diff;
                            closest_c = c;
                        }
                    }
                }
            }

            if(find_down){

            }




        }
        // cout << "MIN HPWL DIFF: " << min_hpwl << " , ALLOW DIFF: " << f->allow_displace << endl;
        if(find){ // If find, meaning this ff is belong to one cluster
           closest_c->size =  closest_c->size + f->type->bit_num;
           closest_c->member_list.push_back(f);
        }
        else{// if not find, this ff is a non cluster instance.
            NCLS.push_back(f);
        }
        ff_cnt++;
    }
    cout << "Clustered FFs   : " << INST.ff_umap.size() - NCLS.size() << endl;
    cout << "None cluster FFs: " << NCLS.size() << endl;
}

bool UpdateCentroid(list<cluster*>& KCR){
    bool move;
    bool no_move = true;
    for(const auto& c : KCR){
        move = c->updateCentroid();
        if(move) no_move = false;
    }
    return no_move;
}


void KmeansCls(lib& LIB, inst& INST, list<cluster*>& KCR, list<ffi*>& NCLS){
    cout << "K means cluster >>>" << endl;

    int ITR_BOUND = 1;
    int itr = 0;
    bool no_move;
    vector<int> map_to_cluster;
    list<ffi*> ff_list;


    KCR.clear();
    cout << "Initializing center ..." << endl;
    InitialCenter(LIB, INST, ff_list, KCR);

    map_to_cluster.clear();
    map_to_cluster.resize(ff_list.size());


    while(itr < ITR_BOUND){
        cout << "Clustering (Itr" << itr << ") ..." << endl;
        
        DoCluster(LIB, INST, map_to_cluster, ff_list, KCR, NCLS, itr);
        no_move = UpdateCentroid(KCR);
        if(no_move){
            break;
        }
        itr++;
    }
    return;
}