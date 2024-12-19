#include "legalizer.h"


Legalizer::Legalizer(dieInfo* DIE){
    this->DIE = DIE;

}

void Legalizer::Initialize(){
    ConstructBinMap();
    SetBinRows();

}

void Legalizer::ConstructBinMap(){
    cout << "Constructing Bin Map ..." << endl; 

    mapHeight = ceil(DIE->die_height/DIE->bin_height);
    mapWidth  = ceil(DIE->die_width/DIE->bin_width);

    binMap.resize(mapHeight);
    for(int i=0; i<mapHeight; i++){
        binMap[i].resize(mapWidth, NULL);
    }

    int binCnt = 0;
    for(int i=0; i<mapHeight; i++){
        for(int j=0; j<mapWidth; j++){
        double bottomLeftX  = DIE->bin_width*j;
        double bottomLeftY = DIE->bin_height*i;
        double upperRightX  = (bottomLeftX + DIE->bin_width  > DIE->die_width)  ? DIE->die_width  : bottomLeftX + DIE->bin_width;
        double upperRightY  = (bottomLeftY + DIE->bin_height > DIE->die_height) ? DIE->die_height : bottomLeftY + DIE->bin_height;
            Bin* binPtr = new Bin(binCnt, i, j, bottomLeftX, bottomLeftY, upperRightX, upperRightY);
            binCnt++;
            binMap[i][j] = binPtr;
        }
    }

    for(int i=0; i<mapHeight; i++){
        for(int j=0; j<mapWidth; j++){
            // link left bin
            if(j-1 < 0){
                binMap[i][j]->leftBin = NULL;
            }
            else{
                binMap[i][j]->leftBin = binMap[i][j-1];
            }

            // link right bin
            if(j+1 < mapWidth){
                binMap[i][j]->rightBin = binMap[i][j+1];
            }
            else{
                binMap[i][j]->rightBin = NULL;
            }
            
            // link down bin
            if(i-1 < 0){
                binMap[i][j]->downBin = NULL;
            }
            else{
                binMap[i][j]->downBin = binMap[i-1][j];
            }

            // link up bin
            if(i+1 < mapHeight){
                binMap[i][j]->upBin = binMap[i+1][j];
            }
            else{
                binMap[i][j]->upBin = NULL;
            }
        }
    }

}

void Legalizer::SetBinRows(){

}

Bin::Bin(int index, int rowi, int colj, double bottomLeftX, double bottomLeftY, double upperRightX, double upperRightY){
    rowNum = 0;
    this->index = index;
    this->rowi = rowi;
    this->colj = colj;
    this->bottomLeftX = bottomLeftX;
    this->bottomLeftY = bottomLeftY;
    this->upperRightX = upperRightX;
    this->upperRightY = upperRightY;
    cenX = (bottomLeftX + upperRightX)/2;
    cenY = (bottomLeftY + upperRightY)/2;
}

void Bin::AddRow(double startx, double starty, double siteWidth, double siteHeight, int siteNum){
    if(rowNum == 0){
        rows.reserve((upperRightY - bottomLeftY)/siteHeight);
        rowStartX = startx;
        rowEndX   = startx + siteNum*siteWidth;
        rowStartY = starty;
    }
    rowEndY = starty + siteHeight;
    
    PlacementRow* newRow = new PlacementRow(startx, starty, siteWidth, siteHeight, siteNum);
    newRow->idx = rowNum;
    
    if(rowNum == 0){
        if(rowi != 0){
            newRow->down_rows.push_back(downBin->rows[downBin->rowNum-1]);
            downBin->rows[downBin->rowNum-1]->up_rows.push_back(newRow);
        }
    }
    else{
        newRow->down_rows.push_back(rows[rowNum-1]);
        rows[rowNum-1]->up_rows.push_back(newRow);
    }

    rows.push_back(newRow);
    rowNum++;

    return;
}