#ifndef _PLACEMENT_H_
#define _PLACEMENT_H_

#include <vector>
#include <list>
#include <cmath>
#include <ctime>
#include <utility>
#include <iterator>

#include "inst.h"
#include "die_info.h"

using namespace std;


class PlacementRow{
    private:
        struct Dummy{
            int start;
            int end;
        };
        list<Dummy*> allDummy;



    public:
        int idx;
        bool is_tested;
        bool is_visited;
        double start_x;
        double start_y;
        double xmax;
        double ymax;
        double site_w;
        double site_h;
        int site_num;
        PlacementRow* upRow;
        PlacementRow* downRow;
        PlacementRow* rightRow;
        list<gatei*> glist;              // store the gate instances which were placed in this row;
        list<pair<int, int>> space_list; // the available space before pivot

        // Member functions
        PlacementRow(double sx, double sy, double sw, double sh, int sn);
 
        void FillGap(double gapWidth);
        int  FillDummy(double width);
        void ClearDummy();


        void AddBlockAnyway(double start, double end);
        void add_fblock(double start, double end);
        void add_ff(double start, double end, double height);
        void delete_ff(double start, double end, double height);
        void delete_fblock(double start, double end);
        bool check_available(double start, double end, double height3);
        bool height_available(double height);
        bool x_overlapped(double x1, double x2, bool& fit); // x2 must >= x1
        bool y_overlapped(double y1, double y2); // y2 must >= y1
        bool x_inrange(double x1, double x2);
        bool y_inrange(double y1, double y2);
        double closest_x(double x);


        void print_spacelist();


        // given space [ds, de], if this space can fined space to place, return true and the x coor displace cost
        // otherwise reture false.
        // if dir == 0 start finding from ds to de (start to end)
        // if dir == 1 start finding from de to ds (end to start)
        bool seg_mincost(ffi* fi, int ds, int de, int dw, int& best_pos_idx, double& mincost, bool dir, bool set_constrain);
        double place_trial(ffi* fi, bool& available, int& best_pos_idx, double global_mincost, bool set_constrain);
        void PlaceTrial(ffi* f, int& bestRowIndex, int& bestSiteIndex, double& globalMincost, dieInfo& DIE);
        
        // idealcoox, idealcooy: the coordinate using to calculate the displacement cost
        // globalMincost: the global minimum diplacement cost
        // askCoox:
        // replyCoox:
        // width:
        // height:
        // direction;
        bool FindSpaceOrJump(double idealcoox, double idealcooy,
                             double globalMincost, 
                             double askCoox, double askCooy, double& replyCoox, 
                             double width, double height, 
                             bool direction,
                             dieInfo& DIE);
};  

class placement{
    private:
        dieInfo* DIE;
        lib* LIB;
        inst* INST;
        int new_ff_cnt;
        list<PlacementRow*> temp_rows; 

        // Member function   
        int  closest_IDX(double x, double y);
        static bool ff_cmp(ffi* a, ffi* b);
        static bool row_cmp(PlacementRow* a, PlacementRow* b);
        void place_formal(ffi* fi, PlacementRow* best_row, int best_pos_idx);
    
    public:
        vector<PlacementRow*> rows;

        // Member functions
        placement(lib* LIB, inst* INST, dieInfo* DIE);
        void addRow(double sx, double sy, double sw, double sh, int sn);
        void initial();
        



        /* below may not be need after Legalizer is construceted. */
        void placeGateInst(gatei* g, double x, double y, double w, double h);
        void GatePlacement();
        bool placeFlipFlop(ffi* f, bool set_constrain, double displace_constrain);
        void DeleteFlipFlop(ffi* f);
        void PlaceBackFlipFlop(ffi* f);
        void FillDummy(double minWidth);
        void ClearDummy();

};

#endif

