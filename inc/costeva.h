#ifndef _COSTEVA_H_
#define _COSTEVA_H_

#include <iomanip>
#include <fstream>

#include "lib.h"
#include "inst.h"
#include "die_info.h"

using namespace std;

class bin{
    public:
        double xmax;
        double ymax;
        double coox;
        double cooy;
        double bin_area;
        double acc_area;
        double util;
        bin* up_bin;
        bin* next_bin;

        bin(double width, double height, double util, double coox, double cooy);
        bool add_cell(double coox, double cooy, double sizex, double sizey);
        bool check_violation();
};

class costeva{
    private:
        dieInfo* DIE;
        lib* LIB;
        inst* INST;
        list<ffi*>* ffs;
        string filename;
        fstream fout;


        double aproxOriTns();
        double aproxTns();
        double calOriTns();
        double calArea();
        double calPower();
        double calTns(double* WNS);
        int    calDensity();


        double get_ct(gatei* g);

    public:
        costeva(dieInfo* DIE, lib* LIB, inst* INST, string testcase_filename);
        ~costeva();
        void InitialCost();
        void ResultCost(list<ffi*>* ff_list);
        void PrintParameter();    
};

#endif