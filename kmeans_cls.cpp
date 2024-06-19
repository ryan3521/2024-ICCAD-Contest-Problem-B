#include "func.h"

void InitialCenter(lib& LIB, inst& INST, list<cluster*>& KCR){
    cluster* cptr; // cluster pointer
    int K = ceil((double)INST.ff_num/(double)LIB.max_ff_size);

    // Generate K's centroid (K-means++)
    // First Centroid
    int R = rand()%(INST.ff_umap.size())+1 << endl; //1~5
    int cnt = 1;
    for(const auto& n : INST.ff_umap){
        if(cnt == R){
            cptr = new cluster(n.second->cen_x, n.second->cen_y);
            KCR.push_back(cptr);
            break;
        }
    }

    // Other Centroids
    ffi* max_hpwl_ff = NULL;
    double max_hpwl = 0;
    double cur_hpwl = 0;
    for(int i=1; i<K; i++){
        for(const auto& n : INST.ff_umap){
            cur_hpwl = 0;
            for(cluster* c : KCR){
                cur_hpwl = cur_hpwl + abs(c->cen_x - n->cen_x) + abs(c->cen_y - n->cen_y);
            }
            if(cur_hpwl > max_hpwl){
                max_hpwl = cur_hpwl;
                max_hpwl_ff = n;
            }
        }
        cptr = new cluster(max_hpwl_ff->cen_x, max_hpwl_ff->cen_y);
        KCR.push_back(cptr);
    }
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
            hpwl_diff = abs(c->cen_x - f->cen_x) + abs(c->cen_y - f->cen_y);
            if((hpwl_diff <= f->allow_displace) && ((c->size)+(f->type->bit_num) <= LIB.max_ff_size)){
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
        if(find){ // If find, meaning this ff is belong to one cluster
           closest_c->size =  closest_c->size + f->type->bit_num;
           closest_c->member_list.push_back(f);
        }
        else{// if not find, this ff is a non cluster instance.
            NCLS.push_back(f);
        }
    }
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
    int ITR_BOUND = 10;
    int itr = 0;
    bool no_move;
    KCR.clear();
    InitialCenter(LIB, INST, KCR);

    while(itr < ITR_BOUND){
        DoCluster(LIB, INST, KCR, NCLS);
        no_move = UpdateCentroid(KCR);
        if(no_move){
            break;
        }
        itr++;
    }
    return;
}