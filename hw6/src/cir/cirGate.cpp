/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
//CIRGate <int gateId>
void
CirGate::reportGate() const
{
   cout << "==================================================" << endl;
   string p = "";
   if (_type == PI_GATE || _type == PO_GATE) {
      p = getName();
      if (p != "") p = "\"" + p + "\"";
   }
   string s = "= " + getTypeStr() + "(" + to_string(getId()) + ")" + p + 
      ", line " + to_string(getLineNo());
   cout << s << right << setw(49-s.size()) << "=" << endl;
   cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
}

