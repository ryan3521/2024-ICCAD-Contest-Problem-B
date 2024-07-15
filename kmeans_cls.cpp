#include "func.h"


bool cmp_cx(cluster* a, cluster* b){
    return a->cen_x < b->cen_x;
}

bool cmp_ffx(ffi* a, ffi* b){
    return a->cen_x < b->cen_y;
}

void InitialCenter(lib& LIB, inst& INST, list<ffi*>& ff_list , vector<cluster*>& kcr){
    int cnt = 0;
    cluster* cptr; // cluster pointer
    
    // ff_list.clear();
    // for(auto& it: INST.ff_umap){
    //     ff_list.push_back(it.second);
    // }
    ff_list.sort(cmp_ffx);

    kcr.clear();
    kcr.reserve(ceil(ff_list.size()/LIB.max_ff_size));

    for(auto& fi: ff_list){
        if(cnt == 0){
        // if(cnt == LIB.max_ff_size-1){
            cptr = new cluster(fi->cen_x, fi->cen_y);
            kcr.push_back(cptr);
        }
        if(cnt == LIB.max_ff_size-1){
            cnt = 0;
        }
        else{
            cnt++;
        }
    }



    return;
}

void DoCluster(double dis_delay, lib& LIB, inst& INST, vector<int>& map_to_cluster, list<ffi*>& ff_list, vector<cluster*>& kcr, list<ffi*>& ncls, int itr){
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
    ncls.clear();
    sort(kcr.begin(), kcr.end(), cmp_cx);

    if(itr == 0){
        // initial "ff map to the closest cluster"
        for(int i=0; i<map_to_cluster.size(); i++){
            map_to_cluster[i] = (double)i/(double)LIB.max_ff_size;
        }
    }
    int t_cnt = 0;
    int f_cnt = 0;

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
                hpwl_diff = abs(c->cen_x - f->cen_x) + abs(c->cen_y - f->cen_y);
                //if(f->allow_displace(c->cen_x, c->cen_y, dis_delay)==true && ((c->size)+(f->type->bit_num) <= LIB.max_ff_size)){
                if(((c->size)+(f->type->bit_num) <= LIB.max_ff_size)){
                    if(find == false){
                        find = true;
                        min_hpwl = hpwl_diff;
                        closest_c = c;
                        map_to_cluster[ff_cnt] = up_i; 
                        //break;
                    }
                    else{
                        if(hpwl_diff < min_hpwl){
                            min_hpwl = hpwl_diff;
                            closest_c = c;
                            map_to_cluster[ff_cnt] = up_i; 
                        }
                    }
                }
                else f_cnt++;
                up_i++;
            }

            // Find DOWN
            c = kcr[down_i];
            if(find_down){
                if(find == true){
                    if(min_hpwl <=  f->cen_x - c->cen_x){
                        find_down = false;
                    }
                }
            }
            if(find_down){
                hpwl_diff = abs(c->cen_x - f->cen_x) + abs(c->cen_y - f->cen_y);
                //if(f->allow_displace(c->cen_x, c->cen_y, dis_delay)==true && ((c->size)+(f->type->bit_num) <= LIB.max_ff_size)){
                if((c->size)+(f->type->bit_num) <= LIB.max_ff_size){
                    if(find == false){
                        find = true;
                        min_hpwl = hpwl_diff;
                        closest_c = c;
                        map_to_cluster[ff_cnt] = down_i; 
                        // break;
                    }
                    else{
                        if(hpwl_diff < min_hpwl){
                            min_hpwl = hpwl_diff;
                            closest_c = c;
                            map_to_cluster[ff_cnt] = down_i; 
                        }
                    }
                }
                else f_cnt++;
                down_i--;
            }

        }

        if(find){ // If find, meaning this ff is belong to one cluster
           closest_c->size =  closest_c->size + f->type->bit_num;
           closest_c->member_list.push_back(f);
        }
        else{// if not find, this ff is a non cluster instance.
            ncls.push_back(f);
        }
        
        ff_cnt++;
    }
}

