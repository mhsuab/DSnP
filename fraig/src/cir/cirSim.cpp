/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <unordered_map>
#include <bitset>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   ++CirGate::GlobalRef;
   dfs();

   size_t threshold = 10, fail = 0,  pre = 0, n_cycle = 0, fsize = 0, rn;
   while (fail < threshold && n_cycle < 10000) {
      for (int i = 0; i < _i; ++i) {
         //srand(time(NULL));
         #if RAND_MAX == 2147483647
         rn = rand()*2147483648 + rand();
         #elif RAND_MAX == 32767
         rn = rand()*32768*32768 + rand()*32768 + rand();
         #elif RAND_MAX == 92233720336854775807
         rn = rand();
         #endif
         _totalList[i]->setPIrandsim(rn);
      }
      /*
      for(int i = 0; i < _i; ++i)
      {
         #if RAND_MAX == 2147483647
         rn = rand()*2147483648 + rand();
         #else
         rn = rand()*32768*32768+rand()*32768+rand();
         #endif
         _totalList[i]->setPIrandsim(rn);
      }
      */
      sim();
      fsize = _fec.size();
      //if (fsize >= pre) ++fail;
      if (fsize == pre) ++fail;
      else {
         fail = 0;
      }
      ++n_cycle;
      //if (fail >= threshold || fsize == 1) break;
      //if (fsize < pre || pre == 0) pre = fsize;
      pre = fsize;
   }
   cout << char(13) << setw(30) << char(13);
   setFEC();
   cout << "Total #FEC Group = " << _fec.size() << endl;
   cout << n_cycle*64 << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   string s;
   size_t n = 0;
   vector<string> patterns;
   while (patternFile >> s) {
      ++n;
      if ((int) s.size() != _i) {
         cerr << "Error: Pattern(" << s <<") length(" << s.size() << ") does not match the number of inputs(" << _i << ") in a circuit!!" << endl;
         return;
      }
      for (size_t	i = 0; i < s.size(); ++i) {
         if (s[i] != '0' && s[i] != '1') {
            cerr << "Error: Pattern(" << s << ") contains a non-0/1 character(‘" << s[i] << "’)." << endl;
            return;
         }
         //_totalList[i]->setPIsim(s[i]);
      }
      patterns.push_back(s);
      /*
      if (n%64 == 0) {
         sim();
         for (auto it = _totalList.begin(); it != _totalList.end(); ++it) {
            cerr << (*it)->getTypeStr() << (*it)->getId() << " " << bitset<64>((*it)->getSim()) << endl;
         }
      }
      */
   }
   resetSim();
   for (size_t j = 1; j <= n; ++j) {
   //for (auto it = patterns.begin(); it != patterns.end(); ++it) {
      for (size_t i = 0; i < patterns[j - 1].size(); ++i) {
         _totalList[i]->setPIsim(patterns[j - 1][i]);
      }
      if (j%64 == 0 && j != 0) {
         sim();
         /*
         for (auto it = _totalList.begin(); it != _totalList.end(); ++it) {
            string s = (bitset<64>((*it)->getSim())).to_string();
            reverse(s.begin(), s.end());
            cerr << s << endl;
         }
         */
         if (_simLog != 0) writeSim(64);
         //cerr << "w" << j << endl;
      }
   }
   if (n%64 != 0) {
         for (int i = 0; i < (_i + _o + _a); ++i) {
            _totalList[i]->moveto64(64 - n%64);
         }
         sim();
         //debug
         /*
         for (auto it = _totalList.begin(); it != _totalList.end(); ++it) {
            string s = (bitset<64>((*it)->getSim())).to_string();
            reverse(s.begin(), s.end());
            cerr << s << endl;
         }
         */
         if (_simLog != 0) writeSim(n%64);
         //cerr << "q" << endl;
   }
   cout << char(13) << setw(30) << ' ' << char(13);
   cout << n << " patterns simulated." << endl;
   setFEC();
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::simAllGate()
{
   dfs();
   for (auto it = (_totalList.begin() + _i); (*it)->getType() != CONST_GATE; ++it) {
      (*it)->resetSimVal();
   }
   for (GateList::iterator it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
      (*it)->simAIG();
      if (_fec.empty()) (*it)->resetFECGate();
   }
   for (int i = _i; i < (_i + _o); ++i) {
      _totalList[i]->simPO();
   }
   if (_fec.empty()) _totalList[_i + _o + _a]->resetFECGate();
}

