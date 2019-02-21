/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
#include <algorithm>
#include <bitset>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   CirGate(){}
   CirGate(GateType t, int id, int line):
      _type(t), _in{}, _out({}), _inSign{}, _gateId(id), _lineNo(line), _name(""), _mark(0), _dfs(0), _simVal(0), _fecGate{} {}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const {
      string s;
      switch (_type) {
         case UNDEF_GATE:
            s = "UNDEF"; break;
         case PI_GATE:
            s = "PI"; break;
         case PO_GATE:
            s = "PO"; break;
         case AIG_GATE:
            s = "AIG"; break;
         case CONST_GATE:
            s = "CONST"; break;
         default: break;
      }
      return s;
   }
   int getLineNo() const { return _lineNo; }
   int getId() const { return _gateId; }
   string getName() const { return _name; }
   bool isAig() const { return (_type == AIG_GATE); }
   CirGate* getFanIn(int i) const { return _in[i];}
   GateType getType() const { return _type; }
   GateList getFanOut() const { return _out; }

   // Printing functions
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

   void subFanIn(int, int, bool) const;
   void subFanOut(int, int, const CirGate*) const;

   void setIn(CirGate* ingate, int index, char inv) {
      _in[index] = ingate;
      _inSign[index] = inv;
   }
   void setOut(CirGate* outgate) { _out.push_back(outgate); }
   void addOutList(GateList addition) {
      //_out.insert(_out.end(), addition.begin(), addition.end());
      for (GateList::iterator it = addition.begin(); it != addition.end(); ++it) {
         _out.push_back((*it));
      }
      sort(_out.begin(), _out.end());
      _out.erase(unique(_out.begin(), _out.end()), _out.end());
   }
   void setName(string s) { _name = s; }

   bool unDef() { return (_type == UNDEF_GATE); }
   bool floating() {
      if (_type == AIG_GATE) { if (_in[0]->unDef() || _in[1]->unDef()) return true; }
      else if (_type == PO_GATE) { if (_in[0]->unDef()) return true; }
      return false;
   }
   bool unused() { return (_out.empty()); }
   bool inv(int i) { return (_inSign[i] == '0')||(_inSign[i] == '1'); }

   //dfs
   static size_t GlobalRef;
   void dfsNet(GateList&, GateList&, bool);
   bool isGlobalRef() const { return (_mark == GlobalRef); }
   void setToGlobalRef() const { _mark = GlobalRef; }
   bool inDfs() const { return _dfs; }
   void resetDfs() { _dfs = false; }

   bool operator < (const CirGate& i) const { return (_gateId < i._gateId); }

   //optimization
   bool OptCase(CirGate*&, bool&, bool&);
   void removeOut(CirGate*);
   void removeInofOut(CirGate*, bool);

   //strash
   void changeINofOUT(CirGate* replace, bool bubble = 0) {
      for (GateList::iterator it = _out.begin(); it != _out.end(); ++it) {
         if ((*it)->getFanIn(0) == this) {
            (*it)->changeIN(0, replace, bubble);
         }
         else if ((*it)->getFanIn(1) == this) {
            (*it)->changeIN(1, replace, bubble);
         }
      }
   }
   void changeIN(int index, CirGate* replace, bool bubble) {
      _in[index] = replace;
      inSigninverse(index, bubble);
   }
   void inSigninverse(int index, bool bubble) {
      if (bubble) {
         switch (_inSign[index]) {
            case '0':
               _inSign[index] = '2';
               break;
            case '1':
               _inSign[index] = '3';
               break;
            case '2':
               _inSign[index] = '0';
               break;
            case '3':
               _inSign[index] = '1';
               break;
            default: break;
         }
      }
   }

   //sim
   size_t getSim() { return _simVal; }
   void simPO() {
      size_t n = (inv(0))? ~(_in[0]->getSim()):(_in[0]->getSim());
      updataSim(n);
   }
   void simAIG() {
      size_t n = ((inv(0))? ~(_in[0]->getSim()):(_in[0]->getSim())) & ((inv(1))? ~(_in[1]->getSim()):(_in[1]->getSim()));
      updataSim(n);
   }
   void setPIsim(char c) { _simVal = (_simVal << 1) + (c - '0'); }
   void setPIrandsim(size_t i) { _simVal = i; }
   void updataSim(size_t i) { _simVal = (_simVal << 1) + i; }
   void resetSimVal() { _simVal = 0; }
   void moveto64(int i) { _simVal = _simVal << i; }
   void setFECGate(PointerInvList u) {
      //_fecGate = u;
      bool inv = false;
      for (auto it = u.begin(); it != u.end(); ++it) {
         if (((CirGate*)((*it)/2*2)) != this) {
            if ((*it)%2 == 0) _fecGate.push_back(((CirGate*)((*it)/2*2))->getId());
            else _fecGate.push_back((-1) * ((CirGate*)((*it)/2*2))->getId());
         }
         else {
            if ((*it)%2 == 0) { inv = false; }
            else { inv = true; }
         }
      }
      if (inv) {
         for (auto it = _fecGate.begin(); it != _fecGate.end(); ++it) { (*it) *= (-1); }
      }
   }
   void resetFECGate() { IdList().swap(_fecGate); }
   string SimtoStr() { return (bitset<64>(_simVal)).to_string(); }

private:
   GateType       _type;
   CirGate*       _in[2];
   GateList       _out;
   char           _inSign[2];
   int            _gateId;
   int            _lineNo;
   string         _name;
   mutable size_t _mark;
   bool           _dfs;
   size_t         _simVal;
   IdList         _fecGate;

   void simplePrint(bool inv, bool visited, int step) const {
      // inv true-> !
      // visited true -> (*)
      if (inv && visited) cout << string(2 * step, ' ') << "!" << getTypeStr() << " " << _gateId << " (*)" << endl;
      else if (inv && !visited) cout << string(2 * step, ' ') << "!" << getTypeStr() << " " << _gateId << endl;
      else if (!inv && visited) cout << string(2 * step, ' ') << getTypeStr() << " " << _gateId << " (*)" << endl;
      else cout << string(2 * step, ' ') << getTypeStr() << " " << _gateId << endl;
   }

protected:
};

#endif // CIR_GATE_H
