#include "maxClique.h"

// procedure:
// 1. read all FF
//      --> calMovableRegion
//          * calculate the movable region (diamond) of each FF
//          * rotate movable region to rectangle
//      --> calFeasibleRegion
//          * overlap rectangles and get the feasible region of FF