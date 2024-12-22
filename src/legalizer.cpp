#include "legalizer.h"


Legalizer::Legalizer(dieInfo* DIE, placement* PM){
    this->DIE = DIE;
    this->PM  = PM;
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
    for(auto r: PM->rows){
        // initial
        int remain_siteNum = r->site_num;
        double remain_startx = r->start_x;
        double starty        = r->start_y;
        double siteWidth  = r->site_w;
        double siteHeight = r->site_h;

        while(remain_siteNum > 0){
            // Step 1: find closest Bin base on startx & starty
            int rowi = starty/DIE->bin_height;
            int colj = remain_startx/DIE->bin_width;

            // Step 2: cut down a proper placement row >> need to specify "siteNum and starty" from the new placement row
            double startx = remain_startx;
            int siteNum = ceil((binMap[rowi][colj]->upperRightX - startx)/siteWidth);

            // Step 3: add it into the bin
            binMap[rowi][colj]->AddRow(startx, starty, siteWidth, siteHeight, siteNum);

            // Step 4: update the remain placement row >> specify the "siteNum and starty
            remain_startx = remain_startx + siteWidth*siteNum;
            remain_siteNum = remain_siteNum - siteNum;
        }       
    }
    return;
}


void Legalizer::PlaceGate(gatei* gatePointer){
    double startx = gatePointer->coox;
    double starty = gatePointer->cooy;
    double width  = gatePointer->type->size_x;
    double height = gatePointer->type->size_y;

    int rowi = starty/DIE->bin_height;
    int colj = startx/DIE->bin_width;

    rowi = (binMap[rowi][colj]->rowStartY > starty) ? rowi - 1:rowi;
    colj = (binMap[rowi][colj]->rowStartX > startx) ? colj - 1:colj;

    binMap[rowi][colj]->Block(startx, starty, width, height);
    return;
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
            newRow->downRow = downBin->rows[downBin->rowNum-1];
            downBin->rows[downBin->rowNum-1]->upRow = newRow;
        }
    }
    else{
        newRow->downRow = rows[rowNum-1];
        rows[rowNum-1]->upRow = newRow;
    }

    // Check
    if(newRow->downRow != NULL){
        auto downRow = newRow->downRow;
        if(newRow->start_y != downRow->start_y + siteHeight){
            cout << "Error: row do not connect together (Bin::AddRow)" << endl;
        }
        if(newRow->site_num != downRow->site_num){
            cout << "Error: site number not match (Bin::AddRow)" << endl;
        }
        if(newRow->start_x != downRow->start_x){
            cout << "Error: placement row not align (Bin::AddRow)" << endl;
        }
    }

    rows.push_back(newRow);
    rowNum++;

    return;
}

void Bin::Block(double startx, double starty, double width, double height){
    double endx = startx + width;
    double endy = starty + height;

    if(endx > rowEndX && rightBin != NULL){
        rightBin->Block(rowEndX, starty, endx - rowEndX, height);
        endx = rowEndX;
        width = rowEndX - startx;
    }

    if(endy > rowEndY && upBin != NULL){
        upBin->Block(startx, rowEndY, width, endy - rowEndY);
        endy = rowEndY;
        height = rowEndY - starty;
    }

    int start_row_i = (starty - rowStartY)/rows[0]->site_h;
    int end_row_i   = (endy - rowStartY)/rows[0]->site_h;

    for(int i=start_row_i; i<=end_row_i; i++){
        rows[i]->AddBlockAnyway(startx, endx);
    }
    return;
}