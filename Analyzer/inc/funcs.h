#ifndef _FUNC_H_
#define _FUNC_H_

#include <bits/stdc++.h>

#include "lib.h"
#include "inst.h"
#include "netlist.h"
#include "placement.h"



void ReadInput(string , lib&, inst&, dieInfo&, netlist&, placement&);
void ReadOutput(string, lib&, inst&, dieInfo&, netlist&, placement&, list<ffi*>&);
void Output(string , list<ffi*>& , inst& );
void DrawFFs(dieInfo&, lib&, inst&, list<ffi*>&);
void DrawGates(dieInfo&, lib&, inst&);
void Analyzer(lib&, inst&, dieInfo&, netlist&, list<ffi*>&);

#endif