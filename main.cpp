#include <bits/stdc++.h>

#include "func.h"
#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "netlist.h"
#include "placement.h"
// #include "cluster.h"

using namespace std;

void PrintInput(lib& LIB, inst& INST, dieInfo& DIE, netlist& NL){
    cout << "#################################" << endl;
    cout << "#             INFO              #" << endl; 
    cout << "#################################" << endl;
    // --------------------------------------
    cout << "Alpha "   << DIE.Alpha  << endl;
    cout << "Beta "    << DIE.Beta   << endl; 
    cout << "Gamma "   << DIE.Gamma  << endl; 
    cout << "Lambda "  << DIE.Lambda << endl; 
    cout << "DieSize 0.0 0.0 " << DIE.die_width << " " << DIE.die_height << endl;
    // --------------------------------------
    cout << "NumInput " << NL.DI_num << endl;
    for(auto it: NL.Diopins_umap){
        if(it.second->io == 0 && it.second->pin_type == 'd'){
            cout << "Input " << it.second->name << " ";
            cout << it.second->coox << " " << it.second->cooy << endl;
        }
    }
    cout << "NumOutput " << (NL.Diopins_umap.size() - NL.DI_num) << endl;
    for(auto it: NL.Diopins_umap){
        if(it.second->io == 1 && it.second->pin_type == 'd'){
            cout << "Output " << it.second->name << " ";
            cout << it.second->coox << " " << it.second->cooy << endl;
        }
    }
    // --------------------------------------
    for(auto& it: LIB.fftable){
        if(it.size() > 0){
            for(auto ff: it){
                cout << "FlipFlop " << ff->bit_num << " " << ff->name << " " << ff->size_x << " " << ff->size_y << " " << ff->pin_num << endl;
                for(auto& p: ff->d_pins){
                    cout << "Pin " << p.name << " " << p.x_plus << " " << p.y_plus << endl;
                }
                
                for(auto& p: ff->q_pins){
                    cout << "Pin " << p.name << " " << p.x_plus << " " << p.y_plus << endl;
                }
                cout << "Pin " << ff->clk_pin.name << " " << ff->clk_pin.x_plus << " " << ff->clk_pin.y_plus << endl;
            }
        }
    }
    for(auto& c: LIB.cell_umap){
        if(c.second->type == "Gate"){
            gcell* g = (gcell*)(c.second);
            cout << "Gate " << g->name << " " << g->size_x << " " << g->size_y << " " << g->pin_num << endl;
            for(auto& p: g->in_pins){
                cout << "Pin " << p.name << " " << p.x_plus << " " << p.y_plus << endl;
            }
            
            for(auto& p: g->out_pins){
                cout << "Pin " << p.name << " " << p.x_plus << " " << p.y_plus << endl;
            }
        }
    }
    // --------------------------------------
    cout << "NumInstances " << INST.type_umap.size() << endl;
    for(auto& u: INST.ff_umap){
        auto& i = u.second;
        cout << "Inst " << i->name << " " << i->type->name << " " << i->coox << " " << i->cooy << endl;
    }
    for(auto& u: INST.gate_umap){
        auto& i = u.second;
        cout << "Inst " << i->name << " " << i->type->name << " " << i->coox << " " << i->cooy << endl;
    }
    // --------------------------------------
    cout << "NumNets " << NL.net_num << endl;
    for(auto& n: NL.nets){
        cout << "Net " << n->name << " " << (n->ipins.size() + n->opins.size()) << endl;
        for(auto& p: n->ipins){
            cout << "Pin " << p->name;
            if(p->pin_type == 'd') cout << endl;
            else if(p->pin_type == 'f') cout << "/" << p->to_ff->name   << endl;
            else if(p->pin_type == 'g') cout << "/" << p->to_gate->name << endl; 
        }
        for(auto& p: n->opins){
            cout << "Pin " << p->name;
            if(p->pin_type == 'd') cout << endl;
            else if(p->pin_type == 'f') cout << "/" << p->to_ff->name   << endl;
            else if(p->pin_type == 'g') cout << "/" << p->to_gate->name << endl; 
        }
    }
    // --------------------------------------
    cout << "DisplacementDelay " << DIE.displacement_delay << endl;
    // --------------------------------------
    for(auto& it: LIB.fftable){
        if(it.size() > 0){
            for(auto ff: it){
                cout << "QpinDelay " << ff->name << " " << ff->Qpin_delay << endl;
            }
        }
    }
    // --------------------------------------
    for(auto& i: INST.ff_umap){
        auto& f = i.second;
        for(auto& p: f->d_pins){
            cout << "TimingSlack " << f->name << " " << p->name << " " << p->slack << endl;
        }
    }
    // --------------------------------------
    for(auto& it: LIB.fftable){
        if(it.size() > 0){
            for(auto ff: it){
                cout << "GatePower " << ff->name << " " << ff->gate_power << endl;
            }
        }
    }
}

