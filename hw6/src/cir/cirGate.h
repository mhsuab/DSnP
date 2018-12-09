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
   CirGate(GateType t = PI_GATE, size_t id = 0, size_t lineno = 0, string f = ""):
       _type(t), _faninList({}), _sign({}), _fanoutList({}), _gateId(id), _lineNo(lineno), _name(""), _infile(f) {}
   CirGate(GateType t = UNDEF_GATE, vector<CirGate*> fanin = {}, vector<bool> sign = {}, vector<CirGate*> fanout = {}, size_t id = 0, size_t lineno = 0, string n = "", string f = ""):
       _type(t), _faninList(fanin), _sign(sign), _fanoutList(fanout), _gateId(id), _lineNo(lineno), _name(n), _infile(f) {}
   ~CirGate() {}

   // Basic access methods
   string getTypeStr() const {
      switch (_type) {
	 case UNDEF_GATE:
	    return "UNDEF";
	    break;
	 case PI_GATE:
	    return "PI";
	    break;
	 case PO_GATE:
	    return "PO";
	    break;
	 case AIG_GATE:
	    return "AIG";
	    break;
	 case CONST_GATE:
	    return "CONST";
	    break;
	 default: break;
      }
   }
   size_t getLineNo() const { return _lineNo; }
   size_t getId() const { return _gateId; }
   string getName() const { return _name; }
   string getLine() const { return _infile; }

   // Printing functions
   //virtual void printGate() const = 0;
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   //void setFANIn(const CirGate* fanin, bool inv) const;
   //void setFANOut(const CirGate* fanout) const;

private:
   GateType		_type;
   //GateFlag 		_flag;
   vector<CirGate*> 	_faninList;
   vector<bool> 	_sign;
   vector<CirGate*> 	_fanoutList;
   size_t 		_gateId;
   size_t 		_lineNo;
   string 		_name;
   string 		_infile;

protected:

};

#endif // CIR_GATE_H
