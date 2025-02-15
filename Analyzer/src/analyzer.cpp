#include "funcs.h"

void Analyzer(lib& LIB, inst& INST, dieInfo& DIE, netlist& NL, list<ffi*>& MBFFs){
    // analyze mbff size distribution
    cout << "Analyzing MBFF size distribution ..." << endl;

    vector<int> arr(LIB.max_ff_size+1, 0);
    double total_cost;

    for(auto f: MBFFs){
        arr[f->type->bit_num]++;
    }


    cout << "+---------------------+" << endl;
    cout << "|  Size  Distribution |" << endl;
    cout << "+---------------------+" << endl;

    for(int i=1; i<=LIB.max_ff_size; i++){
        cout << "|  Size " << right << setw(3) << " |   " << setw(5) << arr[i] << "    |" << endl; 
    }
    cout << "+---------------------+" << endl;
    cout << endl;
}