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
#include <vector>

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   cout << "==================================================" << endl;
   string p = "";
   if (_type == PI_GATE || _type == PO_GATE) {
      p = getName();
      if (p != "") p = "\"" + p + "\"";
   }
   string s = "= " + getTypeStr() + "(" + to_string(getId()) + ")" + p + ", line " + to_string(getLineNo());
   cout << s << right << setw(50-(s.size())) << "=" << endl;

   cout << "==================================================" << endl;
}

void
CirGate::printGate() const
{
   switch(_type) {
      case UNDEF_GATE:
         break;
      case PI_GATE: {
         if (_name == "") cout << "PI  " << _gateId << endl;
         else cout << "PI  " << _gateId << " (" << _name << ")" << endl;
         break;
      }
      case PO_GATE: {
         cout << "PO  " << _gateId << " ";
         string k = (_name == "")? (""):(" (" + _name + ")");
         switch (_inSign[0]) {
            case '0':
               cout << "!" << _in[0]->getId() << k;
               break;
            case '1':
               cout << "*!" << _in[0]->getId() << k;
               break;
            case '2':
               cout << _in[0]->getId() << k;
               break;
            case '3':
               cout << "*" << _in[0]->getId() << k;
               break;
            default: break;
         }
         cout << endl;
         break;
      }
      case AIG_GATE: {
         cout << "AIG " << _gateId;
         switch (_inSign[0]) {
            case '0':
               cout << " !" << _in[0]->getId();
               break;
            case '1':
               cout << " *!" << _in[0]->getId();
               break;
            case '2':
               cout << " " << _in[0]->getId();
               break;
            case '3':
               cout << " *" << _in[0]->getId();
               break;
            default: break;
         }
         switch (_inSign[1]) {
            case '0':
               cout << " !" << _in[1]->getId();
               break;
            case '1':
               cout << " *!" << _in[1]->getId();
               break;
            case '2':
               cout << " " << _in[1]->getId();
               break;
            case '3':
               cout << " *" << _in[1]->getId();
               break;
            default: break;
         }
         cout << endl;
         break;
      }
      case CONST_GATE: {
         cout << "CONST0" << endl;
         break;
      }
      default: break;
   }
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   ++GlobalRef;
   subFanIn(level, 0, 0);
}

void
CirGate::subFanIn(int level, int step, bool inv) const
{
   if (level == 0) simplePrint(inv, 0, step);
   else if (isGlobalRef()) simplePrint(inv, 1, step);
   else {
      simplePrint(inv, 0, step);
      if (_inSign[0]) {
      setToGlobalRef();
      --level; ++step;
         _in[0]->subFanIn(level, step, (_inSign[0]=='0'||_inSign[0]=='1'));
         if (_inSign[1]) _in[1]->subFanIn(level, step, (_inSign[1]=='0'||_inSign[1]=='1'));
      }
   }
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   ++GlobalRef;
   subFanOut(level, 0, 0);
}

void
CirGate::subFanOut(int level, int step, const CirGate* prev) const
{
   bool inv;
   if (prev != 0) {
      int w = (prev == _in[0])? 0:1;
      inv = (_inSign[w] == '0' || _inSign[w] == '1');
   }
   else inv = false;
   if (level == 0) simplePrint(inv, 0, step);
   else if (isGlobalRef()) simplePrint(inv, 1, step);
   else {
      simplePrint(inv, 0, step);
      if (!_out.empty()) {
         setToGlobalRef();
         --level; ++step;
         for (GateList::const_iterator it = _out.begin(); it != _out.end(); ++it) { (*it)->subFanOut(level, step, this); }
      }
   }
}


void
CirGate::setIn(CirGate* ingate, int index, char inv)
{
   _in[index] = ingate;
   _inSign[index] = inv;
   //_in.push_back(ingate);
   //_inSign.push_back(inv);
}

void
CirGate::setOut(CirGate* outgate)
{
   _out.push_back(outgate);
}

void
CirGate::dfsNet(GateList& dfsList, GateList& aigList)
{
   if (isGlobalRef()) return;
   switch(_type) {
      case PI_GATE:
         dfsList.push_back(this);
         setToGlobalRef();
         break;
      case PO_GATE:
         _in[0]->dfsNet(dfsList, aigList);
         dfsList.push_back(this);
         setToGlobalRef();
         break;
      case AIG_GATE:
         _in[0]->dfsNet(dfsList, aigList);
         _in[1]->dfsNet(dfsList, aigList);
         dfsList.push_back(this);
         aigList.push_back(this);
         setToGlobalRef();
         break;
      case CONST_GATE:
         dfsList.push_back(this);
         setToGlobalRef();
         break;
      case UNDEF_GATE:
         break;
      default: break;
   }
}

