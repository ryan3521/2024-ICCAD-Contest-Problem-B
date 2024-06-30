#include "inst.h"

struct Vertex{
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
    ~Diamond();

    // Rectangle rotate();
    // 
private:
    vector<Vertex> corners; // [0]: left
                            // [1]: bot
                            // [2]: right
                            // [3]: top
};

class MaxClique{
public:
    MaxClique();
    ~MaxClique();

    double calHPWL(pin*,pin*);
    // Diamond calMovableRegion(...);
    // Rectangles calFeasibleRegion(list<Diamond> movableRegions);


private:

};