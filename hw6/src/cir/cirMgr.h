/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr():
      _totalList{}, _dfsList{}, _AIGdfsList{}, _m(0), _i(0), _l(0), _o(0), _a(0) {}
   ~CirMgr() {
      for (GateList::const_iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
         delete (*it);
      }
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(int gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);
   bool readError(const string&) const;

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;
   //static size_t GlobalRef;

private:
   GateList _totalList;
   map<int, CirGate*> _sortList;
   mutable GateList _dfsList, _AIGdfsList;
   int _m, _i, _l, _o, _a;
   
   void linkPO(CirGate*, int);
   void linkAIG(CirGate*, int, int);

   void dfs() const;

};

#endif // CIR_MGR_H
