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
            Bin* binPtr = new Bin(binCnt, i, j, bottomLeftX, bottomLeftY, upperRightX, upperRightY, DIE);
            binCnt++;
            binMap[i][j] = binPtr;
            allBins.push_back(binPtr);
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

    binMap[rowi][colj]->AddBlock(startx, starty, width, height);
    return;
}

void Legalizer::LegalizeAllBins(){
    for(auto b: allBins){
        b->distanceToCentroid = pow(b->cenX - DIE->cenx, 2) + pow(b->cenY - DIE->ceny, 2);
    }
    allBins.sort(cmpBin);

    for(auto b: allBins){
        b->LegalizeFFList();
        LegalizeFailedFFs(b);
    }
    return;
}

void Legalizer::LegalizeFailedFFs(Bin* targetBin){
    while(targetBin->placeFailFFs.empty() == false){
        ffi* f = targetBin->placeFailFFs.front();
        targetBin->placeFailFFs.pop_front();

        if(ExpansionLegalize(targetBin, f) == false){
            cout << "Cannot find space to legalize (call by Legalizer::LegalizeFailedFFs)" << endl;
            break;
        }
    }
}

void Legalizer::FillTrivialGap(double gapWidth){
    for(auto b: allBins){
        b->FillTrivialGap(gapWidth);
    }
    return;
}

void Legalizer::AddToBePlacedFF(ffi* f){
    int rowi = f->cen_y/DIE->bin_height;
    int colj = f->cen_x/DIE->bin_width;

    if(rowi < 0) {rowi = 0;}
    else if(rowi >= mapHeight) {rowi = mapHeight - 1;}
    if(colj < 0) {colj = 0;}
    else if(colj >= mapWidth)  {colj = mapWidth - 1;}

    binMap[rowi][colj]->toBePlacedFFs.push_back(f);
}

