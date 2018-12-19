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

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   CirGate() {}
   CirGate(GateType t, int id, int line):
      _type(t), _in{}, _out({}), _inSign{}, _gateId(id), _lineNo(line), _name(""), _mark(0) {}
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
   
   CirGate* getFanIn(int i) const { return _in[i]; }


   // Printing functions
   //virtual void printGate() const = 0;
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

   void subFanIn(int, int, bool) const;
   void subFanOut(int, int, const CirGate*) const;

   void setIn(CirGate*, int, char);
   void setOut(CirGate*);
   void setName(string s) { _name = s; }

   bool unDef() { return (_type == UNDEF_GATE); }
   bool floating() {
      if (_type == AIG_GATE) { if (_in[0]->unDef() || _in[1]->unDef()) return true; }
      else if (_type == PO_GATE) {if (_in[0]->unDef()) return true;}
      return false;
   }
   bool unused() { return (_out.empty()); }
   bool inv(int i) { return (_inSign[i] == '0' || _inSign[i] == '1'); }

   //dfs
   static size_t GlobalRef;
   void dfsNet(GateList&, GateList&);
   bool isGlobalRef() const { return (_mark == GlobalRef); }
   void setToGlobalRef() const { _mark = GlobalRef; }

private:
   GateType          _type;
   CirGate*          _in[2];
   GateList          _out;
   char              _inSign[2];
   int               _gateId;
   int               _lineNo;
   string            _name;
   mutable size_t    _mark;

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
