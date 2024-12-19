#ifndef _DIE_INFO_H_
#define _DIE_INFO_H_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class dieInfo{
    public:
        // design weight
        double Alpha;
        double Beta;
        double Gamma;
        double Lambda;

        // die size
        double die_width;
        double die_height;


        // displacement delay coefficient
        double displacement_delay;

        // Bin Info
        double bin_width;
        double bin_height;
        double bin_util;

        
};




#endif