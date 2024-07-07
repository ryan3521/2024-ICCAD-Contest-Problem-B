#include "INTEGRA.h"

Vertex::Vertex(){
    ;
}

Vertex::Vertex(double x_in, double y_in):x(x_in), y(y_in) {
    ;
}

Vertex::~Vertex(){
    ;
}


INTEGRA::INTEGRA(){
    ;
}

INTEGRA::INTEGRA(inst I, netlist N):INST(I), NL(N) {
    ;
}

INTEGRA::~INTEGRA(){
    ;
}

Diamond::Diamond(){
    ;
}

Diamond::Diamond(vector<Vertex> c):corners(c){
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
    for(net* const &net:NL.nets){
        pin* inputPin = *(net->ipins.begin());
        if(inputPin->pin_type != 'd') continue;

        // traverse the circuit from current pin
        queue<pin*> q;
        q.push(inputPin);
        while(!q.empty()){
            pin* curPin = q.front();
            q.pop();
            if(curPin->isVisited) continue;
            curPin->isVisited = true;
            visitedPins.push_back(curPin);
            if(curPin->pin_type == 'd') continue;
            if(curPin->pin_type == 'f'){
                topFFs.push_back(curPin->to_ff);
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

void INTEGRA::calMovableRegion(){
    for(ffi* ff:topFFs){
        list<Diamond> diamonds; // moveable regions

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
            diamonds.push_back(Diamond({Vertex(prevGatePin->coox - radius, prevGatePin->cooy),
                                    Vertex(prevGatePin->coox, prevGatePin->cooy - radius),
                                    Vertex(prevGatePin->coox + radius, prevGatePin->cooy),
                                    Vertex(prevGatePin->coox, prevGatePin->cooy + radius)}));

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
                            nextLevelFFs.push_back(curPin->to_ff);
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
                    for(ffi* FF:nextLevelFFs){
                        if(FF->d_pins[0]->slack < minSlack) minSlack = FF->d_pins[0]->slack;
                    }

                    minSlack = (minSlack < 0)? minSlack : minSlack/2;
                    for(ffi* FF:nextLevelFFs){
                        FF->d_pins[0]->slack -= minSlack;
                    }
                    
                    radius = calHPWL(Q,opin) + bias + minSlack;
                    
                }
                // push current inst's moveable region into diamonds
                diamonds.push_back(Diamond({Vertex(opin->coox - radius, opin->cooy),
                                            Vertex(opin->coox, opin->cooy - radius),
                                            Vertex(opin->coox + radius, opin->cooy),
                                            Vertex(opin->coox, opin->cooy + radius)}));
            }
        }
    }
}

void INTEGRA::run(){
    // 1. Find all "Top Level FlipFlops"
    findTopFF();

    // 2. Use BFS on every top level FFs, calculate their feasible region

    // 3. INTEGRA
    

}