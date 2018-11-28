/****************************************************************************
  FileName      [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis      [ Define command parsing member functions for class CmdParser ]
  Author         [ Chung-Yang (Ric) Huang ]
  Copyright     [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"
#include <unistd.h>
#include <cstring>

using namespace std;

//----------------------------------------------------------------------
//     External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//     Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
    // TODO...
    _dofile = new ifstream(dof.c_str());
    if (_dofileStack.size() > 252)
    {
        delete _dofile;
        _dofile = _dofileStack.top();
        cerr << "Error: dofile stack overflow (252)" << endl;
        return false;
    }
    if (!(_dofile -> is_open()))
    {
        delete _dofile;
        if (!_dofileStack.empty()) _dofile = _dofileStack.top();
        else _dofile = 0;
        return false;
    }
    _dofileStack.push(_dofile);
    return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
    assert(_dofile != 0);
    // TODO...
    _dofile -> close();
    _dofileStack.pop();
    delete _dofile;
    if (!_dofileStack.empty()) _dofile = _dofileStack.top();
    else _dofile = 0;
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
    // Make sure cmd hasn't been registered and won't cause ambiguity
    string str = cmd;
    unsigned s = str.size();
    if (s < nCmp) return false;
    while (true) {
        if (getCmd(str)) return false;
        if (s == nCmp) break;
        str.resize(--s);
    }

    // Change the first nCmp characters to upper case to facilitate
    //     case-insensitive comparison later.
    // The strings stored in _cmdMap are all upper case
    //
    assert(str.size() == nCmp);  // str is now mandCmd
    string& mandCmd = str;
    for (unsigned i = 0; i < nCmp; ++i)
        mandCmd[i] = toupper(mandCmd[i]);
    string optCmd = cmd.substr(nCmp);
    assert(e != 0);
    e->setOptCmd(optCmd);

    // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
    return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
    bool newCmd = false;
    if (_dofile != 0)
    {
        while (_dofile->eof())
        {
            closeDofile();
        }
        if (_dofile != 0) newCmd = readCmd(*_dofile);
        else newCmd = readCmd(cin);
    }
    else
        newCmd = readCmd(cin);

    // execute the command
    if (newCmd) {
        string option;
        CmdExec* e = parseCmd(option);
        if (e != 0)
            return e->exec(option);
    }
    return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
    // TODO...
    CmdMap::const_iterator it;
    for (it = _cmdMap.begin(); it != _cmdMap.end(); it++)
    {
        (it->second)->help();
    }
    cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
    assert(_tempCmdStored == false);
    if (_history.empty()) {
        cout << "Empty command history!!" << endl;
        return;
    }
    int s = _history.size();
    if ((nPrint < 0) || (nPrint > s))
        nPrint = s;
    for (int i = s - nPrint; i < s; ++i)
        cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//     part of str (i.e. the first word) and retrive the corresponding
//     CmdExec* from _cmdMap
//     ==> If command not found, print to cerr the following message:
//          Illegal command!! "(string cmdName)"
//     ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//     "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//     words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
    assert(_tempCmdStored == false);
    assert(!_history.empty());
    string str = _history.back();
    // TODO...
    size_t end_of_cmd;
    string command = "";
    end_of_cmd = myStrGetTok(str, command);
    //cerr << "command:" << command << endl;
    CmdExec* e = getCmd(command);
    if (e == 0)
    //if (getCmd(command) == 0)
    {
        cerr << "Illegal command!! \"(" << command << ")\"" << endl;
    }
    if (end_of_cmd != string::npos) option = str.substr(end_of_cmd+1);
    //cerr << "option:" << option << endl;
    //
    assert(str[0] != 0 && str[0] != ' ');
    return e;
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//     --- 1.1 ---
//     [Before] Null cmd
//     cmd> $
//     --- 1.2 ---
//     [Before] Cmd with ' ' only
//     cmd>      $
//     [After Tab]
//     ==> List all the commands, each command is printed out by:
//              cout << setw(12) << left << cmd;
//     ==> Print a new line for every 5 commands
//     ==> After printing, re-print the prompt and place the cursor back to
//          original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//     --- 2.1 ---
//     [Before] partially matched (multiple matches)
//     cmd> h$                         // partially matched
//     [After Tab]
//     HELp          HIStory         // List all the parially matched commands
//     cmd> h$                         // and then re-print the partial command
//     --- 2.2 ---
//     [Before] partially matched (multiple matches)
//     cmd> h$llo                     // partially matched with trailing characters
//     [After Tab]
//     HELp          HIStory         // List all the parially matched commands
//     cmd> h$llo                     // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//     ==> In either of the following cases, print out cmd + ' '
//     ==> and reset _tabPressCount to 0
//     --- 3.1 ---
//     [Before] partially matched (single match)
//     cmd> he$
//     [After Tab]
//     cmd> heLp $                    // auto completed with a space inserted
//     --- 3.2 ---
//     [Before] partially matched with trailing characters (single match)
//     cmd> he$ahah
//     [After Tab]
//     cmd> heLp $ahaha
//     ==> Automatically complete on the same line
//     ==> The auto-expanded part follow the strings stored in cmd map and
//          cmd->_optCmd. Insert a space after "heLp"
//     --- 3.3 ---
//     [Before] fully matched (cursor right behind cmd)
//     cmd> hElP$sdf
//     [After Tab]
//     cmd> hElP $sdf                // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//     --- 4.1 ---
//     [Before] No match
//     cmd> hek$
//     [After Tab]
//     ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//     --- 5.1 ---
//     [Before] Already matched on first tab pressing
//     cmd> help asd$gh
//     [After] Print out the usage for the already matched command
//     Usage: HELp [(string cmd)]
//     cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//     ==> Note: command usage has been printed under first tab press
//     ==> Check the word the cursor is at; get the prefix before the cursor
//     ==> So, this is to list the file names under current directory that
//          match the prefix
//     ==> List all the matched file names alphabetically by:
//              cout << setw(16) << left << fileName;
//     ==> Print a new line for every 5 commands
//     ==> After printing, re-print the prompt and place the cursor back to
//          original location
//     --- 6.1 ---
//     Considering the following cases in which prefix is empty:
//     --- 6.1.1 ---
//     [Before] if prefix is empty, and in this directory there are multiple
//                 files and they do not have a common prefix,
//     cmd> help $sdfgh
//     [After] print all the file names
//     .                    ..                  Homework_3.docx Homework_3.pdf  Makefile
//     MustExist.txt    MustRemove.txt  bin                 dofiles            include
//     lib                 mydb                ref                 src                 testdb
//     cmd> help $sdfgh
//     --- 6.1.2 ---
//     [Before] if prefix is empty, and in this directory there are multiple
//                 files and all of them have a common prefix,
//     cmd> help $orld
//     [After]
//     ==> auto insert the common prefix and make a beep sound
//     ==> DO NOT print the matched files
//     cmd> help mydb-$orld
//     --- 6.1.3 ---
//     [Before] if prefix is empty, and only one file in the current directory
//     cmd> help $ydb
//     [After] print out the single file name followed by a ' '
//     cmd> help mydb $
//     --- 6.2 ---
//     [Before] with a prefix and with mutiple matched files
//     cmd> help M$Donald
//     [After]
//     Makefile          MustExist.txt    MustRemove.txt
//     cmd> help M$Donald
//     --- 6.3 ---
//     [Before] with a prefix and with mutiple matched files,
//                 and these matched files have a common prefix
//     cmd> help Mu$k
//     [After]
//     ==> auto insert the common prefix and make a beep sound
//     ==> DO NOT print the matched files
//     cmd> help Must$k
//     --- 6.4 ---
//     [Before] with a prefix and with a singly matched file
//     cmd> help MustE$aa
//     [After] insert the remaining of the matched file name followed by a ' '
//     cmd> help MustExist.txt $aa
//     --- 6.5 ---
//     [Before] with a prefix and NO matched file
//     cmd> help Ye$kk
//     [After] beep and stay in the same location
//     cmd> help Ye$kk
//
//     [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//     --- 7.1 ---
//     [Before] Cursor NOT on the first word and NOT matched command
//     cmd> he haha$kk
//     [After Tab]
//     ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str)
{
    // TODO...
    vector<string> splitStr = split(str);
    //part 1
    //no char other than space before cursor
    if (splitStr.empty())
    {
        _tabPressCount = 0;
        size_t count = 1, i = 1;
        cout << endl;
        for (CmdMap::iterator it = _cmdMap.begin(); it != _cmdMap.end(); it++)
        {
            cout << setw(12) << left << (it->first) + (it->second)->getOptCmd();
            if (count == 5 && i != _cmdMap.size())
            {
                cout << endl;
                count = 0;
            }
            count ++;
            i++;
        }
        reprintCmd();
        return;
    }
    
    //part 2, 3, 4
    //cursor on the first word
    else if (splitStr.size() == 1 && !isspace(str.back()))
    {
        _tabPressCount = 0;
        vector<string> command_name, match;
        for (CmdMap::iterator it = _cmdMap.begin(); it != _cmdMap.end(); it++)
        {
            command_name.push_back((it->first) + ((it->second)->getOptCmd()));
        }
        match = find_prefix_match(splitStr[0], command_name, false);
        //part 4
        //no command match
        if (match.empty()) return mybeep();
        //part 3
        //single command match
        else if (match.size() == 1)
        {
            for (size_t i = (splitStr[0].size() ); i < match[0].size(); i++) insertChar(char(match[0][i]));
            insertChar(' ');
            return;
        }
        //part 2
        //multiple commands match
        else
        {
            int count = 1;
            cout << endl;
            for (size_t i = 0; i < match.size(); i++)
            {
                cout << setw(12) << left << match[i];
                if (count == 5 && i != match.size() - 1)
                {
                    cout << endl;
                    count = 0;
                }
                count++;
            }
            reprintCmd();
            return;
            /*
            for (size_t i = 0; i < match.size(); i++)
            {
                int count = 1;
                cout << endl;
                for (size_t i = 0; i < match.size(); i++)
                {
                    cout << setw(12) << left << match[i];
                    if (count == 5)
                    {
                        cout << endl;
                        count = 0;
                    }
                    count ++;
                }
                reprintCmd();
                return;
            }
            */
        }
    }
    //part 5
    //first time tab
    //with first word matches the command and cursor on the other word
    else if (CmdExec* e = getCmd(splitStr[0]))
    {
        if (_tabPressCount == 1 || e != _prevtabCmd)
        {
            _prevtabCmd = e;
            cout << endl;
            e->usage(cout);
            //usage(e);
            //cout << endl;
            reprintCmd();
            return;        
        }
        else
        {
            vector<string> files;
            string prefix;
            char* dir = getcwd(NULL, 0);
            listDir(files, prefix, dir);
            //space before the cursor
            if (isspace(str.back()))
            {
                if (files.size() == 1)
                {
                    for (size_t i = 0; i < files[0].size(); i++) insertChar(char(files[0][i]));
                    insertChar(' ');
                    return;
                }
                else if (!prefix.empty())
                {
                    for (size_t i = 0; i < prefix.size(); i++) insertChar(char(prefix[i]));
                    return;
                }
                else
                {
                    int count = 1;
                    cout << endl;
                    for (size_t i = 0; i < files.size(); i++)
                    {
                        cout << setw(16) << left << files[i];
                        if (count == 5 && i != files.size() - 1)
                        {
                            cout << endl;
                            count = 0;
                        }
                        count ++;
                    }
                    reprintCmd();
                    return;
                }
            }
            //NOT space before cursor
            else
            {
                vector<string> match;
                string p;
                match = find_prefix_match(splitStr.back(), files, true);
                //no match file
                if (match.empty()) return mybeep();
                //single match file
                else if (match.size() == 1)
                {
                    for (size_t i = splitStr.back().size(); i < match[0].size(); i++) insertChar(char(match[0][i]));
                    insertChar(' ');
                    return;
                }
                //multiple match file
                else
                {
                    p = find_common_prefix(match);
                    //match files with more common prefix
                    if (p.size() > (splitStr.back().size() + 1))
                    {
                        for (size_t i = splitStr.back().size() + 1; i < p.size(); i++) insertChar(char(p[i]));
                        return mybeep();
                    }
                    //match files without other common prefix
                    else
                    {
                        int count = 1;
                        cout << endl;
                        for (size_t i = 0; i < match.size(); i++)
                        {
                            cout << setw(16) << left << match[i];
                            if (count == 5 && i != match.size() - 1)
                            {
                                cout << endl;
                                count = 0;
                            }
                            count++;
                        }
                        reprintCmd();
                        return;
                    }
                }
                
            }
        }
    }
}

