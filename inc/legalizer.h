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
        list<Bin*> allBins;
        void ConstructBinMap();
        void SetBinRows();
        void LegalizeFailedFFs(Bin* targetBin);
        bool ExpansionLegalize(Bin* targetBin, ffi* f);
        static bool cmpBin(Bin* a, Bin* b);

    public:
        Legalizer(dieInfo* DIE, placement* PM);
        void Initialize();
        void PlaceGate(gatei* gatePointer);
        void LegalizeAllBins();
};

class Bin{
    private:
        static bool cmpFF(ffi* a, ffi* b);
        struct FailSize{
            double size_x;
            double size_y;
        };

    public: 
        dieInfo* DIE;
        int rowNum = 0;
        int index, rowi, colj;
        bool finishLegalization = false;
        // bool travelMark = false; 
        double cenX, cenY;
        double bottomLeftX, bottomLeftY;
        double upperRightX, upperRightY;
        double rowStartX, rowEndX;
        double rowStartY, rowEndY;
        double distanceToCentroid;
        Bin* upBin;
        Bin* downBin;
        Bin* leftBin;
        Bin* rightBin;
        vector<PlacementRow*> rows;
        list<ffi*> toBePlacedFFs;
        list<ffi*> placeSuccessFFs;
        list<ffi*> placeFailFFs;
        list<FailSize*> failSizeHistory;

        // Functions
        Bin(int index, int rowi, int colj, double bottomLeftX, double bottomLeftY, double upperRightX, double upperRightY, dieInfo* DIE);
        void AddRow(double startx, double starty, double siteWidth, double siteHeight, int siteNum);
        void AddBlock(double startx, double starty, double width, double height);
        void DeleteFFBlock(ffi* f);
        void LegalizeFFList();
        void PlaceFFAt(ffi* f, int bestRowIndex, int bestSiteIndex);
        bool TryToLegalizeFF(ffi* f);
        bool FindAvailable(ffi* f, int& bestRowIndex, int& bestSiteIndex);
        bool matchFailSizeHistory(ffi* f); // true: in history, false: not in history

        //void Push(int direction); 
};


#endif