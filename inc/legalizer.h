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
        lib*  LIB;
        inst* INST;
        dieInfo* DIE;
        placement* PM;
        int mapWidth;
        int mapHeight;
        vector<vector<Bin*>> binMap;
        list<Bin*> allBins;
        void ConstructBinMap();
        void SetBinRows();
        void PlaceAllGates();
        void FillTrivialGap();
        void PlaceGate(gatei* gatePointer);
        void LegalizeFailedFFs(Bin* targetBin);
        bool ExpansionLegalize(Bin* targetBin, ffi* f, bool place);
        static bool cmpBin(Bin* a, Bin* b);

    public:
        Legalizer(inst* INST, lib* LIB, dieInfo* DIE, placement* PM);
        void Initialize();
        void AddToBePlacedFF(ffi* f);
        void LegalizeAllBins();
        void FindAvailableAndUpdatePin(ffi* f);
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
        void FillTrivialGap(double gapWidth);
        bool TryToLegalizeFF(ffi* f);
        bool FindAvailable(ffi* f, int& bestRowIndex, int& bestSiteIndex);
        bool matchFailSizeHistory(ffi* f); // true: in history, false: not in history

        //void Push(int direction); 
};


#endif