void
CirMgr::sim()
{
   SimHash hash(getHashSize(_AIGdfsList.size()));
   PointerInvList* u = new PointerInvList();
   simAllGate();
   if (_fec.empty()) {
      u->push_back((size_t) _totalList[_i + _o + _a]);
      hash.insert({0, u});      
      for (GateList::iterator it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
         SimHash::iterator r = hash.find((*it)->getSim()), b = hash.find(~((*it)->getSim()));
         if (r != hash.end()) {
            //cerr << "a" << endl;
            //if (r->first == 0) cerr << 'a' << (*it)->getId();
            (r->second)->push_back((size_t)(*it));
         }
         else if (b != hash.end()) {
            //cerr << "b" << endl;
            //if (b->first == 0) cerr << 'b' << (*it)->getId();
            (b->second)->push_back((size_t)(*it) + 1);
         }
         else {
            //cerr << "c" << endl;
            u = new PointerInvList();
            u->push_back((size_t)(*it));
            hash.insert({(*it)->getSim(), u});
         }
      }
      if ((hash.find(0)->second)->size() == 1) { hash.erase(0); }
      for (SimHash::iterator it = hash.begin(); it != hash.end(); ++it) {
         if (((*it).second)->size() != 1) _fec.push_back((*it).second);
      }
      hash.clear();
   }
   else {
      vector<PointerInvList*> fec;
      for (auto it = _fec.begin(); it != _fec.end(); ++it) {
         if ((*it)->size() != 1) {
            for (auto j = (*it)->begin(); j != (*it)->end(); ++j) {
               CirGate* cur = (CirGate*) (((*j)/2)*2);
               auto r = hash.find(cur->getSim()), b = hash.find(~(cur->getSim()));
               if (r != hash.end()) {
                  //cerr << "A" << endl;
                  //if (r->first == 0) cerr << 'c' << cur->getId();
                  (r->second)->push_back((size_t)cur);
               }
               else if (b != hash.end()) {
                  //cerr << "B" << endl;
                  //if (b->first == 0) cerr << 'd' << cur->getId();
                  (b->second)->push_back((size_t)cur + 1);
               }
               else {
                  //cerr << "C" << endl;
                  u = new PointerInvList();
                  u->push_back((size_t)cur);
                  hash.insert({cur->getSim(), u});
               }
            }
            if (hash.find(0) != hash.end()) {
               if ((hash.find(0)->second)->size() == 1) { hash.erase(0); }
            }
            for (auto k = hash.begin(); k != hash.end(); ++k) {
               if (((*k).second)->size() != 1) fec.push_back((*k).second);
            }
            hash.clear();
            assert(hash.size() == 0);
         }
      }
      fec.swap(_fec);
      vector<PointerInvList*>().swap(fec);
      /*
      for (auto it = _fec.begin(); it != _fec.end(); ++it) {
         if ((*it)->size() == 1) {
            delete (*it);
            _fec.erase(it);
         }
      }
      */
   }

   /*
   vector<vector<int>> b = {{1,2,3},{2,3},{4},{5,6},{7}};
   cout << count_if(b.begin(), b.end(), [](vector<int> v){ return v.size() != 1; }) << endl;
   3
   */

   cout << "Total #FEC Group = " << _fec.size() << char(13) << flush;
   //cout << "Total #FEC Group = " << ((_fec.empty())? 0:count_if(_fec.begin(), _fec.end(), [](PointerInvList* v){ return v->size() != 1; })) << char(13) << flush;
   //setFEC();
}

void
CirMgr::resetSim()
{
   for (auto it = _totalList.begin(); (*it)->getType() != CONST_GATE; ++it) {
      (*it)->resetSimVal();
   }
}

void
CirMgr::sortFEC()
{
   for (size_t i = 0; i < _fec.size(); ++i) { sort(_fec[i]->begin(), _fec[i]->end(), compareSubFEC); }
   sort(_fec.begin(), _fec.end(), compareFEC);
}

void
CirMgr::setFEC()
{
   sortFEC();
   for (size_t j = 0; j < _fec.size(); ++j) {
      if (_fec[j]->size() > 1) {
         for (size_t k = 0; k < _fec[j]->size(); ++k) {
            ((CirGate*)((((*_fec[j])[k])/2)*2))->setFECGate(*(_fec[j]));
         }
      }
   }
}

void
CirMgr::writeSim(size_t n)
{
   vector<string> PISim, POSim;
   for (int i = 0; i < _i; ++i) {
      PISim.push_back(_totalList[i]->SimtoStr());
   }
   for (int i = _i; i < (_i + _o); ++i) {
      POSim.push_back(_totalList[i]->SimtoStr());
   }
   for (size_t i = 0; i < n; ++i) {
      for (int j = 0; j < _i; ++j) {
         *_simLog << PISim[j][i];
      }
      *_simLog << ' ';
      for (int k = 0; k < _o; ++k) {
         *_simLog << POSim[k][i];
      }
      *_simLog << endl;
   }
}



















