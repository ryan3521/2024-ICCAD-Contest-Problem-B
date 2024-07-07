#ifndef INTEGRA_H
#define INTEGRA_H
#include "inst.h"
#include "netlist.h"
#include <queue>
#include <cfloat>

struct Vertex{
    Vertex();
    Vertex(double x_in, double y_in);
    ~Vertex();

    double x;
    double y;
};

struct Rectangle{
    double x;
    double y;
    double w;
    double h;
};

class Diamond{
public:
    Diamond();
    Diamond(vector<Vertex> c);
    ~Diamond();

    Rectangle rotate();

private:
    vector<Vertex> corners; // [0]: left
                            // [1]: bot
                            // [2]: right
                            // [3]: top
};

class INTEGRA{
public:
    INTEGRA();
    INTEGRA(inst I, netlist N);
    ~INTEGRA();

    void run();
    double calHPWL(pin* from, pin* to);
    void findTopFF();
    void calMovableRegion();
    Rectangle calFeasibleRegion(list<Diamond> movableRegions);


private:
    inst INST;
    netlist NL;
    list<ffi*> topFFs;
    list<Rectangle> feasRegions;
};


#endif