bool Legalizer::ExpansionLegalize(Bin* targetBin, ffi* f){
    int  expansion = 1;
    int  bestRowIndex;
    int  bestSiteIndex;

    Bin* bestBin = NULL;
    int  bestBinRowIndex;
    int  bestBinSiteIndex;
    double globalMincost = numeric_limits<double>::max();

    if(targetBin == NULL) return false;

    while(1){
        int maxi = targetBin->rowi + expansion;
        int mini = targetBin->rowi - expansion;
        int maxj = targetBin->colj + expansion;
        int minj = targetBin->colj - expansion;
        
        int upRight_i = maxi;
        int upRight_j = maxj;

        int upLeft_i = maxi;
        int upLeft_j = minj;
        
        int downRight_i = mini;
        int downRight_j = maxj;
        
        int downLeft_i = mini;
        int downLeft_j = minj;

        if(minj < 0 && mini < 0 && maxi >= mapHeight && maxj >= mapWidth){
            return false;
        }    
        
        // Up
        if(maxi < mapHeight){
            int startj = (minj < 0) ? 0 : minj;
            int endj   = (maxj < mapWidth) ? (maxj) : (mapWidth - 1);
        
            for(int j=startj; j<=endj; j++){
                Bin* targetBin = binMap[maxi][j];
                if(targetBin->FindAvailable(f, bestRowIndex, bestSiteIndex)){
                    // calculate cost 
                    double coox = targetBin->rows[bestRowIndex]->start_x + bestSiteIndex*targetBin->rows[bestRowIndex]->site_w;
                    double cooy = targetBin->rows[bestRowIndex]->start_y;
                    double cost = pow(f->coox - coox, 2) + pow(f->cooy - cooy, 2);

                    // compare with globalMincost
                    if(cost < globalMincost){
                        globalMincost = cost;
                        bestBin          = targetBin;
                        bestBinRowIndex  = bestRowIndex;
                        bestBinSiteIndex = bestSiteIndex;
                    }
                }
            }
        }

        // Right
        if(maxj < mapWidth){
            int starti   = (mini < 0) ? 0 : (mini + 1);
            int endi = (maxi < mapHeight) ? (maxi - 1) : (mapHeight - 1); 

            for(int i=starti; i<=endi; i++){
                Bin* targetBin = binMap[i][maxj];
                if(targetBin->FindAvailable(f, bestRowIndex, bestSiteIndex)){
                    // calculate cost 
                    double coox = targetBin->rows[bestRowIndex]->start_x + bestSiteIndex*targetBin->rows[bestRowIndex]->site_w;
                    double cooy = targetBin->rows[bestRowIndex]->start_y;
                    double cost = pow(f->coox - coox, 2) + pow(f->cooy - cooy, 2);

                    // compare with globalMincost
                    if(cost < globalMincost){
                        globalMincost = cost;
                        bestBin          = targetBin;
                        bestBinRowIndex  = bestRowIndex;
                        bestBinSiteIndex = bestSiteIndex;
                    }
                }
            }
        }

        // Down
        if(mini >= 0){
            int startj = (minj < 0) ? 0 : minj;
            int endj   = (maxj < mapWidth) ? (maxj) : (mapWidth - 1);
            
            for(int j=startj; j<=endj; j++){
                Bin* targetBin = binMap[mini][j];
                if(targetBin->FindAvailable(f, bestRowIndex, bestSiteIndex)){
                    // calculate cost 
                    double coox = targetBin->rows[bestRowIndex]->start_x + bestSiteIndex*targetBin->rows[bestRowIndex]->site_w;
                    double cooy = targetBin->rows[bestRowIndex]->start_y;
                    double cost = pow(f->coox - coox, 2) + pow(f->cooy - cooy, 2);

                    // compare with globalMincost
                    if(cost < globalMincost){
                        globalMincost = cost;
                        bestBin          = targetBin;
                        bestBinRowIndex  = bestRowIndex;
                        bestBinSiteIndex = bestSiteIndex;
                    }
                }
            }
        }


        // Left
        if(minj >= 0){
            int starti   = (mini < 0) ? 0 : (mini + 1);
            int endi = (maxi < mapHeight) ? (maxi - 1) : (mapHeight - 1); 

            for(int i=starti; i<=endi; i++){
                Bin* targetBin = binMap[i][minj];
                if(targetBin->FindAvailable(f, bestRowIndex, bestSiteIndex)){
                    // calculate cost 
                    double coox = targetBin->rows[bestRowIndex]->start_x + bestSiteIndex*targetBin->rows[bestRowIndex]->site_w;
                    double cooy = targetBin->rows[bestRowIndex]->start_y;
                    double cost = pow(f->coox - coox, 2) + pow(f->cooy - cooy, 2);

                    // compare with globalMincost
                    if(cost < globalMincost){
                        globalMincost = cost;
                        bestBin          = targetBin;
                        bestBinRowIndex  = bestRowIndex;
                        bestBinSiteIndex = bestSiteIndex;
                    }
                }
            }
        }
    
        if(globalMincost != numeric_limits<double>::max()){
            bestBin->PlaceFFAt(f, bestBinRowIndex, bestBinSiteIndex);
            return true;
        }

        expansion++;
    }
    return false;
}

bool Legalizer::cmpBin(Bin* a, Bin* b){
    return a->distanceToCentroid < b->distanceToCentroid;
}

