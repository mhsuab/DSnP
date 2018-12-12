/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

size_t CirGate::GlobalRef = 1;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   //handle error case
   if (readError(fileName)) { cirMgr = 0; return false; }
   else {
      ifstream f(fileName);
      if (!f.is_open()) {
         cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
         return false;
      }
      int line = 2, end = line, id, fanin1, fanin2;
      string s;
      f >> s >> _m >> _i >> _l >> _o >> _a;

      CirGate* current;
      int fan[_o + _a * 2];

      //PI
      end += _i;
      for (; line < end; ++line) {
         f >> id;
         current = new CirGate(PI_GATE, id/2, line);
         _totalList.push_back(current);
         _sortList.insert(pair<int, CirGate*>(id/2, current));
      }

      //PO
      end += _o;
      id = _m + 1;
      for (; line < end; ++line) {
         f >> fanin1;
         current = new CirGate(PO_GATE, id, line);
         _totalList.push_back(current);
         _sortList.insert(pair<int, CirGate*>(id, current));
         ++id;
         fan[line - _i -2] = fanin1;
      }

      //AIG
      end += _a;
      for (; line < end; ++ line) {
         f >> id >> fanin1 >> fanin2;
         current = new CirGate(AIG_GATE, id/2, line);
         _totalList.push_back(current);
         _sortList.insert(pair<int, CirGate*>(id/2, current));
         fan[(line-_i-_o-2)*2 + _o] = fanin1;
         fan[(line-_i-_o-2)*2 + _o + 1] = fanin2;
      }

      //comment
      while (f >> s && s != "c") {
         id = 0;
         for (size_t i = 1; i < s.size(); ++i) {
            id *= 10;
            id += int(s[i] - '0');
         }
         if (s[0] == 'i') {
            f >> s;
            _totalList[id]->setName(s);
         }
         else if (s[0] == 'o') {
            f >> s;
            _totalList[id + _i]->setName(s);
         }
      }

      //CONST0
      current = new CirGate(CONST_GATE, 0, 0);
      _totalList.push_back(current);
      _sortList.insert(pair<int, CirGate*>(0, current));

      //linkPO
      for (int i = _i; i < (_i + _o); ++i) {
         linkPO(_totalList[i], fan[i - _i]);
      }

      //linkAIG
      for (int i = (_i + _o); i < (_i + _o + _a); ++i) {
         linkAIG(_totalList[i], fan[(i-_i-_o)*2 + _o], fan[(i-_i-_o)*2 + _o + 1]);
      }

      f.close();

      /*
      for (vector<CirGate*>::iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
         //(*it)->reportGate();
         (*it)->printGate();
      }

      for (int i = 0; i < (_o + _a * 2); ++i) {
         cout << fan[i] << endl;
      }
      */

      return true;
   }
}

CirGate*
CirMgr::getGate(int gid) const
{
   /*
   for (vector<CirGate*>::const_iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
      if ((*it)->getId() == gid) return (*it);
   }
   */
   map<int, CirGate*>::const_iterator it = _sortList.find(gid);
   if (it == _sortList.end()) return 0;
   return (it)->second;
}

void
CirMgr::linkPO(CirGate* po, int fanin)
{
   CirGate* add = getGate(fanin/2);
   int undef = 0;
   if (add == 0) {
      add = new CirGate(UNDEF_GATE, fanin/2, 0);
      _totalList.push_back(add);
      _sortList.insert(pair<int, CirGate*>(fanin/2, add));
      undef = 1;
   }
   if (fanin%2 == 0) po->setIn(add, 0, (2 + undef) + '0');
   else po->setIn(add, 0, (0 + undef) + '0');
   add->setOut(po);
}

void
CirMgr::linkAIG(CirGate* aig, int fanin1, int fanin2)
{
   CirGate* add = getGate(fanin1/2);
   int undef = 0;
   if (add == 0) {
      add = new CirGate(UNDEF_GATE, fanin1/2, 0);
      _totalList.push_back(add);
      _sortList.insert(pair<int, CirGate*>(fanin1/2, add));
      undef = 1;
   }
   if (fanin1%2 == 0) aig->setIn(add, 0, (2 + undef) + '0');
   else aig->setIn(add, 0, (0 + undef) + '0');
   add->setOut(aig);

   undef = 0;
   add = getGate(fanin2/2);
   if (add == 0) {
      add = new CirGate(UNDEF_GATE, fanin2/2, 0);
      _totalList.push_back(add);
      _sortList.insert(pair<int, CirGate*>(fanin2/2, add));
      undef = 1;
   }
   if (fanin2%2 == 0) aig->setIn(add, 1, (2 + undef) + '0');
   else aig->setIn(add, 1, (0 + undef) + '0');
   add->setOut(aig);
}

