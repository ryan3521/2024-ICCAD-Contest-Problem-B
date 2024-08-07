#include "func.h"

void DrawFFs(dieInfo& DIE, lib& LIB, inst& INST, list<ffi*>& UPFFS){
    fstream fout;

// ==============================================================================================
    fout.open("./Draw/ff_original.txt", ios::out);
    // Format: Original FFs
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

    fout << DIE.die_width << " " << DIE.die_height << endl;

    int ffcnt = INST.ff_umap.size();
    fout << ffcnt << endl;

    for(auto itr: INST.ff_umap){
        auto f = itr.second;
        fout << f->name << " " << f->coox << " " << f->cooy << " ";
        fout << f->type->size_x << " " << f->type->size_y << " ";
        fout << endl;
    } 

    fout.close();

// ==============================================================================================
    fout.open("./Draw/ff_single_bit.txt", ios::out);
    // Format: Single Bit FFs (Debank All)
    // --------------
    // Die.W Die.H
    // FF num
    // Name X Y W H canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // Name X Y W H canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // Name X Y W H canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // .
    // .
    // .
    // --------------

    fout << DIE.die_width << " " << DIE.die_height << endl;

    ffcnt = 0;
    for(auto flist: INST.ffs_sing){
        ffcnt = ffcnt + flist->size();
    } 
    fout << ffcnt << endl;

    for(auto flist: INST.ffs_sing){
        for(auto f: *flist){
            fout << f->name << " " << f->coox << " " << f->cooy << " ";
            fout << f->type->size_x << " " << f->type->size_y << " ";
            fout << f->fsr.can_move << " ";
            fout << f->fsr.xmax << " ";
            fout << f->fsr.xmin << " ";
            fout << f->fsr.ymax << " ";
            fout << f->fsr.ymin << " ";
            fout << endl;
        }
    } 


    fout.close();
// ==============================================================================================
    fout.open("./Draw/ff_multi_bit.txt", ios::out);
    // Format: Multi Bit FFs (After Banking)
    // --------------
    // Die.W Die.H
    // FF num
    // Name X Y W H bit_num canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // Name X Y W H bit_num canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // Name X Y W H bit_num canmove fsr.xmax fsr.xmin fsr.ymax fsr.ymin
    // .
    // .
    // .
    // --------------

    fout << DIE.die_width << " " << DIE.die_height << endl;

    ffcnt = UPFFS.size();
    fout << ffcnt << endl;

    for(auto f: UPFFS){

        fout << f->name << " " << f->coox << " " << f->cooy << " ";
        fout << f->type->size_x << " " << f->type->size_y << " ";
        fout << f->type->bit_num << " ";
        fout << f->fsr.can_move << " ";
        fout << f->fsr.xmax << " ";
        fout << f->fsr.xmin << " ";
        fout << f->fsr.ymax << " ";
        fout << f->fsr.ymin << " ";
        fout << endl;

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