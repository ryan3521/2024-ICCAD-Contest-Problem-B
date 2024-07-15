#include "func.h"



void ReadInput(string filename, lib& LIB, inst& INST, dieInfo&  DIE, netlist& NL, placement& PM){
    bool PRINT_INFO = true;
    fstream fin;
    string title;
    string name;
    string typename_;
    string fftype;
    string pin_name;
    double weight;
    double coox, cooy;
    double size_x, size_y;
    int pin_num;
    int bit_num;
    int inst_num;
    int net_num;
    int site_num;
    double width, height;
    double utli_ratio;
    double displacement_delay;
    double delay, slack;
    double power;
    

    fin.open(filename, ios::in);

    if(PRINT_INFO) cout << "Start reading INPUT >>>" << endl;

    // Read weight
    if(PRINT_INFO) cout << "Reading weight ..." << endl;
    fin >> title >> DIE.Alpha;
    fin >> title >> DIE.Beta;
    fin >> title >> DIE.Gamma;
    fin >> title >> DIE.Lambda;

    // Read die info
    if(PRINT_INFO) cout << "Reading die info ..." << endl;
    fin >> title;
    fin >> coox >> cooy;
    fin >> DIE.die_width >> DIE.die_height;

    // Read input pin
    if(PRINT_INFO) cout << "Reading input pin ..." << endl;
    fin >> title >> pin_num;
    for(int i=0; i<pin_num; i++){
        fin >> title >> name ;
        fin >> coox >> cooy ;
        NL.add_Di_pin(name, coox, cooy);
    }

    // Read output pin
    if(PRINT_INFO) cout << "Reading output pin ..." << endl;
    fin >> title >> pin_num;
    for(int i=0; i<pin_num; i++){
        fin >> title >> name ;
        fin >> coox >> cooy ;
        NL.add_Do_pin(name, coox, cooy);
    }

    // Read Library (FF and Gate)
    if(PRINT_INFO) cout << "Reading library ..." << endl;
    ffcell* new_ffcell;
    gcell* new_gcell;
    int cnt = 0;
    while(1){
        cnt++;
        fin >> title;
        // cout << title << "...";
        if((title != "FlipFlop" && title != "Gate")) break;

        if(title == "FlipFlop"){
            fin >> bit_num >> name >> size_x >> size_y >> pin_num;
            // cout << name << endl;
            new_ffcell = new ffcell(name, title, bit_num, size_x, size_y, pin_num);

            // Read pin info
            for(int i=0; i<pin_num-1; i++){
                fin >> title >> name >> coox >> cooy;
                // cout << title << " " << name <<" " << coox<<" " << cooy << endl;
                new_ffcell->add_pin(name, coox, cooy);
            }
            // Read CLK pin
            fin >> title >> name >> coox >> cooy;
            // cout << title << " " << name <<" " << coox<<" " << cooy << endl;
            //cout << coox << endl;
            new_ffcell->set_CLKpin(name, coox, cooy);
            
            LIB.add_cell(new_ffcell);
        }
        else{
            fin >> name >> size_x >> size_y >> pin_num;
            // cout << name << endl;
            new_gcell = new gcell(name, title, size_x, size_y, pin_num);

            // Read pin info
            for(int i=0; i<pin_num; i++){
                fin >> title >> name >> coox >> cooy;
                new_gcell->add_pin(name, coox, cooy);

            }
            
            LIB.add_cell(new_gcell);
        }
    }


    // Read instance
    if(PRINT_INFO) cout << "Reading Instance ..." << endl;
    ffi* new_ffi;
    gatei* new_gatei;
    fin >> inst_num;
    for(int i=0; i<inst_num; i++){
        fin >> title >> name >> typename_ >> coox >> cooy;
        // cout << name << endl;
        INST.add_inst(LIB, name, typename_, coox, cooy);
    }

    // Read Netlist
    if(PRINT_INFO) cout << "Reading Netlist ..." << endl;
    fin >> title >> net_num;
    NL.set_net_num(net_num);

    for(int i=0; i<net_num; i++){
        fin >> title >> name >> pin_num;
        // cout << title << " " << name << endl;
        NL.add_net(name);
        for(int j=0; j<pin_num; j++){
            fin >> title >> name;
            NL.add_pin(name, INST);
        }
    }

    // Read bin info
    if(PRINT_INFO) cout << "Reading bin info ..." << endl;
    fin >> title >> DIE.bin_width;
    fin >> title >> DIE.bin_height;
    fin >> title >> DIE.bin_util;

    // Read placement row info
    if(PRINT_INFO) cout << "Reading placement row info ..." << endl;
    while(1){
        fin >> title ;
        if(title != "PlacementRows"){PM.initial(); break;}
        fin >> coox >> cooy >> width >> height >> site_num;
        PM.addRow(coox, cooy, width, height, site_num);
    }


    // Read DisplacementDelay
    if(PRINT_INFO) cout << "Reading DisplacementDelay ..." << endl;
    fin >> DIE.displacement_delay;


    // Read Q pin delay
    if(PRINT_INFO) cout << "Reading Q pin delay ..." << endl;
    while(1){
        fin >> title ;
        if(title != "QpinDelay") break;

        fin >> name >> delay;
        LIB.set_Qpin_delay(name, delay); // (clk to Q delay)
    }

    // Read Timing Slack
    if(PRINT_INFO) cout << "Reading Timing Slack ..." << endl;
    while(1){
        if(title != "TimingSlack") break;

        fin >> name >> pin_name >> slack;
        INST.set_TSlack(name, pin_name, slack);
        fin >> title;
    }

    // Read Gate Power
    if(PRINT_INFO) cout << "Reading Gate Power ..." << endl;
    while(1){
        if(title != "GatePower" || fin.eof()) break;

        fin >> name >> power;
        LIB.set_gate_power(name, power);
        fin >> title;
    }

    if(PRINT_INFO) cout << "Reading DONE" << endl;

    return;
}