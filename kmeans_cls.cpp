#include "func.h"

bool cmp(ffi* a, ffi* b){
    return a->coox < b->coox;
}

void InitialCenter(lib& LIB, inst& INST, list<cluster*>& KCR){
    int cnt = 0;
    cluster* cptr; // cluster pointer
    list<ffi*> ff_list;
    
    for(auto& it: INST.ff_umap){
        ff_list.push_back(it.second);
    }
    ff_list.sort(cmp);

    int neg_cnt = 0;
    for(auto& fi: ff_list){
        //if(fi->d_pins[0]->slack <= 0) neg_cnt++;
        if(fi->allow_dis <= 0) neg_cnt++;
    }
    cout << "Negative Slack FF Num: " << neg_cnt << endl;






    for(auto& fi: ff_list){
        if(cnt == LIB.max_ff_size-1){
            cnt = 0;
            cptr = new cluster(fi->cen_x, fi->cen_y);
            // cout << "CLS cen: " << fi->cen_x << " " << fi->cen_y << endl;
            KCR.push_back(cptr);
        }
        else{
            cnt++;
        }
    }


    // int K = ceil((double)INST.ff_num/(double)LIB.max_ff_size);

    // Generate K's centroid (K-means++)
    // First Centroid
    // int R = rand()%(INST.ff_umap.size())+1; //1~5
    // int cnt = 1;
    // for(const auto& n : INST.ff_umap){
    //     if(cnt == R){
    //         cptr = new cluster(n.second->cen_x, n.second->cen_y);
    //         KCR.push_back(cptr);
           
    //         break;
    //     }
    //     cnt++;
    // }

    // Other Centroids
    // ffi* max_hpwl_ff = NULL;
    // double max_hpwl = 0;
    // double cur_hpwl = 0;
    // for(int i=1; i<K; i++){
    //     for(const auto& n : INST.ff_umap){
    //         cur_hpwl = 0;
    //         for(cluster* c : KCR){
    //             cur_hpwl = cur_hpwl + abs(c->cen_x - n.second->cen_x) + abs(c->cen_y - n.second->cen_y);
    //         }
    //         if(cur_hpwl > max_hpwl){
    //             max_hpwl = cur_hpwl;
    //             max_hpwl_ff = n.second;
    //         }
    //     }
    //     cptr = new cluster(max_hpwl_ff->cen_x, max_hpwl_ff->cen_y);
    //     KCR.push_back(cptr);
    // }
    return;
}

void DoCluster(lib& LIB, inst& INST, list<cluster*>& KCR, list<ffi*>& NCLS){
    cluster* closest_c; // closest cluster
    double min_hpwl;
    bool find;
    double hpwl_diff;

    for(const auto& c : KCR){ c->clearMemberList(); }
    NCLS.clear();


    for(const auto& f : INST.ff_umap){
        find = false;
        closest_c = NULL;
        for(const auto& c : KCR){
            hpwl_diff = abs(c->cen_x - f.second->cen_x) + abs(c->cen_y - f.second->cen_y);
            //if((hpwl_diff <= f.second->allow_dis) /*&& ((c->size)+(f.second->type->bit_num) <= LIB.max_ff_size)*/){
            if(f.second->allow_displace(c->cen_x, c->cen_y, 0.01)==true && ((c->size)+(f.second->type->bit_num) <= LIB.max_ff_size)){
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
        // cout << "MIN HPWL DIFF: " << min_hpwl << " , ALLOW DIFF: " << f.second->allow_displace << endl;
        if(find){ // If find, meaning this ff is belong to one cluster
           closest_c->size =  closest_c->size + f.second->type->bit_num;
           closest_c->member_list.push_back(f.second);
        }
        else{// if not find, this ff is a non cluster instance.
            NCLS.push_back(f.second);
        }
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
    KCR.clear();
    cout << "Initializing center ..." << endl;
    InitialCenter(LIB, INST, KCR);

    while(itr < ITR_BOUND){
        cout << "Clustering (Itr" << itr << ") ..." << endl;
        
        DoCluster(LIB, INST, KCR, NCLS);
        no_move = UpdateCentroid(KCR);
        if(no_move){
            break;
        }
        itr++;
    }
    return;
}