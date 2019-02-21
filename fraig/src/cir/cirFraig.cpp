/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "util.h"
#include <unordered_map>

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   ++CirGate::GlobalRef;
   dfs();
   StrashHash hash(getHashSize(_AIGdfsList.size()));

   for (GateList::iterator it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
      size_t in0 = 2 * (*it)->getFanIn(0)->getId() + (*it)->inv(0),
      in1 = 2 * (*it)->getFanIn(1)->getId() + (*it)->inv(1),
      key = (in0 < in1)? ((in0 << 32) + in1):((in1 << 32) + in0);
      pair<StrashHash::iterator, bool> r = hash.insert({key, (*it)});
      if (!r.second) {
         cout << "Strashing: " << r.first->second->getId() << " merging " << (*it)->getId() << "..." << endl;
         //cout << "Strashing: " << (*it)->getId() << " merging " << r.first->second->getId() << "..." << endl;
         //cerr << key << " " << (*it)->getId() << endl;
         //cerr << r.first->first << " " << r.first->second->getId() << endl;
         (*it)->getFanIn(0)->removeOut((*it));
         (*it)->getFanIn(1)->removeOut((*it));
         (*it)->changeINofOUT(r.first->second);
         (r.first->second)->addOutList((*it)->getFanOut());
         _totalList.erase(find((_totalList.begin() + _i + _o), _totalList.end(), (*it)));
         _sortList.erase((*it)->getId());
         delete (*it);
         (*it) = 0;
         --_a;
      }
   }
   ++CirGate::GlobalRef;
   GateList().swap(_dfsList);
   GateList().swap(_AIGdfsList);
   ++GlobalDfs;
}

void
CirMgr::fraig()
{
   SatSolver s;
   fraigMap fmap;

   ++CirGate::GlobalRef;
   dfs();

   s.initialize();

   //assign SAT varID
   //CONST0
   Var v = s.newVar();
   fmap.insert(pair<CirGate*, Var>(_totalList[_i + _o + _a], v));
   //PI
   for (int i = 0; i < _i; ++i) {
      v = s.newVar();
      fmap.insert(pair<CirGate*, Var>(_totalList[i], v));
   }
   //AIG
   Var zero = fmap.find(_totalList[_i + _o + _a])->second;
   s.addAigCNF(zero, v, true, v, false);
   for (auto it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
      v = s.newVar();
      fmap.insert(pair<CirGate*, Var>((*it), v));
      if (((*it)->getFanIn(0))->unDef() && ((*it)->getFanIn(1))->unDef()) {
         s.addAigCNF(v, zero, false, zero, false);
      }
      else if (((*it)->getFanIn(0))->unDef()) {
         s.addAigCNF(v, zero, false, fmap.find((*it)->getFanIn(1))->second, (*it)->inv(1));
      }
      else if (((*it)->getFanIn(1))->unDef()) {
         s.addAigCNF(v, fmap.find((*it)->getFanIn(0))->second, (*it)->inv(0), zero, false);
      }
      else {
         s.addAigCNF(v, fmap.find((*it)->getFanIn(0))->second, (*it)->inv(0), fmap.find((*it)->getFanIn(1))->second, (*it)->inv(1));
      }
   }

   size_t i = 0, j = 1, count = 0, q = 0, a, b;
   bool result;
   SimHash mergeHash(getHashSize(_AIGdfsList.size()));
   PointerInvList* u;
   SimHash::iterator r;
   int k;
   if (!_fec.empty()) a = (*_fec[0])[0];

   while (!_fec.empty()) {
      if (_fec.size() == 1 && _fec[0]->size() == 2) {
         break;
      }
      if (j >= _fec[i]->size()) {
         ++q;
         j = q + 1;
         if (j >= _fec[i]->size()) {
            ++i;
            q = 0;
            j = 1;
         }
         /*
         if (q == (j - 1)) {
            ++i;
            q = 0;
            j = 1;
         }
         else {
            ++q;
            j = q + 1;
         }
         */
         a = (*_fec[i])[q];
      }

      //SAT
      v = s.newVar();
      b = (*_fec[i])[j];
      if (((a%2 == 1) && (b%2 == 1)) || ((a%2 == 0) && (b%2 == 0))) {
         s.addXorCNF(v, fmap.find((CirGate*)(a/2*2))->second, false, fmap.find((CirGate*)(b/2*2))->second, false);
      }
      else {
         s.addXorCNF(v, fmap.find((CirGate*)(a/2*2))->second, false, fmap.find((CirGate*)(b/2*2))->second, true);
      }
      s.assumeRelease();
      s.assumeProperty(v, true);
      result = s.assumpSolve();
      if (result) {
         for (int p = 0; p < _i; ++p) {
            k = s.getValue(fmap.find(_totalList[p])->second);
            if (k < 0) _totalList[p]->setPIsim('0');
            else _totalList[p]->setPIsim(k + '0');
         }
         ++count;
      }
      else {
         r = mergeHash.find(a);
         if (r != mergeHash.end()) {
            (r->second)->push_back(b);
         }
         else {
            u = new PointerInvList();
            u->push_back(b);
            mergeHash.insert({a, u});
         }
         _fec[i]->erase((_fec[i]->begin()) + j);
         //fmap.erase((CirGate*)(((*_fec[i])[j])/2*2));
         //fmap.erase((CirGate*)(b/2*2));
         --j;
      }
      ++j;

      //to simulate?
      if (count == 64) {
         sim();
         sortFEC();
         cout << "Updating by SAT... Total #FEC Group = " << _fec.size() << endl;
         i = 0;
         q = 0;
         j = 1;
         count = 0;
      }
      else if ((i == (_fec.size() - 1)) && (j == (_fec[i])->size())) {
         sim();
         sortFEC();
         cout << "Updating by SAT... Total #FEC Group = " << _fec.size() << endl;
         i = 0;
         q = 0;
         j = 1;
         count = 0;
      }
   }

   merge(mergeHash);
   for (int i = (_i + _o); i < (_i + _o + _a + 1); ++i) {
      _totalList[i]->resetFECGate();
   }

   ++CirGate::GlobalRef;
   GateList().swap(_dfsList);
   GateList().swap(_AIGdfsList);
   ++GlobalDfs;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::merge(SimHash hash)
{
   CirGate *a, *b;
   for (auto it = hash.begin(); it != hash.end(); ++it) {
      a = (CirGate*) (((*it).first)/2*2);
      for (auto j = ((*it).second)->begin(); j != ((*it).second)->end(); ++j) {
         b = (CirGate*) ((*j)/2*2);
         b->getFanIn(0)->removeOut(b);
         b->getFanIn(1)->removeOut(b);
         a->addOutList(b->getFanOut());
         if (((((*it).first)%2 == 1) && ((*j)%2 == 1)) || ((((*it).first)%2 == 0) && ((*j)%2 == 0))) {
            b->changeINofOUT(a, 0);
            cout << "Fraig: " << a->getId() << " merging " << b->getId() << "..." << endl;
         }
         else {
            b->changeINofOUT(a, 1);
            cout << "Fraig: " << a->getId() << " merging !" << b->getId() << "..." << endl;
         }
         _totalList.erase(find((_totalList.begin() + _i + _o), _totalList.end(), b));
         _sortList.erase(b->getId());
         delete b;
         b = 0;
         --_a;
      }
   }
}













