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
   virtual ~CirGate() {}

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
   unsigned getLineNo() const { return _lineNo; }
   unsigned getId() const { return _gateId; }
   string getName() const { return _name; }

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

private:
   GateType		_type;
   //GateFlag 		_flag;
   vector<CirGate*> 	_faninList;
   vector<CirGate*> 	_fanoutList;
   unsigned 		_gateId;
   unsigned 		_lineNo;
   string 		_name;

protected:

};

#endif // CIR_GATE_H
