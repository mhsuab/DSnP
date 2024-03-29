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
   ILLEGAL_NEWLINE,
   ILLEGAL_LATCHES,
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
//static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static int lineNo = 0;
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

size_t CirGate::GlobalRef = 1;

static bool
parseError(CirParseError err = DUMMY_END)
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
      case ILLEGAL_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Exist Unexpexted newline!!" << endl;
         break;
      case ILLEGAL_LATCHES:
         cerr << "[ERROR] Line " << lineNo+1
              << ": Illegal latches!!" << endl;
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
      default:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1 << endl;
         break;
   }
   lineNo = 0;
   colNo = 0;
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   //for error parsing
   char c, prev;
   int seq_pos, count_space = 0;

   ifstream f(fileName);
   if (!f.is_open()) {
      cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
      return false;
   }
   int end, id, fanin1, fanin2;
   string s;

   seq_pos = f.tellg();
   while (true) {
      f.get(c);
      switch (colNo) {
         case 0:
            if (c == ' ') {
               parseError(EXTRA_SPACE);
               return false;
            }
            else if (c == '\n') {
               errMsg = "aag";
               parseError(MISSING_IDENTIFIER);
               return false;
            }
            else if (iscntrl(c)) {
               errInt = (int)c;
               parseError(ILLEGAL_WSPACE);
               return false;
            }
            else if (c != 'a') {
               f.seekg(seq_pos);
               f >> errMsg;
               parseError(ILLEGAL_IDENTIFIER);
               return false;
            }
            break;
         case 1:
            if (c != 'a') {
               f.seekg(seq_pos);
               f >> errMsg;
               parseError(ILLEGAL_IDENTIFIER);
               return false;
            }
            break;
         case 2:
            if (c != 'g') {
               f.seekg(seq_pos);
               f >> errMsg;
               parseError(ILLEGAL_IDENTIFIER);
               return false;
            }
            break;
         case 3:
            if (c != ' ') {
               f.seekg(seq_pos);
               f >> errMsg;
               parseError(ILLEGAL_IDENTIFIER);
               return false;
            }
            else if (isdigit(c)) {
               f.seekg(seq_pos);
               f >> errMsg;
               parseError(ILLEGAL_IDENTIFIER);
               return false;
            }
            ++count_space;
            break;
         default:
            //numbers
            if (isdigit(c)) {}
            else if (c == ' ') {
               if (prev == ' ') {
                  parseError(EXTRA_SPACE);
                  return false;
               }
               else if (isdigit(prev) && count_space == 5) {
                  parseError(MISSING_NEWLINE);
                  return false;
               }
               ++count_space;
            }
            else if (c == '\n') {
               if (isdigit(prev) && count_space == 5) break;
               else {
                  parseError(ILLEGAL_NEWLINE);
                  return false;
               }
            }
            else if (iscntrl(c)) {
               errInt = (int) c;
               parseError(ILLEGAL_WSPACE);
               return false;
            }
            else {
               //usual character
               errMsg = c;
               parseError(ILLEGAL_NUM);
               return false;
            }
            break;
      }
      if (c == '\n') break;
      ++colNo;
      prev = c;
   }
   f.seekg(seq_pos);
   colNo = 0;

   f >> s >> _m >> _i >> _l >> _o >> _a;
   if (_m < (_i + _l + _a)) {
      errMsg = "Number of variables";
      errInt = _m;
      parseError(NUM_TOO_SMALL);
      return false;
   }
   else if (_l != 0) {
      parseError(ILLEGAL_LATCHES);
      return false;
   }
   ++lineNo;

   CirGate* current;
   int fan[_o + _a * 2];


   //PI
   end = lineNo + _i;
   for (; lineNo < end; ++lineNo) {
      if (f.peek() == '\n') f.get(c);
      seq_pos = f.tellg();
      while (true) {
         f.get(c);
         if (colNo == 0 && !isdigit(c)) {
            if (c == '\n') {
               parseError(ILLEGAL_NUM);
               return false;
            }
            else if (c == ' ') {
               parseError(EXTRA_SPACE);
               return false;
            }
            else if (!iscntrl(c)) {
               errMsg = c;
               parseError(ILLEGAL_NUM);
               return false;
            }
            errInt = (int) c;
            parseError(ILLEGAL_WSPACE);
            return false;
         }
         else if (c == '\n') break;
         else if (!isdigit(c)) {
            errInt = (int) c;
            parseError(ILLEGAL_WSPACE);
            return false;
         }
         ++colNo;
      }
      f.seekg(seq_pos);
      colNo = 0;
      f >> id;
      if (id % 2 == 1) {
         errMsg = "PI";
         errInt = id;
         parseError(CANNOT_INVERTED);
         return false;
      }
      else if (id/2 > _m) {
         errInt = id/2;
         parseError(MAX_LIT_ID);
         return false;
      }
      else if (id/2 == 0) {
         errInt = id/2;
         parseError(REDEF_CONST);
         return false;
      }
      else if (_sortList.find(id/2) != _sortList.end()) {
         errInt = id;
         errGate = _sortList.find(id/2)->second;
         parseError(REDEF_GATE);
         return false;
      }
      current = new CirGate(PI_GATE, id/2, lineNo + 1);
      _totalList.push_back(current);
      _sortList.insert(pair<int, CirGate*>(id/2, current));
   }


   //PO
   end += _o;
   id = _m + 1;
   for (; lineNo < end; ++lineNo) {
      if (f.peek() == '\n') f.get(c);
      seq_pos = f.tellg();
      while (true) {
         f.get(c);
         if (colNo == 0 && !isdigit(c)) {
            if (c == '\n') {
               parseError(ILLEGAL_NEWLINE);
               return false;
            }
            else if (c == ' ') {
               parseError(EXTRA_SPACE);
               return false;
            }
            else if (!iscntrl(c)) {
               errMsg = c;
               parseError(ILLEGAL_NUM);
               return false;
            }
            errInt = (int) c;
            parseError(ILLEGAL_WSPACE);
            return false;
         }
         else if (c == '\n') break;
         else if (!isdigit(c)) {
            errMsg = c;
            parseError(ILLEGAL_NUM);
            return false;
         }
         ++colNo;
      }
      f.seekg(seq_pos);
      colNo = 0;
      f >> fanin1;
      current = new CirGate(PO_GATE, id, lineNo + 1);
      _totalList.push_back(current);
      _sortList.insert(pair<int, CirGate*>(id, current));
      ++id;
      fan[lineNo - _i - 1] = fanin1;
   }


   //AIG
   end += _a;
   for (; lineNo < end; ++ lineNo) {
      if (f.peek() == '\n') f.get(c);
      seq_pos = f.tellg();
      count_space = 0;
      while (true) {
         f.get(c);
         if (colNo == 0 && !isdigit(c)) {
            if (c == '\n') {
               parseError(ILLEGAL_NEWLINE);
               return false;
            }
            else if (c == ' ') {
               parseError(EXTRA_SPACE);
               return false;
            }
            else if (!iscntrl(c)) {
               errMsg = c;
               parseError(ILLEGAL_NUM);
               return false;
            }
            errInt = (int) c;
            parseError(ILLEGAL_WSPACE);
            return false;
         }
         else if (c == ' ') {
            if (prev == ' ') {
               parseError(EXTRA_SPACE);
               return false;
            }
            else if (count_space == 2) {
               parseError(EXTRA_SPACE);
               return false;
            }
            ++count_space;
         }
         else if (c == '\n') {
            if (count_space == 2) break;
            else {
               parseError(ILLEGAL_NEWLINE);
               return false;
            }
         }
         else if (!isdigit(c)) {
            errMsg = c;
            parseError(ILLEGAL_NUM);
            return false;
         }
         prev = c;
         ++colNo;
      }
      f.seekg(seq_pos);
      colNo = 0;
      f >> id >> fanin1 >> fanin2;
      if (id % 2 == 1) {
         errMsg = "AIG";
         errInt = id;
         parseError(CANNOT_INVERTED);
         return false;
      }
      else if (id/2 > _m) {
         errInt = id/2;
         parseError(MAX_LIT_ID);
         return false;
      }
      else if (id/2 == 0) {
         errInt = id/2;
         parseError(REDEF_CONST);
         return false;
      }
      else if (_sortList.find(id/2) != _sortList.end()) {
         errInt = id;
         errGate = _sortList.find(id/2)->second;
         parseError(REDEF_GATE);
         return false;
      }
      current = new CirGate(AIG_GATE, id/2, lineNo + 1);
      _totalList.push_back(current);
      _sortList.insert(pair<int, CirGate*>(id/2, current));
      fan[(lineNo-_i-_o-1)*2 + _o] = fanin1;
      fan[(lineNo-_i-_o-1)*2 + _o + 1] = fanin2;
   }


   //comment
   seq_pos = f.tellg();
   count_space = 0;
   int line = lineNo;
   f.ignore();
   while (f.peek() != EOF) {
      f.get(c);
      if (colNo == 0 && c == 'c') {
         if (f.peek() == '\n' || f.peek() == EOF) break;
         else {
            parseError(MISSING_NEWLINE);
            return false;
         }
      }
      else if (colNo == 0 && c != 'i' && c != 'o') {
         if (c == ' ') {
            parseError(EXTRA_SPACE);
            return false;
         }
         else if (c == '\n') {
            parseError(ILLEGAL_NEWLINE);
            return false;
         }
         else if (iscntrl(c)) {
            errInt = (int) c;
            parseError(ILLEGAL_WSPACE);
            return false;
         }
         errMsg = c;
         parseError(ILLEGAL_SYMBOL_TYPE);
         return false;
      }
      else if (colNo == 1 && !isdigit(c)) {
         if (c == '\n') {
            parseError(ILLEGAL_NEWLINE);
            return false;
         }
         else if (c == ' ') {
            parseError(EXTRA_SPACE);
            return false;
         }
         else if (!iscntrl(c)) {
            errMsg = c;
            parseError(ILLEGAL_NUM);
            return false;
         }
         errInt = (int) c;
         parseError(ILLEGAL_WSPACE);
         return false;
      }
      else if (c == ' ') {
         ++count_space;
      }
      /*
      else if (c == ' ') {
         if (prev == ' ') {
            parseError(EXTRA_SPACE);
            return false;
         }
         else if (count_space == 1) {
            parseError(EXTRA_SPACE);
            return false;
         }
         ++count_space;
      }
      */
      else if (c == '\n') {
         if ((count_space >= 2) || (count_space == 1 && prev != ' ')) {
            colNo = -1;
            count_space = 0;
            ++lineNo;
         }
         else if (count_space == 0) {
            parseError(ILLEGAL_NEWLINE);
            return false;
         }
         else {
            parseError(ILLEGAL_NEWLINE);
            return false;
         }
      }
      else if (iscntrl(c)) {
         errInt = (int) c;
         parseError(ILLEGAL_WSPACE);
         return false;
      }
      prev = c;
      ++colNo;
   }
   colNo = 0;
   lineNo = line;
   f.seekg(seq_pos);

   while (f >> s && s != "c") {
      id = 0;
      for (size_t i = 1; i < s.size(); ++i) {
         id *= 10;
         id += int(s[i] - '0');
      }
      if (s[0] == 'i') {
         if (id > (_i - 1)) {
            errMsg = "PI";
            errInt = id;
            parseError(NUM_TOO_BIG);
            return false;
         }
         f >> s;
         if (_totalList[id]->getName() != "") {
            errMsg = "i";
            errInt = id;
            parseError(REDEF_SYMBOLIC_NAME);
            return false;
         }
         _totalList[id]->setName(s);
      }
      else if (s[0] == 'o') {
         if (id > (_o - 1)) {
            errMsg = "PO";
            errInt = id;
            parseError(NUM_TOO_BIG);
            return false;
         }
         f >> s;
         if (_totalList[id + _i]->getName() != "") {
            errMsg = "o";
            errInt = id;
            parseError(REDEF_SYMBOLIC_NAME);
            return false;
         }
         _totalList[id + _i]->setName(s);
      }
      ++lineNo;
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
      (*it)->reportGate();
      //(*it)->printGate();
   }
   for (int i = 0; i < (_o + _a * 2); ++i) {
      cout << fan[i] << endl;
   }
   */

   lineNo = 0;
   colNo  = 0;

   return true;
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
   if (add->unDef()) undef = 1;
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
   if (add->unDef()) undef = 1;
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
   if (add->unDef()) undef = 1;
   if (fanin2%2 == 0) aig->setIn(add, 1, (2 + undef) + '0');
   else aig->setIn(add, 1, (0 + undef) + '0');
   add->setOut(aig);
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
