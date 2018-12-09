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
//#include <fstream>

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
   //handle parsing error message
   if (false) return false;
   else {
      ifstream f(fileName);
      if (!f.is_open()) {
	 cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
	 return false;
      }
      string s;
      vector<string> content{""};
      while (getline(f, s)) {
	  //if (s == "c") break;
	  if (s[0] == 'c') break;
	  content.push_back(s);
      }
      f.close();

      string agg, M, I, L, O, A, token;
      int pos = myStrGetTok(content[1], agg), line = 2, end = line, token_i;
      vector<int> idlist;
      pos = myStrGetTok(content[1], M, pos);
      pos = myStrGetTok(content[1], I, pos);
      pos = myStrGetTok(content[1], L, pos);
      pos = myStrGetTok(content[1], O, pos);
      _m = int(M[0] - '0');
      _i = int(I[0] - '0');
      _l = int(L[0] - '0');
      _o = int(O[0] - '0');

      //PI
      end += _i;
      CirGate* current = new CirGate(CONST_GATE, 0, 0, "");
      _totalList.push_back(current);
      for (; line < end; ++line) {
	  myStrGetTok(content[line], token);
	  token_i = s2i(token)/2;
	  idlist.push_back(token_i);
	  current = new CirGate(PI_GATE, token_i, line, content[line]);
	  _piList.push_back(current);
	  _totalList.push_back(current);
      }
      //PO
      /*
      end += _o;
      //line = _i + 1
      for (; line < end; ++line) {
	  myStrGetTok(content[line], token);
	  token_i = s2i(token_i);
	  if (token_i%2) { po.push_back((-1) * token_i/2); }
	  else { po.push_back(token_i/2); }
      }
      */
      line += _o;
      //AIG
      vector<CirGate*> aigList;
      for (; line < content.size(); ++line) {
	  myStrGetTok(content[line], token);
	  token_i = s2i(token)/2;
	  idlist.push_back(token_i);
	  current = new CirGate(AIG_GATE, token_i, line, content[line]);
	  _totalList.push_back(current);
	  aigList.push_back(current);
      }

      int isMax = 0;
      for (vector<int>::iterator it = idlist.begin(); it != idlist.end(); ++it) {
	  if (*it >= isMax) isMax = (*it) + 1;
      }
      line = _i + 2;
      end = line + _o;
      for (; line < end; ++line) {
	  current = new CirGate(PO_GATE, isMax, line, content[line]);
	  ++isMax;
	  _totalList.push_back(current);
	  _poList.push_back(current);
      }

      //for (vector<CirGate*>::iterator it = aigList.begin(); it != aigList.end(); ++it) { linkAIG(*it); }
      //for (vector<CirGate*>::iterator it = _poList.begin(); it != _poList.end(); ++it) { linkPO(*it); }

      vector<string>().swap(content);
      vector<int>().swap(idlist);
      vector<CirGate*>().swap(aigList);
      delete current;

      //checking
      for (vector<CirGate*>::iterator it = _totalList.begin(); it != _totalList.end(); ++it) { (*it)->reportGate(); }

      return true;
   }
}
/*
void
CirMgr::linkAIG(const CirGate* aig)
{
    string gate, fanin1, fanin2;
    int pos = myStrGetTok(aig->getLine(), gate), tmp;
    const CirGate* add;
    pos = myStrGetTok(aig->getLine(), fanin1, pos);
    pos = myStrGetTok(aig->getLine(), fanin2, pos);
    tmp = s2i(fanin1);
    add = getGate(tmp/2);
    (tmp%2)? (aig->setFANIn(add, false)):(aig->setFANIn(add, true));
    add->setFANOut(aig);
    tmp = s2i(fanin2);
    add = getGate(tmp/2);
    (tmp%2)? (aig->setFANIn(add, false)):(aig->setFANIn(add, true));
    add->setFANOut(aig);
}*/

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
}

void
CirMgr::printNetlist() const
{
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
}
