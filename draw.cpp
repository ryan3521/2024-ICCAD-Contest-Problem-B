#include "func.h"

void DrawFFs(dieInfo& DIE, lib& LIB, inst& INST){
    // Format:
    // --------------
    // Die.W Die.H
    // FF num
    // Name X Y W H 
    // Name X Y W H 
    // Name X Y W H 
    // .
    // .
    // .
    // --------------

    fstream fout;
    fout.open("./Draw/ff.txt", ios::out);

    fout << DIE.die_width << " " << DIE.die_height << endl;

    int ffcnt = 0;
    for(auto flist: INST.ffs_sing){
        ffcnt = ffcnt + flist->size();
    } 
    fout << ffcnt << endl;

    for(auto flist: INST.ffs_sing){
        for(auto f: *flist){
            fout << f->name << " " << f->coox << " " << f->cooy << " ";
            fout << f->type->size_x << " " << f->type->size_y << " ";
            fout << endl;
        }
    } 


    fout.close();
    return;
}

void DrawGates(dieInfo& DIE, lib& LIB, inst& INST){
    // Format:
    // ----------------------
    // Die.width Die.height
    // GateNum
    // G1.name coox cooy size_x size_y
    // G2.name coox cooy size_x size_y
    // G3.name coox cooy size_x size_y
    // . 
    // . 
    // . 
    // ----------------------

    fstream fout;
    fout.open("./Draw/gate.txt", ios::out);

    fout << DIE.die_width << " " << DIE.die_height << endl;

    int gcnt = INST.gate_umap.size();
    fout << gcnt << endl;

    for(auto itr: INST.gate_umap){
        auto g = itr.second;
        fout << g->name << " " << g->coox << " " << g->cooy << " ";
        fout << g->type->size_x << " " << g->type->size_y << " ";
        fout << endl;
    }
    

    fout.close();
    return;
}