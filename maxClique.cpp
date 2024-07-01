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
#include "maxClique.h"

MaxClique::MaxClique(){
    ;
}

MaxClique::~MaxClique(){
    ;
}

double MaxClique::calHPWL(pin* from, pin* to){
    
}