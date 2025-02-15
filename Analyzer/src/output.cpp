#include "func.h"

void Output(string filename, list<ffi*>& PFFS, inst& INST){
    // cout << endl << "Writing Output >>> " << endl;
    fstream fout;

    fout.open(filename, ios::out);

    // CellInst num
    fout << "CellInst " << PFFS.size() << endl;

    // Inst <instName> <type> <locationX> <locationY>
    for(auto& fi: PFFS){
        fout << "Inst " << fi->name << " " << fi->type->name << " " << fi->coox << " " << fi->cooy << endl;
    }

    // <originalCellPinFullName> map <resultCellPinFullNameName>
    pin* p;
    for(auto& it: INST.ff_umap){
        auto fi = it.second;
        for(int i=0; i<fi->d_pins.size(); i++){
            p = fi->d_pins[i];
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
            p = fi->q_pins[i];
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
            fout << fi->name << "/CLK map " << p->to_new_ff->name << "/CLK" << endl;
        }  
    }

    fout.close();


    // fout.open("test_legal_1.txt", ios::out);
    
    // fout << "CellNum " <<  INST.gate_umap.size() + PFFS.size() << endl;

    // for(auto& gc_itr: INST.gate_umap){
    //     auto& gc = gc_itr.second;
    //     fout << gc->name << " " << gc->coox << " " << gc->cooy << " " << gc->type->size_x << " " << gc->type->size_y << endl;
    // }

    // for(auto fi: PFFS){
    //     fout << fi->name << " " << fi->coox << " " << fi->cooy << " " << fi->type->size_x << " " << fi->type->size_y << endl;
    // }

    // fout.close();

    
    return;
}