#include "func.h"

void Output(string filename, list<ffi*>& PFFS, inst& INST){
    fstream fout;

    fout.open(filename, ios::out);

    // CellInst num
    fout << "CellInst " << PFFS.size() << endl;

    // Inst <instName> <locationX> <locationY>
    for(auto& fi: PFFS){
        fout << "Inst " << fi->name << " " << fi->coox << " " << fi->cooy << endl;
    }

    // <originalCellPinFullName> map <resultCellPinFullNameName>
    for(auto& it: INST.ff_umap){
        auto fi = it.second;
        for(auto& p: fi->d_pins){
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
        }
        for(auto& p: fi->q_pins){
            fout << fi->name << "/" << p->name << " map " << p->to_new_ff->name << "/" << p->new_name << endl;
        }
        // fout << fi->name << "/" << fi->clk_pin->name << " map " << fi->clk_pin->to_new_ff->name << "/" << fi->clk_pin->new_name << endl;
    }

    fout.close();
    
    return;
}