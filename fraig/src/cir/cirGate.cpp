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
#include <bitset>

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   cout << "================================================================================" << endl;
   string p = "";
   if (_type == PI_GATE || _type == PO_GATE) {
      p = getName();
      if (p != "") p = "\"" + p + "\"";
   }
   cout << "= " << getTypeStr() << "(" << getId() << ")" << p << ", line " << getLineNo() << endl;
   //string s = "= " + getTypeStr() + "(" + to_string(getId()) + ")" + p + ", line " + to_string(getLineNo());
   //cout << s << endl;
   //cout << s << right << setw(50-(s.size())) << "=" << endl;
   cout << "= FECs:";
   for (auto it = _fecGate.begin(); it != _fecGate.end(); ++it) {
      if ((*it)<0) cout << " !" << (-1) * (*it);
      else cout << " " << (*it);
   }
   cout << endl;
   cout << "= Value: ";
   p = (bitset<64>(_simVal)).to_string();
   for (size_t i = 0; i < p.size(); ++i) {
      if (i%8 == 0 && i != 0) cout << "_";
      cout << p[63 - i];
   }
   cout << endl;
   cout << "================================================================================" << endl;
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
CirGate::dfsNet(GateList& dfsList, GateList& aigList, bool need)
{
   //need == false, PI_list, AIGlist
   if (isGlobalRef()) return;
   switch(_type) {
      case PI_GATE:
         dfsList.push_back(this);
         setToGlobalRef();
         _dfs = true;
         break;
      case PO_GATE:
         _in[0]->dfsNet(dfsList, aigList, need);
         if (need) dfsList.push_back(this);
         setToGlobalRef();
         _dfs = true;
         break;
      case AIG_GATE:
         _in[0]->dfsNet(dfsList, aigList, need);
         _in[1]->dfsNet(dfsList, aigList, need);
         if (need) dfsList.push_back(this);
         _dfs = true;
         aigList.push_back(this);
         setToGlobalRef();
         break;
      case CONST_GATE:
         if (need) dfsList.push_back(this);
         setToGlobalRef();
         _dfs = true;
         break;
      case UNDEF_GATE:
         _dfs = true;
         break;
      default: break;
   }
}

/*
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
*/

/*
bool
CirGate::OptCase(CirGate*& prev, bool& zero, bool& inverse)
{
   cerr << "0 gateid" << _in[0]->getId() << endl;
   cerr << "0 sign" << _inSign[0] << endl;
   cerr << "1 gateid" << _in[1]->getId() << endl;
   cerr << "1 sign" << _inSign[1] << endl;
   if (_in[0]->getType() == CONST_GATE) {
      //cerr << "ccc" << endl;
      if (inv(0)) {
         cerr << "case a:0" << endl; 
         prev = _in[1];
         zero = 1;
         if (inv(1) == true) inverse = 1;
         else inverse = 0;
         return true;
      }
      else {
         cerr << "case b:0" << endl;
         prev = _in[0];
         zero = 0;
         inverse = 0;
         return true;
      }
   }
   else if (_in[1]->getType() == CONST_GATE) {
      if (inv(1)) {
         cerr << "case a:1" << endl;
         prev = _in[0];
         zero = 1;
         if (inv(0)) inverse = 1;
         else inverse = 0;
         return true;
      }
      else {
         cerr << "case b:1" << endl;
         prev = _in[1];
         zero = 0;
         inverse = 0;
         return true;
      }
   }
   else if (_in[0] == _in[1]) {
      if (inv(0) == inv(1)) {
         //cerr << "case c" << endl;
         prev = _in[0];
         zero = 1;
         if (inv(0) == true) inverse = 1;
         else inverse = 0;
         return true;
      }
      else {
         //cerr << "case d" << endl;
         prev = _in[0];
         zero = 0;
         return true;
      }
   }
   return false;
}
*/

void
CirGate::removeOut(CirGate* s)
{
   for (GateList::iterator it = _out.begin(); it != _out.end(); ++it) {
      if ((*it) == s) {
         _out.erase(it);
         --it;
      }
   }
}

/*
void
CirGate::removeInofOut(CirGate* updatein, bool inve)
{
   int undef = (updatein->unDef())? 1:0;
   char inverse;
   for (GateList::iterator it = _out.begin(); it != _out.end(); ++it) {
      //assert((*it)->getType() == AIG_GATE);
      if ((*it)->getFanIn(0) == this) {
         inverse = (inve == inv(0))? ((2 + undef) + '0'):((0 + undef) + '0');
         (*it)->setIn(updatein, 0, inverse);
      }
      else if ((*it)->getFanIn(1) == this) {
         inverse = (inve == inv(1))? ((2 + undef) + '0'):((0 + undef) + '0');
         (*it)->setIn(updatein, 1, inverse);
      }
   }
}
*/

void
CirGate::removeInofOut(CirGate* prev, bool inverse)
{
   int undef = (prev->unDef())? 1:0;
   for (GateList::iterator it = _out.begin(); it != _out.end(); ++it) {
      if ((*it)->getFanIn(0) == this) {
         (*it)->setIn(prev, 0, (inverse == (*it)->inv(0))? ((2 + undef) + '0'):(0 + undef) + '0');
      }
      else if ((*it)->getFanIn(1) == this) {
         (*it)->setIn(prev, 1, (inverse == (*it)->inv(1))? ((2 + undef) + '0'):(0 + undef) + '0');
      }
   }
}

bool
CirGate::OptCase(CirGate*& prev, bool& zero, bool& inverse)
{
   if (_in[0]->getType() == CONST_GATE) {
      if (inv(0)) {
         zero = 0;
         prev = _in[1];
         inverse = inv(1);
         prev->removeOut(this);
         prev->addOutList(_out);
         removeInofOut(prev, inverse);
         return true;
      }
      else {
         //replace by const0
         zero = 1;
         prev = _in[1];
         inverse = 0;
         prev->removeOut(this);
         return true;
      }
   }
   else if (_in[1]->getType() == CONST_GATE) {
      if (inv(1)) {
         zero = 0;
         prev = _in[0];
         inverse = inv(0);
         prev->removeOut(this);
         prev->addOutList(_out);
         removeInofOut(prev, inverse);
         return true;
      }
      else {
         zero = 1;
         prev = _in[0];
         inverse = 0;
         prev->removeOut(this);
         return true;
      }
   }
   else if (_in[0] == _in[1]) {
      if (inv(0) == inv(1)) {
         zero = 0;
         prev = _in[1];
         inverse = inv(0);
         prev->removeOut(this);
         prev->addOutList(_out);
         removeInofOut(prev, inverse);
         return true;
      }
      else {
         zero = 1;
         prev = _in[0];
         inverse = 0;
         prev->removeOut(this);
         return true;
      }
   }
   return false;
}












