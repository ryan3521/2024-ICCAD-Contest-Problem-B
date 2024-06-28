#ifndef _FUNC_H_
#define _FUNC_H_

#include <bits/stdc++.h>

#include "lib.h"
#include "inst.h"
#include "netlist.h"
#include "cluster.h"
#include "placement.h"



void ReadInput(string , lib&, inst&, dieInfo&, netlist&, placement&);
void KmeansCls(dieInfo&, lib&, inst&, list<cluster*>&, list<ffi*>&);
void MapClstoMBFF(lib&, list<cluster*>&, list<ffi*>&);
void FineTune(lib&, list<ffi*>&, list<ffi*>&, list<ffi*>&, dieInfo&);
void Output(string , list<ffi*>& , inst& );

#endif