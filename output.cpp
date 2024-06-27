#include "func.h"

void Output(string filename, list<ffi*>& PFFS, inst& INST){
    fstream fout;
    std::unordered_map<std::string,double> umap;

    fout.open(filename, ios::out);

    // CellInst num
    fout << "CellInst " << PFFS.size() << endl;

    // Inst <instName> <locationX> <locationY>
    for(auto& fi: PFFS){
        fout << "Inst " << fi->name << " " << fi->type->name << " " << fi->coox << " " << fi->cooy << endl;
    }

    // <originalCellPinFullName> map <resultCellPinFullNameName>
    for(auto& it: INST.ff_umap){
        umap.clear();
        auto fi = it.second;
        for(auto& p: fi->d_pins){
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
            umap.insert(pair<string, int> (p->to_new_ff->name, 0));
        }
        for(auto& p: fi->q_pins){
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
        }
        for(auto& it: umap){
            fout << fi->name << "/CLK map " << it.first << "/CLK" << endl;
        }
    
    }

    fout.close();


    fout.open("test_legal_1.txt", ios::out);
    
    fout << "CellNum " <<  INST.gate_umap.size() + PFFS.size() << endl;

    for(auto& gc_itr: INST.gate_umap){
        auto& gc = gc_itr.second;
        fout << gc->name << " " << gc->coox << " " << gc->cooy << " " << gc->type->size_x << " " << gc->type->size_y << endl;
    }

    for(auto fi: PFFS){
        fout << fi->name << " " << fi->coox << " " << fi->cooy << " " << fi->type->size_x << " " << fi->type->size_y << endl;
    }

    fout.close();

    
    return;
}