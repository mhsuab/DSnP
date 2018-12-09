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

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(){}
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return 0; }

   // Member functions about circuit constructions
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;

private:
   vector<CirGate*> _piList;
   vector<CirGate*> _poList;
   vector<CirGate*> _totalList;
   vector<CirGate*> _dfsList;
   mutable int _m, _i, _l, _o, _a;

   int s2i(const string& str) {
       int n = 0;
       for (size_t i = 0; i < str.size(); ++i) {
	   n *= 10;
	   n += int(str[i] - '0');
       }
       return n;
   }

   void linkAIG(const CirGate* aig);
   void linkPO(const CirGate*);

};

#endif // CIR_MGR_H