bool UpdateCentroid(vector<cluster*>& kcr, list<ffi*>& ncls){
    bool move;
    bool no_move = true;

    for(const auto& c : kcr){
        if(c->size > 1){
            move = c->updateCentroid();
            if(move) no_move = false;
        }
        else if(c->size == 1){
            ncls.push_back(c->member_list.front());
            c->member_list.clear();
            c->size = 0;
        }
    }
    return no_move;
}

static bool cmp_avail_num(pair<int, ffi*>* a, pair<int, ffi*>* b){
    return a->first < b->first;
}

void AssignFFtoCls(lib& LIB, list<ffi*>& ff_list, list<ffi*>& ncls, vector<cluster*>& kcr, double dis_delay){
    bool find;
    list<pair<int, ffi*>* > fflist_pair;

    ncls.clear();
    for(const auto& c : kcr){ c->clearMemberList(); }

    for(auto fi: ff_list){
        pair<int, ffi*>* p = new pair<int, ffi*>(0, fi);
        fflist_pair.push_back(p);
    }

    for(auto p: fflist_pair){
        for(auto c: kcr){
            if(p->second->allow_displace(c->cen_x, c->cen_y, dis_delay)==true){
                p->first++;
            }
        }
    }

    fflist_pair.sort(cmp_avail_num);

    for(auto p: fflist_pair){
        find = false;
        for(auto c: kcr){
            if(p->second->allow_displace(c->cen_x, c->cen_y, dis_delay)==true && ((c->size)+(p->second->type->bit_num) <= LIB.max_ff_size)){
                c->size =  c->size + p->second->type->bit_num;
                c->member_list.push_back(p->second);
                find = true;
                break;
            }
        }
        if(!find){
            ncls.push_back(p->second);
        }
    }
    return;
}

void KmeansCls(dieInfo& DIE, lib& LIB, inst& INST, list<cluster*>& KCR, list<ffi*>& NCLS){
    bool PRINT_INFO = true;
    if(PRINT_INFO) cout << endl << "K means cluster >>>" << endl;



    int ITR_BOUND = 5;
    int itr = 0;
    bool no_move;
    vector<cluster*> kcr;
    list<ffi*> ncls;
    vector<int> map_to_cluster;
    list<ffi*> ff_list;

    for(auto itr: INST.ff_umap) ff_list.push_back(itr.second);


    KCR.clear();

    for(auto& clock_group_list: INST.ffs_sing){

        InitialCenter(LIB, INST, *clock_group_list, kcr);
        //InitialCenter(LIB, INST, ff_list, kcr);
        
        itr = 0;
        map_to_cluster.clear();
        map_to_cluster.resize(clock_group_list->size());
        //map_to_cluster.resize(ff_list.size());


        while(itr < ITR_BOUND){
            
            DoCluster(DIE.displacement_delay, LIB, INST, map_to_cluster, *clock_group_list, kcr, ncls, itr);
            //DoCluster(DIE.displacement_delay, LIB, INST, map_to_cluster, ff_list, kcr, ncls, itr);
            no_move = UpdateCentroid(kcr, ncls);
            if(no_move){
                break;
            }
            itr++;
        }

        AssignFFtoCls(LIB, *clock_group_list, ncls, kcr, DIE.displacement_delay);
        //AssignFFtoCls(LIB, ff_list, ncls, kcr, DIE.displacement_delay);

        for(auto&  c: kcr){
            KCR.push_back(c);
        }
        for(auto&  fi: ncls){
            NCLS.push_back(fi);
        }
    }

    int clsffcnt = 0;
    for(auto& c: KCR){
        clsffcnt = clsffcnt + c->size;
    }
    if(PRINT_INFO){
        cout << "Clk net numbers: " << INST.ffs_sing.size() << endl;
        cout << "-------------------------------------------" << endl;
        cout << "K-mean Cluster Done: " << endl;
        cout << "Clustered FFs:     " << clsffcnt << endl;
        cout << "Non-Clustered FFs: " << NCLS.size() << endl;
        cout << "-------------------------------------------" << endl;
    }

    return;
}