vector<string>
CmdParser::split(string str)
{
    vector<string> split_str;
    char* str_c = new char[str.length() + 1], *sub;
    strcpy(str_c, str.c_str());
    sub = strtok(str_c, " ");
    while(sub)
    {
        split_str.push_back(string(sub));
        sub = strtok(NULL, " ");
    }
    return split_str;
}

vector<string>
CmdParser::find_prefix_match(string prefix, vector<string> listed, bool seNsiTIve)
{
    vector<string> matches = {};
    bool test;
    for (size_t i = 0; i < listed.size(); i++)
    {
        test = true;
        for (size_t j = 0; j < prefix.size(); j++)
        {
            if (seNsiTIve)
            {
                if (listed[i][j] != prefix[j]) test = false;
            }
            else
            {
                char ch1 = (isupper(listed[i][j]))? tolower(listed[i][j]) : listed[i][j];
                char ch2 = (isupper(prefix[j]))? tolower(prefix[j]) : prefix[j];
                if (ch1 != ch2) test = false;
            }
        }
        if (test) matches.push_back(listed[i]);
    }
    return matches;
}

string
CmdParser::find_common_prefix(vector<string> matches)
{
    string prefix = "";
    size_t index = 0;
    bool common = true;
    char c, n;
    while (common)
    {
        prefix += c;
        if (index < matches[0].size()) c = (matches[0][index]);
        else break;
        for (size_t i = 1; i < matches.size(); i++)
        {
            n = char(matches[i][index]);
            if (n != c) common = false;
        }
        index++;
    }
    return prefix;
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
    CmdExec* e = 0;
    // TODO...
    //cerr << _cmdMap.size();
    CmdMap::iterator it;
    for (it = _cmdMap.begin(); it != _cmdMap.end(); it++)
    {
        /*
        if (myStrNCmp(it->first, cmd, (it->first).size()) == 0)
        //if (myStrNCmp(cmd, it->first, (it->first).size()) == 0)
        {
            if (((it->second)->getOptCmd()).find(cmd) != string::npos)
            {
                e = it->second;
                //cerr << e->getOptCmd() << endl;
                break;
            }
        }
        cerr << (it->second)->getOptCmd() << endl;
        cerr << (it->first) << endl;
        string s = (it->first) + (it->second)->getOptCmd();
        cerr << s << endl;
        */
        if (myStrNCmp((it->first)+(it->second)->getOptCmd(), cmd, (it->first).size()) == 0)
        {
            e = it->second;
            break;
        }
    }
    return e;
}


