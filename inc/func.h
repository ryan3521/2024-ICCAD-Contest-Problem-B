#ifndef _FUNC_H_
#define _FUNC_H_

#include <bits/stdc++.h>

#include "lib.h"
#include "inst.h"
#include "netlist.h"
#include "placement.h"



void ReadInput(string , lib&, inst&, dieInfo&, netlist&, placement&);
void Output(string , list<ffi*>& , inst& );
void DrawFFs(dieInfo&, lib&, inst&, list<ffi*>&, list<ffi*>&);
void DrawGates(dieInfo&, lib&, inst&);

#endif