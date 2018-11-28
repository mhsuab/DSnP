/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   int numObjects, arraySize, iora = 0;
   //iora == 1 -> -Index, iora == 2 -> -Array
   if (!CmdExec::lexOptions(option,options)) return CMD_EXEC_ERROR;
   if (options.empty()) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   bool second_num = myStr2Int(options[0], numObjects) && (numObjects>0), second_a = myStrNCmp("-Array", options[0], 2);
   if ((!second_num) && (second_a)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   //the second one is numObjects
   else if (second_num)
   {
	  if (options.size() == 1) iora = 1;
	  //if (options.size() == 1) mtest.newObjs((size_t)numObjects);
	  //if (options.size() == 1) return CMD_EXEC_DONE;
	  else if (myStrNCmp("-Array", options[1], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[1]);
	  else if (options.size() == 2) return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
	  else if (!myStr2Int(options[2], arraySize) || (arraySize <= 0)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
	  else if (options.size() >= 4) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
	  else iora = 2;
	  //else mtest.newObjs((size_t)numObjects);
   }
   //the second one is -Array
   else if (!second_a)
   {
	  if (options.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
	  else if (!myStr2Int(options[1], arraySize) || (arraySize<=0)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
	  else if (options.size() == 2) return CmdExec::errorOption(CMD_OPT_MISSING, "");
	  else if (!myStr2Int(options[2], numObjects) || (numObjects<=0)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
	  else if (options.size() >= 4) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
	  else iora = 2;
	  //else mtest.newArrs((size_t)numObjects, (size_t)arraySize);
   }

   //TODO NOT DONE YET
   // Use try-catch to catch the bad_alloc exception
   try
   {
	  if (iora == 1) mtest.newObjs((size_t) numObjects);
	  else if (iora == 2) mtest.newArrs((size_t) numObjects, (size_t)arraySize);
   }
   catch (bad_alloc& e) {return CMD_EXEC_ERROR;}
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   int dele_size, irao = 0;
   //irao = 1 -> -Index, Obj
   //irao = 2 -> -Index, Arr
   //irao = 3 -> -Random, Obj
   //irao = 4 -> -Random, Arr
   size_t arr_size = mtest.getArrListSize(), obj_size = mtest.getObjListSize();
   if (!CmdExec::lexOptions(option,options)) return CMD_EXEC_ERROR;
   if (options.empty()) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   bool i_b = myStrNCmp("-Index", options[0], 2), r_b = myStrNCmp("-Random", options[0], 2), arr = myStrNCmp("-Array", options[0], 2);
   if (i_b && r_b && arr) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   //-Index
   else if (!i_b)
   {
	  if (options.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
	  else if (!myStr2Int(options[1], dele_size) || dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
	  //deleteArr
	  if (options.size() >= 3)
	  {
		 //-a
		 if (!myStrNCmp("-Array", options[2], 2))
		 {
			//size_t arr_size = mtest.getArrListSize();
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			if (options.size() >= 4)
			{
			   if (!myStrNCmp("-Random", options[3], 2) || !myStrNCmp("-Index", options[3], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[3]);
			}
			else if (dele_size >= (int)arr_size)
			{
			   cerr << "Size of array list (" << arr_size << ") is <= " << options[1] << "!!" << endl;
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			}
			else irao = 2;
		 }
		 else if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2))
		 {
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
		 }
		 else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
	  }
	  //deleteObj
	  else
	  {
		 //size_t obj_size = mtest.getObjListSize();
		 //if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
		 if (dele_size >= (int)obj_size)
		 {
			cerr << "Size of object list (" << obj_size << ") is <= " << options[1] << "!!" << endl;
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
		 }
		 else irao = 1;
	  }
   }
   //-Random
   else if(!r_b)
   {
	  if (options.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
	  else if (!myStr2Int(options[1], dele_size) || dele_size <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
	  //deleteArr
	  else if (options.size() >= 3)
	  {
		 //-a
		 if (!myStrNCmp("-Array", options[2], 2))
		 {
			//size_t arr_size = mtest.getArrListSize();
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			if (options.size() >= 4)
			{
			   if (!myStrNCmp("-Random", options[3], 2) || !myStrNCmp("-Index", options[3], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[3]);
			}
			else if (arr_size == 0)
			{
			   cerr << "Size of array list is 0!!" << endl;
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
			}
			else irao = 4;
		 }
		 else if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2))
		 {
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
		 }
		 else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
	  }
	  //deleteObj
	  else
	  {
		 //size_t obj_size = mtest.getObjListSize();
		 //if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
		 if (obj_size == 0)
		 {
			cerr << "Size of object list is 0!!" << endl;
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
		 }
		 else irao = 3;
	  }
   }
   //undetermined
   else if(!arr)
   {
	  if (options.size() == 1) return CmdExec::errorOption(CMD_OPT_MISSING, "");
	  //-Index
	  else if (!myStrNCmp("-Index", options[1], 2))
	  {
		 if (options.size() == 2) return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
		 else if (!myStr2Int(options[2], dele_size) || dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
		 else
		 {
			//size_t arr_size = mtest.getArrListSize();
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
			if (options.size() >= 4)
			{
			   if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[3]);
			}
			else if (dele_size >= (int)arr_size)
			{
			   cerr << "Size of array list (" << arr_size << ") is <= " << options[2] << "!!" << endl;
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
			}
			else irao = 2;
		 }
	  }
	  else if (!myStrNCmp("-Random", options[1], 2))
	  {
		 if (options.size() == 2) return CmdExec::errorOption(CMD_OPT_MISSING, options[1]);
		 else if (!myStr2Int(options[2], dele_size) || dele_size <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
		 else
		 {
			//size_t arr_size = mtest.getArrListSize();
			//if (dele_size < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
			if (options.size() >= 4)
			{
			   if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[3]);
			}
			else if (arr_size == 0)
			{
			   cerr << "Size of array list is 0!!" << endl;
			   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
			}
			else irao = 4;
		 }
	  }
	  else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
   }
   if (irao == 1) mtest.deleteObj((size_t) dele_size);
   else if (irao == 2) mtest.deleteArr((size_t) dele_size);
   else if (irao == 3) for (int i = 0; i < dele_size; i++) mtest.deleteObj((size_t) rnGen(obj_size));
   else if (irao == 4) for (int i = 0; i < dele_size; i++) mtest.deleteArr((size_t) rnGen(arr_size));
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


