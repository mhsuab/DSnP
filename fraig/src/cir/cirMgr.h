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

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr():
      _totalList{}, _dfsList{}, _AIGdfsList{}, _m(0), _i(0), _l(0), _o(0), _a(0), _dfsMark(0), _fec{} {}
   ~CirMgr() {
      for (GateList::const_iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
         delete (*it);
      }
   } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   // CirGate* getGate(unsigned gid) const { return 0; }
   CirGate* getGate(int gid) const {
      map<int, CirGate*>::const_iterator it = _sortList.find(gid);
      if (it == _sortList.end()) return 0;
      return (it)->second;
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   static size_t GlobalDfs;
   bool isGlobalDfs() { return (_dfsMark == GlobalDfs); }
   void settoGlobalDfs() { _dfsMark = GlobalDfs; }

private:
   GateList _totalList;
   map<int, CirGate*> _sortList;
   mutable GateList _dfsList, _AIGdfsList;
   int _m, _i, _l, _o, _a;
   mutable size_t _dfsMark;
   vector<PointerInvList*> _fec;

   ofstream           *_simLog;

   void linkPO(CirGate*, int);
   void linkAIG(CirGate*, int, int);

   void dfs(bool forced = false) const;

   void replace(CirGate*, CirGate*, bool, bool);

   void simAllGate();
   void sim();
   void resetSim();
   void sortFEC();
   void setFEC();
   void writeSim(size_t);

   static bool compareSubFEC(size_t& i, size_t& j) { return ((CirGate*)((i/2)*2))->getId() < ((CirGate*)((j/2)*2))->getId();}
   static bool compareFEC(PointerInvList*& i, PointerInvList*& j) { return ((CirGate*) ((((*i)[0])/2)*2))->getId() < ((CirGate*) ((((*j)[0])/2)*2))->getId(); }

   void merge(SimHash);

};

#endif // CIR_MGR_H
