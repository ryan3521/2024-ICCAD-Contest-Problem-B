#include <iostream>
#include <vector>
#include <cmath>
#include "inst.h"
#include "lib.h"


#ifndef _CLUSTER_H_
#define _CLUSTER_H_

class cluster{
    public: 
        // 1. cluster centroid
        // 2. cluster size
        // 3. member list
        double cen_x;
        double cen_y;
        int size;
        list<ffi*> member_list;

        cluster(double, double);
        bool updateCentroid();
        void clearMemberList();
};


#endif