bool
CirMgr::readError(const string& fileName) const
{
   /*
   ifstream f(fileName);
   string s, sub;
   while (getline(f, s)) {
      if (s == "c") break;
      ++lineNo;
      colNo = 0;
   }
   f.close();
   */
   return false;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << "\nCircuit Statistics\n==================" << endl;
   cout << "  PI" << right << setw(12) << _i << endl;
   cout << "  PO" << right << setw(12) << _o << endl;
   cout << "  AIG" << right << setw(11) << _a << endl;
   cout << "------------------" << endl;
   cout << "  Total" << right << setw(9) << (_i + _o + _a) << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   ++CirGate::GlobalRef;

   dfs();

   for (size_t i = 0; i < _dfsList.size(); ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
}

void
CirMgr::dfs() const
{
   if (_dfsList.empty()) {
      for (int i = _i; i < (_i + _o); ++i) { _totalList[i]->dfsNet(_dfsList, _AIGdfsList); }
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (int i = 0; i < _i; ++i) { cout << " " << (_totalList[i])->getId(); }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (int i = _i; i < (_i + _o); ++i) { cout << " " << (_totalList[i])->getId(); }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   //find floating fanins
   if (_totalList.back()->unDef()) {
      vector<int> floatingIn;
      for (int i = _i; i < (_i+_o+_a); ++i) {
         if (_totalList[i]->floating()) floatingIn.push_back(_totalList[i]->getId());
      }
      cout << "Gates with floating fanin(s):";
      sort(floatingIn.begin(), floatingIn.end());
      for (vector<int>::iterator it = floatingIn.begin(); it != floatingIn.end(); ++it) {
         cout << " " << (*it);
      }
      cout << endl;
   }

   //find unused gate
   vector<int> unusedGate;
   for (int i = 0; i < _i; ++i) {
      if (_totalList[i]->unused()) unusedGate.push_back(_totalList[i]->getId());
   }
   for (int i = (_i + _o); i < (_i + _o + _a); ++i) {
      if (_totalList[i]->unused()) unusedGate.push_back(_totalList[i]->getId());
   }
   if (!unusedGate.empty()) {
      cout << "Gates defined but not used  :";
      sort(unusedGate.begin(), unusedGate.end());
      for (vector<int>::iterator it = unusedGate.begin(); it != unusedGate.end(); ++it) {
         cout << " " << (*it);
      }
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   if (_AIGdfsList.empty()) dfs();
   outfile << "aag " << _m << " " << _i << " " << _l << " " << _o << " " << _AIGdfsList.size() << endl;

   //PI
   for (int i = 0; i < _i; ++i) {
      outfile << (_totalList[i]->getId()) * 2 << endl;
   }

   //PO
   for (int i = _i;i < (_i + _o); ++i) {
      if (_totalList[i]->inv(0)) outfile << ((_totalList[i]->getFanIn(0))->getId()) * 2 + 1 << endl;
      else outfile << ((_totalList[i]->getFanIn(0))->getId() * 2) << endl;
   }

   //AIG
   for (GateList::const_iterator it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
      outfile << (*it)->getId() * 2;
      for (int i = 0; i < 2; ++i) {
	 if ((*it)->inv(i)) outfile << " " << (((*it)->getFanIn(i))->getId()) * 2 + 1;
	 else outfile << " " << (((*it)->getFanIn(i))->getId()) * 2;
      }
      outfile << endl;
   }

   //_name
   for (int i = 0; i < _i; ++i) {
      if (_totalList[i]->getName() != "") {
	 outfile << "i" << i << " " << _totalList[i]->getName() << endl;
      }
   }

   for (int i = _i; i < (_i + _o); ++i) {
      if (_totalList[i]->getName() != "") {
	 outfile << "o" << (i - _i) << " " << _totalList[i]->getName() << endl;
      }
   }
   outfile << 'c' << endl;
   outfile << "DONE..." << endl;
}
