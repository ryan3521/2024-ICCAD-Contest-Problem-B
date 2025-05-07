#include "func.h"

void DrawFFs(dieInfo& DIE, lib& LIB, inst& INST, list<ffi*>& UPFFS, list<ffi*>& PFFS){
    fstream fout;

// ==============================================================================================
    fout.open("./Draw/ff_original.txt", ios::out);
    // Format: Original FFs
    // --------------
    // Die.W Die.H Bin.W Bin.H
    // FF num
    // Name Size X Y W H 
    // Name Size X Y W H 
    // Name Size X Y W H 
    // .
    // .
    // .
    // --------------

    fout << DIE.die_width << " " << DIE.die_height << " " << DIE.bin_width << " " << DIE.bin_height << endl;

    int ffcnt = INST.ff_umap.size();
    fout << ffcnt << endl;

    for(auto itr: INST.ff_umap){
        auto f = itr.second;
        fout << f->name << " " << f->type->bit_num << " " << f->coox << " " << f->cooy << " ";
        fout << f->type->size_x << " " << f->type->size_y << " ";
        fout << endl;
    } 

    fout.close();

// ==============================================================================================
 

// ==============================================================================================
    fout.open("./Draw/placement result.txt", ios::out);
    // Format:  (After Placing)
    // --------------
    // Die.W Die.H
    // FF num
    // FF Size X Y W H cenx ceny ori_cenx ori_ceny
    // FF Size X Y W H cenx ceny ori_cenx ori_ceny 
    // FF Size X Y W H cenx ceny ori_cenx ori_ceny 
    // .
    // .
    // .
    // --------------

    fout << DIE.die_width << " " << DIE.die_height << " " << DIE.bin_width << " " << DIE.bin_height << endl;


    fout << PFFS.size() << endl;

    for(auto f: PFFS){
        double coox = f->coox;
        double cooy = f->cooy;

        fout << f->name << " " << f->type->bit_num << " " << f->coox << " " << f->cooy << " ";
        fout << f->type->size_x << " " << f->type->size_y << " ";
        fout << f->coox + f->type->size_x/2 << " " << f->cooy + f->type->size_y/2 << " ";
        f->update_coor();
        fout << f->cen_x << " " << f->cen_y << " ";
        fout << endl;
        f->coox = coox;
        f->cooy = cooy;
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

    fout << DIE.die_width << " " << DIE.die_height << " " << DIE.bin_width << " " << DIE.bin_height << endl;


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