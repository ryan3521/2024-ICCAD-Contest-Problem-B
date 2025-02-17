#include "funcs.h"

void Analyzer(lib& LIB, inst& INST, dieInfo& DIE, netlist& NL, list<ffi*>& MBFFs){
    // analyze mbff size distribution
    cout << endl << "Analyzing MBFF size distribution ..." << endl;

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


    // analyze the displacement of pins
    cout << endl << "Analyzing the displacement of pins ..." << endl;
    for(auto f: MBFFs){
        f->update_pin_loc();
    }
    
    double total_displacement = 0;
    vector<double> displacementArr(LIB.max_ff_size+1, 0);
    for(auto f: MBFFs){
        double accDisplacement = 0;
        double hpwl;
        for(int i=0; i<f->type->bit_num; i++){
            hpwl = abs(f->d_pins[i]->coox - f->d_pins[i]->new_coox) + abs(f->d_pins[i]->cooy - f->d_pins[i]->new_cooy);
            accDisplacement += hpwl;
            hpwl = abs(f->q_pins[i]->coox - f->q_pins[i]->new_coox) + abs(f->q_pins[i]->cooy - f->q_pins[i]->new_cooy);
            accDisplacement += hpwl;
        }
        f->pinDisplacement = accDisplacement;
        total_displacement += accDisplacement;
        displacementArr[f->type->bit_num] += accDisplacement;
    }

    cout << "Total pin displacement: " << total_displacement << endl;
    cout << "=========================================================" << endl;
    cout << "Size 1 total displacement: " << displacementArr[1] << endl;
    cout << "Size 2 total displacement: " << displacementArr[2] << endl;
    cout << "Size 4 total displacement: " << displacementArr[4] << endl;
    cout << "=========================================================" << endl;
    cout << "Size 1 displacement per bit: " << displacementArr[1]/(double)(arr[1]*1) << endl;
    cout << "Size 2 displacement per bit: " << displacementArr[2]/(double)(arr[2]*2) << endl;
    cout << "Size 4 displacement per bit: " << displacementArr[4]/(double)(arr[4]*4) << endl;
}