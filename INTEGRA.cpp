#include "INTEGRA.h"

Rectangle::Rectangle(): x(0.0), y(0.0), w(0.0), h(0.0), FF(nullptr) {
    ;
}

Rectangle::Rectangle(double x_in, double y_in, double w_in, double h_in, ffi* FF_in): x(x_in), y(y_in), w(w_in), h(h_in), FF(FF_in) {
    ;
}

Rectangle::~Rectangle(){
    ;
}

// INTEGRA::INTEGRA(){
//     ;
// }

// INTEGRA::INTEGRA(inst I, netlist N):INST(I), NL(N) {
//     ;
// }

INTEGRA::INTEGRA(inst* I, netlist* N){
    INST = I;
    NL = N;
}

INTEGRA::~INTEGRA(){
    ;
}

Diamond::Diamond(){
    ;
}

Diamond::Diamond(double x_in, double y_in, double r):x(x_in), y(y_in), radius(r) {
    ;
}

Diamond::~Diamond(){
    ;
}

double INTEGRA::calHPWL(pin* from, pin* to){
    return abs(from->coox - to->coox) + abs(from->cooy - to->cooy);
}

void INTEGRA::findTopFF(){
    vector<pin*> visitedPins;

    // for(auto& n: NL->nets){
    //     if(n->ipins.size() != 1){
    //         cout << "Wrong: " << n->ipins.size() << " " << n->name << endl;
    //     }
    // }

    for(net* const &net:NL->nets){
        if(net->ipins.empty()) continue;
        pin* inputPin = *(net->ipins.begin());
        //cout << inputPin->pin_type << endl;
        if(inputPin->pin_type != 'd') continue;
        // cout << "Find die input pin" << endl;
        // traverse the circuit from current pin
        queue<pin*> q;
        for(pin* p:net->opins){
            q.push(p);
        }
        while(!q.empty()){
            pin* curPin = q.front();
            q.pop();
            if(curPin->isVisited) continue;
            curPin->isVisited = true;
            visitedPins.push_back(curPin);
            if(curPin->pin_type == 'd') continue;
            if(curPin->pin_type == 'f'){
                topFFs.push_back(curPin->to_new_ff);
                continue;
            }
            // if curPin is gate pin
            vector<pin*> outputPins = curPin->to_gate->opins;
            for(pin* outputPin:outputPins){
                for(pin* nextLevelPin:outputPin->to_net->opins){
                    q.push(nextLevelPin);
                }
            }
        }
    }
    // clear visited flag
    for(pin* visitedPin:visitedPins){
        visitedPin->isVisited = false;
    }
}

void INTEGRA::calFeasibleRegion(){
    for(ffi* ff:topFFs){
        // *********************************
        // *   Calculate Moveable Region   *
        // *********************************
        vector<Diamond> diamonds; // moveable regions

        queue<ffi*> ff_q;
        ff_q.push(ff);

        while(!ff_q.empty()){
            ffi* curFF = ff_q.front();
            ff_q.pop();

            // Assume that all FFs are single bit FF
            pin* D = curFF->d_pins[0];
            pin* Q = curFF->q_pins[0];
            if(D->isVisited) continue;
            D->isVisited = true;
            // ************************************
            // *       D pin movable regions      *
            // ************************************
            pin* const &prevGatePin = *(D->to_net->ipins.begin());
            double bias = calHPWL(D,Q);
            double slack = D->slack;
            double radius = calHPWL(prevGatePin,D) + bias + slack;
            diamonds.push_back(Diamond(prevGatePin->coox, prevGatePin->cooy,radius));

            // ************************************
            // *       Q pin movable regions      *
            // ************************************
            // Find all next level flip-flops and stores in nextLevelFFs
            for(pin* opin:Q->to_net->opins){
                // opin is the input pin of current FF's output gate
                // update current inst's radius
                if(opin->pin_type == 'f'){
                    radius = calHPWL(Q,opin) + bias + opin->slack;
                }
                else if(opin->pin_type == 'g'){
                    list<ffi*> nextLevelFFs;
                    
                    queue<pin*> pin_q;
                    if(opin->isVisited) continue;
                    pin_q.push(opin);

                    while(!pin_q.empty()){
                        pin* curPin = pin_q.front();
                        pin_q.pop();
                        if(curPin->isVisited) continue;
                        if(curPin->pin_type == 'd') continue;
                        if(curPin->pin_type == 'f'){
                            nextLevelFFs.push_back(curPin->to_new_ff);
                            continue;
                        }
                        // push the next level gate input pins into pin_q
                        curPin->isVisited = true;
                        for(pin* p:curPin->to_gate->opins){
                            for(pin* next_ipin:p->to_net->opins){
                                pin_q.push(next_ipin);
                            }
                        }
                    }
                    

                    // Update next level FFs' slack
                    double minSlack = DBL_MAX;
                    if(nextLevelFFs.empty()){
                        radius = -1;
                        continue;
                    }
                    for(ffi* FF:nextLevelFFs){
                        // cout << FF->d_pins[0]->slack << '\n';
                        if(FF->d_pins[0]->slack < minSlack) minSlack = FF->d_pins[0]->slack;
                    }
                    
                    minSlack = (minSlack < 0)? minSlack : minSlack/2;
                    for(ffi* FF:nextLevelFFs){
                        FF->d_pins[0]->slack -= minSlack;
                    }
                    
                    radius = calHPWL(Q,opin) + bias + minSlack;
                    
                }
                // push current inst's moveable region into diamonds
                if(radius != -1) diamonds.push_back(Diamond(opin->coox, opin->cooy, radius));
            }
            // *********************************
            // *   Calculate Feasible Region   *
            // *********************************
            double leftBound = diamonds[0].y + diamonds[0].x - diamonds[0].radius;
            double rightBound = diamonds[0].y + diamonds[0].x - diamonds[0].radius;
            double botBound = diamonds[0].y - diamonds[0].x - diamonds[0].radius;
            double topBound = diamonds[0].y - diamonds[0].x + diamonds[0].radius;
            size_t size = diamonds.size();
            for(size_t i = 1; i < size; ++i){
                double x = diamonds[i].x;
                double y = diamonds[i].y;
                double r = diamonds[i].radius;
                leftBound = max(leftBound, y+x-r);
                rightBound = min(rightBound, y+x-r);
                botBound = max(botBound, y-x-r);
                topBound = min(topBound, y+x-r);
            }
            feasRegions.push_back(Rectangle(leftBound, botBound, rightBound-leftBound, topBound-botBound, curFF));
        }
    }
}

void INTEGRA::copyFSR(){
    for(auto& rec: feasRegions){
        rec.FF->fsr.x = rec.x;
        rec.FF->fsr.y = rec.y;
        rec.FF->fsr.w = rec.w;
        rec.FF->fsr.h = rec.h;

        cout << rec.FF->fsr.w << " " << rec.FF->fsr.h << endl;
    }
    cout << "Copy FSR done ..." << endl;
}

void INTEGRA::run(){
    // 1. Find all "Top Level FlipFlops"
    findTopFF();
    cout << "Find top ff done" << endl;
    cout << "Top FF Num: " << topFFs.size() << endl;
    // 2. Use BFS on every top level FFs, calculate their feasible region
    calFeasibleRegion();
    
    // 3. INTEGRA
    

}