void PrintDataAnalysis(lib& LIB, inst& INST, dieInfo& DIE, netlist& NL){
    string filename = "DataAnalysis.txt";
    ofstream fout(filename);

    fout << "FlipFlop Table" << endl;
    
    double cost_per_bit;
    for(auto& l: LIB.fftable){
        if(l.size() > 0) {
            fout << "\tBit Size: " << l.front()->bit_num << endl;
            fout << std::left << "\t\tName  ClktoQ   Power    Area    CostPerBit" << endl;
        }
        for(auto& f: l){
            cost_per_bit = (DIE.Beta*f->gate_power + DIE.Gamma*f->area)/(double)(l.front()->bit_num);
            fout << "\t\t";
            fout << std::left << setw(5) << f->name << " " << setw(7) <<f->Qpin_delay << "  " << setw(8) << f->gate_power << " " << setw(6) << f->area << "  " << cost_per_bit << endl;
        }
        fout << endl;
    }

    fout << "Optimized FlipFlop Table" << endl;


    fout.close();
}

int main(int argc, char** argv){
    lib LIB;
    inst INST;
    dieInfo DIE;
    netlist NL;
    placement PM;
    // list<cluster*> KCR; // Kmeans Cluster Result
    list<ffi*> NCLS; // Store the non cluster ffs after doing K-means Cluster
    list<ffi*> MBFFS; // Store the new MBFFs after doing K-means Cluster
    list<ffi*> UPFFS; // Store all ffs which need to be placed (== NCLS + MBFFS)
    list<ffi*> PFFS; // Store all ffs which are placed



    srand(time(NULL));
    
    ReadInput(argv[1], LIB, INST, DIE, NL, PM);
    LIB.construct_fftable(DIE);
    INST.SlackDispense_Q(DIE);
    // KmeansCls(LIB, INST, KCR, NCLS);
    // MapClstoMBFF(LIB, KCR, MBFFS);
    // FineTune(LIB, NCLS, MBFFS, UPFFS, DIE); // Not finish yet
    // PM.placeGateInst(INST);
    // PM.placeFlipFlopInst(LIB, INST, DIE, UPFFS, PFFS);

    // If placement success ...
    // Output PFFS Info 
    // Done 














    // PrintInput(LIB, INST, DIE, NL);
    int LIMIT = 50;
    int cnt = 0;
    int neg_dslack_ff = 0;
    int neg_qslack_ff = 0;
    int arr[4] = {0, 0, 0, 0};
    cout << "FlipFlop Instance Count: " << INST.ff_umap.size() << endl;
    for(auto& i: INST.ff_umap){
        auto& f = i.second;
        // cout << "Instance: " << f->name << endl;
        for(int i=0; i<f->type->bit_num; i++){
            // cout << "  " << f->d_pins[i]->name << " " << f->d_pins[i]->dspd_slk << endl;
            // cout << "  " << f->q_pins[i]->name << " " << f->q_pins[i]->dspd_slk << endl;
            if(f->d_pins[i]->slack < 0) neg_dslack_ff++;
            if(f->q_pins[i]->dspd_slk < 0) neg_qslack_ff++;
        }
        // cnt ++;
        // if(cnt > LIMIT) break;
        arr[f->type->bit_num-1]++;
    }
    
    for(int i=0; i<4; i++){
        cout << i << ": " << arr[i] << endl;
    }

    cout << "Negative D Number: " << neg_dslack_ff << endl;
    cout << "Negative Q Number: " << neg_qslack_ff << endl;
    PrintDataAnalysis(LIB, INST, DIE, NL);

    
    
    return 0;
}