//----------------------------------------------------------------------
//     Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
    string err;
    myStrGetTok(option, err);
    if (err.size()) {
        errorOption(CMD_OPT_EXTRA, err);
        return false;
    }
    return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
    size_t n = myStrGetTok(option, token);
    if (!optional) {
        if (token.size() == 0) {
            errorOption(CMD_OPT_MISSING, "");
            return false;
        }
    }
    if (n != string::npos) {
        errorOption(CMD_OPT_EXTRA, option.substr(n));
        return false;
    }
    return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
    string token;
    size_t n = myStrGetTok(option, token);
    while (token.size()) {
        tokens.push_back(token);
        n = myStrGetTok(option, token, n);
    }
    if (nOpts != 0) {
        if (tokens.size() < nOpts) {
            errorOption(CMD_OPT_MISSING, "");
            return false;
        }
        if (tokens.size() > nOpts) {
            errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
            return false;
        }
    }
    return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
    switch (err) {
        case CMD_OPT_MISSING:
            cerr << "Error: Missing option";
            if (opt.size()) cerr << " after (" << opt << ")";
            cerr << "!!" << endl;
        break;
        case CMD_OPT_EXTRA:
            cerr << "Error: Extra option!! (" << opt << ")" << endl;
        break;
        case CMD_OPT_ILLEGAL:
            cerr << "Error: Illegal option!! (" << opt << ")" << endl;
        break;
        case CMD_OPT_FOPEN_FAIL:
            cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
        break;
        default:
            cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
        exit(-1);
    }
    return CMD_EXEC_ERROR;
}

