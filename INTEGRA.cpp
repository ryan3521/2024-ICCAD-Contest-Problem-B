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

INTEGRA::INTEGRA(inst I):INST(I) {
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

list<Diamond> INTEGRA::calMovableRegion(ffi* ff){
    list<Diamond> diamonds;
    // Assume that all FFs are single bit FF
    pin* D = ff->d_pins[0];
    pin* Q = ff->q_pins[0];
    double bias = calHPWL(D,Q); // its a adjustment term, HPWL between D & Q of current FF
    // ************************************
    // *       D pin movable regions      *
    // ************************************
    // D is one of the output pin of the net it belongs to
    // the input pin of such net is the output pin of previous gate that connect to current FF
    pin* const &prevGatePin = *(D->to_net->ipins.begin()); // pin that connects to D
    // todo: add slack to the radius 
    double slack = D->slack / 2.0;
    double radius = calHPWL(prevGatePin,D) + bias + slack;
    // Calculate the 4 corners and construct the diamond
    diamonds.push_back(Diamond({Vertex(prevGatePin->coox - radius, prevGatePin->cooy),
                                Vertex(prevGatePin->coox, prevGatePin->cooy - radius),
                                Vertex(prevGatePin->coox + radius, prevGatePin->cooy),
                                Vertex(prevGatePin->coox, prevGatePin->cooy + radius)}));
    // ************************************
    // *       Q pin movable regions      *
    // ************************************
    // Q is the input pin of the net it belongs to
    // find all output pins of such net and calculate their moveable region
    list<pin*> const &pins = Q->to_net->opins;
    for(pin* const &pin:pins){
        if(pin->pin_type == 'f'){
            slack = 
            radius = calHPWL(Q,pin) + bias;
        }
        else if(pin->pin_type == 'g'){

        }
        // ignore the case pin->pin_type == 'd'
    }
}

void INTEGRA::run(){
// procedure:
// 1. read all FF
//      for all FF in same clock domain
//      --> calMovableRegion
//          * calculate the movable region (diamond) of each FF
//          * rotate movable region to rectangle
//      --> calFeasibleRegion
//          * overlap rectangles and get the feasible region of FF
// 2. INTEGRA
//      --> find the max clique of all feasible regions
    for(list<ffi*>* ffs:INST.ff_clk_group){
        vector<Rectangle> feasRegions;
        for(ffi* ff:(*ffs)){
            list<Diamond> mvRegion = calMovableRegion(ff);
            feasRegions.push_back(calFeasibleRegion(mvRegion));
        }
        // todo: use feasible region to find max clique
        // ...
    }
}