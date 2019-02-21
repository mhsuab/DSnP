/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include <unordered_map>
#include <map>
#include <sat.h>

using namespace std;

// TODO: define your own typedef or enum

class CirGate;
class CirMgr;
class SatSolver;

typedef vector<CirGate*>						GateList;
typedef vector<int>								IdList;
typedef vector<size_t> 							PointerInvList;
typedef unordered_map<size_t, CirGate*> 		StrashHash;
typedef unordered_map<size_t, PointerInvList*> 	SimHash;
typedef map<CirGate*, Var>	 					fraigMap;

enum GateType
{
	UNDEF_GATE = 0, 
	PI_GATE = 1,
	PO_GATE = 2,
	AIG_GATE = 3,
	CONST_GATE = 4,

	TOT_GATE
};

#endif // CIR_DEF_H
