#ifndef _BANKING_H_
#define _BANKING_H_

#include "inst.h"
#include "lib.h"
#include "cluster.h"
#include "placement.h"
#include "legalizer.h"


#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// Define a point and rectangle type using Boost Geometry
typedef bg::model::point<float, 2, bg::cs::cartesian> Point;
typedef bg::model::box<Point> Rectangle;


// Define a value type for R-tree
typedef std::pair<Rectangle, ffi*> Value;

class banking{
    private:
        bool SUCCESS;
        bool FAIL;
// ----------------------------------------
        placement* PM;
        Legalizer* LG;
        inst* INST;
        lib*  LIB;
        dieInfo* DIE;
        list<ffi*>* PFFS;
// ----------------------------------------
        list<list<ffi*>*> ff_groups;
        list<ffi*> placing_ffs;
        list<ffi*> banking_ffs;
        list<se*>  xseq;
        list<se*>  yseq;
        list<ffi*> tracking_list;
        list<ffi*> testList;
        bgi::rtree<Value, bgi::quadratic<16>> rtree;
        
        cluster* cls;
        int target_size;
        ffi* essential_ff;
        comb* target_comb;
        int nameCount = 0;
// ----------------------------------------
        static bool cmp_ff_x(ffi* a, ffi* b);
        static bool cmp_ff_y(ffi* a, ffi* b);
        static bool cmp_ff_slack(ffi* a, ffi* b);
        static bool cmp_ff(ffi* a, ffi* b);
// ----------------------------------------
        void CopyOriginalFFs();
        void InitialFFsCost();
        void RunBanking();
        void RenameAllFlipFlops();
        void Debank(ffi* f, list<ffi*>& debank_list);
        void ConstructRtree();
        bool TestCluster(int targetSize, vector<Value>& nearest_result);
        double totalDegradation = 0;
    public:
        banking(placement* PM, inst* INST, lib* LIB, dieInfo* DIE, list<ffi*>* PFFS);
        void run();
        

};



#endif