Bin::Bin(int index, int rowi, int colj, double bottomLeftX, double bottomLeftY, double upperRightX, double upperRightY, dieInfo* DIE){
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
    toBePlacedFFs.clear();
    placeSuccessFFs.clear();
    placeFailFFs.clear();
    failSizeHistory.clear();
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

    if(leftBin != NULL){
        leftBin->rows[rowNum]->rightRow = newRow;
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

void Bin::AddBlock(double startx, double starty, double width, double height){
    double endx = startx + width;
    double endy = starty + height;

    if(endx > rowEndX && rightBin != NULL){
        rightBin->AddBlock(rowEndX, starty, endx - rowEndX, height);
        endx = rowEndX;
        width = rowEndX - startx;
    }

    if(endy > rowEndY && upBin != NULL){
        upBin->AddBlock(startx, rowEndY, width, endy - rowEndY);
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

bool Bin::TryToLegalizeFF(ffi* f){
    int bestRowIndex;
    int bestSiteIndex;

    if(FindAvailable(f, bestRowIndex, bestSiteIndex)){
        PlaceFFAt(f, bestRowIndex, bestSiteIndex);
        return true;
    }
    else{   
        return false;
    }
}

void Bin::PlaceFFAt(ffi* f, int bestRowIndex, int bestSiteIndex){
    double startx = rowStartX + rows[bestRowIndex]->site_w*bestSiteIndex;
    double starty = rows[bestRowIndex]->start_y;
    double width  = f->type->size_x;
    double height = f->type->size_y;
    AddBlock(startx, starty, width, height);
    f->coox = startx;
    f->cooy = starty;
    f->index_to_placement_row = bestRowIndex;
    f->index_to_site          = bestSiteIndex;
    f->update_pin_loc();
    return;
}

bool Bin::FindAvailable(ffi* f, int& bestRowIndex, int& bestSiteIndex){
    bool findUp   = true;
    bool findDown = true;
    int upRow_i; 
    int downRow_i;
    double globalMincost = numeric_limits<double>::max();
    double rowIdealMincost;


    if(matchFailSizeHistory(f)){
        return false;
    }


    // Initial upRow_i and downRow_i
    upRow_i   = (f->cooy - rowStartY)/rows[0]->site_h;
    if(upRow_i >= rowNum){
        upRow_i = rowNum - 1;
    }
    else if(upRow_i < 0){
        upRow_i = 0;
    }  
    downRow_i = upRow_i - 1;
    

    while(findUp || findDown){
        if(upRow_i >= rowNum){
            findUp = false;
        }
        if(downRow_i < 0){
            findDown = false;
        }

        if(findUp){
            rowIdealMincost = abs(f->cooy - rows[upRow_i]->start_y);
            findUp = (rowIdealMincost >= globalMincost) ? false:true;
        }
        if(findDown){
            rowIdealMincost = abs(f->cooy - rows[downRow_i]->start_y);
            findDown = (rowIdealMincost >= globalMincost) ? false:true;
        }

        if(findUp){
            rows[upRow_i]->PlaceTrial(f, bestRowIndex, bestSiteIndex, globalMincost, *DIE);
            upRow_i++;
        }
        if(findDown){
            rows[downRow_i]->PlaceTrial(f, bestRowIndex, bestSiteIndex, globalMincost, *DIE);
            downRow_i--;
        }
    }

    if(globalMincost == numeric_limits<double>::max()){
        // Record this type to the fail history list
        FailSize* failSize = new FailSize;
        failSize->size_x = f->type->size_x;
        failSize->size_y = f->type->size_y;
        failSizeHistory.push_back(failSize);
        return false;
    }
    else{
        return true;
    }
}

void Bin::LegalizeFFList(){
    placeSuccessFFs.clear();
    placeFailFFs.clear();


    // calculate FF to Die Centroid distance
    for(auto f: toBePlacedFFs){
        f->distanceToDieCentoid = pow(f->coox - DIE->cenx, 2) + pow(f->cooy - DIE->ceny, 2);
    }

    // sort by distance to die centroid
    toBePlacedFFs.sort(cmpFF);
    
    while(toBePlacedFFs.empty() == false){
        auto f = toBePlacedFFs.front();
        toBePlacedFFs.pop_front();

        if(TryToLegalizeFF(f)){
            placeSuccessFFs.push_back(f);
        }
        else{
            placeFailFFs.push_back(f);
        }
    }

    // find other bins to place for the placeFailFFs
    while(placeFailFFs.empty()){
        auto f = placeFailFFs.front();
        placeFailFFs.pop_front();
    }

    finishLegalization = true;
    return;
}


bool Bin::matchFailSizeHistory(ffi* f){
    for(auto failSize: failSizeHistory){
        if(f->type->size_x == failSize->size_x && f->type->size_y == failSize->size_y){
            return true;
        }
    }
    return false;
}

bool Bin::cmpFF(ffi* a, ffi* b){
    return a->distanceToDieCentoid < b->distanceToDieCentoid;
}

void Bin::DeleteFFBlock(ffi* f){
    double ffStart = f->coox;
    double ffEnd   = f->coox + f->type->size_x;

    if(ffEnd > rowEndX){
        if(rows[f->index_to_placement_row]->rightRow == NULL){
            rows[f->index_to_placement_row]->delete_ff(ffStart, rowEndX, f->type->size_y);
        }
        else{
            rows[f->index_to_placement_row]->delete_ff(ffStart, rowEndX, f->type->size_y);
            rows[f->index_to_placement_row]->rightRow->delete_ff(rowEndX, ffEnd, f->type->size_y);
        }
    }
    else{
        rows[f->index_to_placement_row]->delete_ff(ffStart, ffEnd, f->type->size_y);
    }
    return;
}

void Bin::FillTrivialGap(double gapWidth){
    for(auto r: rows){
        r->FillGap(gapWidth);
    }
    return;
}

