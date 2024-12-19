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
        int mapWidth;
        int mapHeight;
        dieInfo* DIE;
        placement* PM;
        vector<vector<Bin*>> binMap;
        void ConstructBinMap();
        void SetBinRows();

    public:
        Legalizer(dieInfo* DIE, placement* PM);
        void Initialize();
        void PlaceGate(gatei* gatePointer);
};

class Bin{
    public: 
        int rowNum = 0;
        int index, rowi, colj;
        double cenX, cenY;
        double bottomLeftX, bottomLeftY;
        double upperRightX, upperRightY;
        double rowStartX, rowEndX;
        double rowStartY, rowEndY;
        Bin* upBin;
        Bin* downBin;
        Bin* leftBin;
        Bin* rightBin;
        vector<PlacementRow*> rows;

        // Functions
        Bin(int index, int rowi, int colj, double bottomLeftX, double bottomLeftY, double upperRightX, double upperRightY);
        void AddRow(double startx, double starty, double siteWidth, double siteHeight, int siteNum);
        void Block(double startx, double starty, double width, double height);

};


#endif