/****************************************************************************
  FileName      [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis      [ Define database Json member functions ]
  Author         [ Chung-Yang (Ric) Huang ]
  Copyright     [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"
#include <limits>
#include <cstring>

using namespace std;

/*****************************************/
/*             Global Functions                 */
/*****************************************/
ostream&
operator << (ostream& os, const DBJsonElem& j)
{
    os << "\"" << j._key << "\" : " << j._value;
    return os;
}

istream& operator >> (istream& is, DBJson& j)
{
    // TODO: to read in data from Json file and store them in a DB 
    // - You can assume the input file is with correct JSON file format
    // - NO NEED to handle error file format
    assert(j._obj.empty());
    string s;
    vector<string> content;
    while (1)
    {
        getline(is, s);
        if (s == "}") break;
        if ((s != "{") && (s != "}") && (s != "\0"))
        {
            content.push_back(s);
        }
    }
    //is.close();
    for (vector<string>::iterator it = content.begin(); it != content.end(); it++)
    {
        string key;
        int value;
        char* org = new char[(*it).length() + 1], *sub;
        strcpy(org, (*it).c_str());
        sub = strtok(org, " :,?!\t\r\\\"\b\r");
        if ((*sub == '{') || (*sub == '}')) continue;
        key = sub;
        sub = strtok(NULL, " :,?!\t\r\\\"\b\r");
        value = atoi(sub);
        DBJsonElem k(key, value);
        j._obj.push_back(k);
        delete [] org;
    }
    content.clear();
    j._read = true;
    return is;
}

ostream& operator << (ostream& os, const DBJson& j)
{
    // TODO
    if (j.empty()) os << "{\n}" << endl;
    else
    {
        os << "{" << endl;
        size_t i;
        for (i = 0; i < j.size() - 1; i++)
        {
            //os << "  \"" << j._obj[i].key() << "\" : " << j._obj[i].value() << "," << endl;
            os << "  " << j._obj[i] << "," << endl;
        }
        //os << "  \"" << j._obj[i].key() << "\" : " << j._obj[i].value() << endl;
        os << "  " << j._obj[i] << endl;
        os << "}";
    }
    return os;
}

/**********************************************/
/*    Member Functions for class DBJsonElem     */
/**********************************************/
/*****************************************/
/*    Member Functions for class DBJson    */
/*****************************************/
void
DBJson::reset()
{
    // TODO
    _read = false;
    _obj.clear();
}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
    // TODO
    for (size_t i = 0; i < size(); i++)
    {
        if (elm.key() == _obj[i].key()) return false;
    }
    _obj.push_back(elm);
    return true;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
    // TODO
    if (empty()) return numeric_limits<double>::quiet_NaN();
    return float(sum())/float(size());
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
    // TODO
    if (empty())
    {
        idx = size();
        return INT_MIN;
    }
    int maxN = _obj[0].value();
    idx = 0;
    for (size_t i = 0; i < size(); i++)
    {
        if (_obj[i].value() > maxN)
        {
            maxN = _obj[i].value();
            idx = i;
        }
    }
    return  maxN;
}

// If DBJson is empty, set idx to size() and return INT_MAX
int
DBJson::min(size_t& idx) const
{
    // TODO
    if (empty())
    {
        idx = size();
        return INT_MAX;
    }
    int minN = _obj[0].value();
    idx = 0;
    for (size_t i = 0; i < size(); i++)
    {
        if (_obj[i].value() < minN)
        {
            minN = _obj[i].value();
            idx = i;
        }
    }
    /*for (vector<DBJsonElem>::iterator it = _obj.begin(); it != _obj.end(); it++)
    {
        if ((*it).value() < minN)
        {
            minN = (*it).value();
            idx = 
        }
    }*/
    return  minN;
}

void
DBJson::sort(const DBSortKey& s)
{
    // Sort the data according to the order of columns in 's'
    ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
    // Sort the data according to the order of columns in 's'
    ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
    // TODO
    if (empty()) return 0;
    int s = 0;
    for (size_t i = 0; i < size(); i++)
    {
        s += _obj[i].value();
    }
    return s;
}
