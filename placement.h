#ifndef _PLACEMENT_H_
#define _PLACEMENT_H_

#include <vector>
#include <list>
#include <cmath>
#include <ctime>
#include <utility>

#include "inst.h"
#include "die_info.h"

using namespace std;


class plcmt_row{
    private:
        dieInfo* DIE;

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
        list<plcmt_row*> up_rows;
        list<plcmt_row*> down_rows;


        list<gatei*> glist; // store the gate instances which were placed in this row;
        list<pair<int, int>> space_list; // the available space before pivot

        // Member functions
        plcmt_row(dieInfo* DIE, double sx, double sy, double sw, double sh, int sn);
 
        bool add_gblock(double start, double end);
        void add_fblock(double start, double end);

        void add_ff(double start, double end, double height);
        bool check_available(double start, double end, double height);
        bool height_available(double height);
        bool x_overlapped(double x1, double x2, bool& fit); // x2 must >= x1
        bool y_overlapped(double y1, double y2); // y2 must >= y1
        bool x_inrange(double x1, double x2);
        bool y_inrange(double y1, double y2);
        double closest_x(double x);




        // given space [ds, de], if this space can fined space to place, return true and the x coor displace cost
        // otherwise reture false.
        // if dir == 0 start finding from ds to de (start to end)
        // if dir == 1 start finding from de to ds (end to start)
        bool seg_mincost(ffi* fi, int ds, int de, int dw, int& best_pos_idx, double& mincost, bool dir);
        double place_trial(ffi* fi, bool& available, int& best_pos_idx, double global_mincost);
        
};

class placement{
    private:
        dieInfo* DIE;
        lib* LIB;
        inst* INST;
        int new_ff_cnt;
        list<plcmt_row*> temp_rows; 
        list<plcmt_row*> virtual_rows;    

        // Member function   
        int  closest_IDX(double x, double y);
        static bool ff_cmp(ffi* a, ffi* b);
        static bool row_cmp(plcmt_row* a, plcmt_row* b);
        void place_formal(ffi* fi, plcmt_row* best_row, int best_pos_idx);


    
    public:
        vector<plcmt_row*> rows;

        // Member functions
        placement(lib* LIB, inst* INST, dieInfo* DIE);
        void addRow(double sx, double sy, double sw, double sh, int sn);
        void initial();
        void placeGateInst(double x, double y, double w, double h);
        void GatePlacement();
        bool placeFlipFlop(ffi* f, bool set_constrain, double displace_constrain);
        


};

#endif

