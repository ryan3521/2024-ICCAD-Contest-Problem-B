#ifndef _LEGALIZER_H_
#define _LEGALIZER_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <limits>


#include "lib.h"
#include "inst.h"
#include "die_info.h"
#include "placement.h"

using namespace std;

class Bin;

class Legalizer{
    private:
        dieInfo* DIE;
        vector<vector<Bin*>> binMap;


    public:
        Legalizer(dieInfo* DIE);
        void Initialize();
        void PlaceGate(gatei* gatePointer);
};

class Bin{
    public: 
        int rowNum = 0;
        double cenX, cenY;
        double bottomLeftX, bottomeLeftY;
        double rowStartX, rowEndX;
        double rowStartY, rowEndY;
        vector<PlacementRow*> rows;

        // Functions
        Bin(int index, int rowi, int colj);
        void AddRow(double startx, double starty, double siteWidth, double siteHeight, int siteNum);
        void Block(double startx, double starty, double width, double height);

};


#endif