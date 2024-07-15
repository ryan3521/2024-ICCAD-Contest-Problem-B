#ifndef INTEGRA_H
#define INTEGRA_H
#include "inst.h"
#include "netlist.h"
#include <queue>
#include <cfloat>

struct Rectangle{
    Rectangle();
    Rectangle(double x_in, double y_in, double w_in, double h_in, ffi* FF_in);
    ~Rectangle();

    
    double x;
    double y;
    double w;
    double h;
    ffi* FF;
};

class Diamond{
public:
    Diamond();
    Diamond(double x_in, double y_in, double r);
    ~Diamond();

    double x, y;
    double radius;
};

class INTEGRA{
public:
    // INTEGRA();
    INTEGRA(inst* I, netlist* N);
    ~INTEGRA();

    void run();
    double calHPWL(pin* from, pin* to);
    void findTopFF();
    void calFeasibleRegion();
    
    // added by your friend Ryan!
    void copyFSR();
    


private:
    inst* INST;
    netlist* NL;
    list<ffi*> topFFs;
    vector<Rectangle> feasRegions;
};


#endif