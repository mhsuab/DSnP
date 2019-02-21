/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   //sweep out all gates that can't be reach by PO

   if (!isGlobalDfs()) {
      for (GateList::iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
         (*it)->resetDfs();
      }

      ++CirGate::GlobalRef;
      GateList().swap(_dfsList);
      GateList().swap(_AIGdfsList);
      dfs();

      settoGlobalDfs();
   }
   CirGate* tmp = 0;

   for (GateList::iterator it = _totalList.begin(); it != _totalList.end(); ++it) {
      if (!(*it)->inDfs()) {
         if ((*it)->isAig()||(*it)->unDef()) {
            cout << "Sweeping: " << (*it)->getTypeStr() << "(" << (*it)->getId() << ") removed..." << endl;
            if ((*it)->isAig()) {
               tmp = (*it)->getFanIn(0);
               if (tmp != 0) tmp->removeOut((*it));
               tmp = ((*it)->getFanIn(1));
               if (tmp != 0) tmp->removeOut((*it));
               //(*it)->removeInofOut(0, 0);
               --_a;
            }
            delete (*it);
            (*it) = 0;
            _totalList.erase(it);
            _sortList.erase((*it)->getId());
            --it;
         }
      }
   }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   CirGate* prev = 0;
   bool zero, inv = 0;

   ++CirGate::GlobalRef;
   GateList().swap(_dfsList);
   GateList().swap(_AIGdfsList);
   dfs();

   for (GateList::iterator it = _AIGdfsList.begin(); it != _AIGdfsList.end(); ++it) {
      if ((*it)->OptCase(prev, zero, inv)) {
         //Simplifying: 1 merging 2...
         //cerr << inv << endl;
         if (zero == 1) {
            prev = _totalList[_i + _o + _a];
            (*it)->removeInofOut(prev, inv);
         }
         cout << "Simplifying: " << prev->getId() << " merging " << (inv? "!":"") << (*it)->getId() << "..." << endl;
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

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/



