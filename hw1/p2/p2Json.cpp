/****************************************************************************
  FileName     [ p2Json.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Json and JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include "p2Json.h"
#include <fstream>
#include <iomanip>
#include <cstring>

using namespace std;

// Implement member functions of class Row and Table here
bool
Json::read(const string& jsonFile)
{
	ifstream f(jsonFile);
	if (!f.is_open())	return false;
	string s;
	vector<string> content;
	while (getline(f, s))
	{
		if ((s != "{") && (s != "}") && (s != "\0"))
		{
			content.push_back(s);
		}
	}
	//for (int i = 0; i < content.size(); i++)	cout << content[i] << endl;	//try
	f.close();
	for (vector<string>::iterator it=content.begin();it!=content.end();it++)
	//for (int i = 0; i < content.size(); i++)
	{
		string key;
		int value;
		char *org = new char[(*it).length() + 1], *sub;
		strcpy(org, (*it).c_str());
		sub = strtok(org, " :,?!\t\r\\\"\b\r");	//split char array
		if ((*sub == '{') || (*sub == '}')) continue;
		key = sub;
		//cout << key << endl;
		sub = strtok(NULL, " :,?!\t\r\\\"\b\r");	//continue to split
		value = atoi(sub);
		//cout << key << endl;
		//cout << value << endl;
		JsonElem j(key, value);
		_obj.push_back(j);
		delete [] org;
	}
	content.clear();
	return true;
}

void
Json::print()
{
	if (_obj.empty())
	{
		cout << "{" << endl;
		cout << "}" << endl;
	}
	else
	{
		cout << "{" << endl;
		vector<JsonElem>::iterator it;
		for (it=_obj.begin();it!=_obj.end()-1;it++)
		//for (int i = 0; i < _obj.size() - 1; i++)
		{
			cout << "  \"" << (*it).getkey() << "\" : " << (*it).getval() << "," << endl;
		}
		cout << "  \"" << (*it).getkey() << "\" : " << (*it).getval() << endl;
		cout << "}" << endl;
	}
}

void
Json::sum()
{
	if (_obj.empty())
	{
		cerr << "Error: No element found!!" << endl;
	}
	else
	{
		int sum = 0;
		vector<JsonElem>::iterator it;
		for (it=_obj.begin();it!=_obj.end();it++)
		//for (int i = 0; i < _obj.size(); i++)
		{
			sum += (*it).getval();
		}
		cout << "The summation of the values is: " << sum << "." << endl;
	}
}

void
Json::ave()
{
	if (_obj.empty())
	{
		cerr << "Error: No element found!!" << endl;
	}
	else
	{
		int sum = 0;
		vector<JsonElem>::iterator it;
		for (it=_obj.begin();it!=_obj.end();it++)
		//for (int i = 0; i < _obj.size(); i++)
		{
			sum += (*it).getval();
		}
		float ave = (float) sum/(float) _obj.size();
		cout << "The average of the values is: " << fixed << setprecision(1) << ave << "." << endl;
	}
}

void
Json::max()
{
	if (_obj.empty())
	{
		cerr << "Error: No element found!!" << endl;
	}
	else
	{
		int maxi = _obj[0].getval();
		string maxi_key = _obj[0].getkey();
		vector<JsonElem>::iterator it;
		for (it=_obj.begin();it!=_obj.end();it++)
		//for (int i = 0; i < _obj.size(); i++)
		{
			//cout << (*it).getval() << endl;
			if ((*it).getval() > maxi)
			{
				maxi = (*it).getval();
				maxi_key = (*it).getkey();
			}
		}
		cout << "The maximum element is: { \"" << maxi_key << "\" : " << maxi << " }." << endl;
	}
}

void
Json::min()
{
	if (_obj.empty())
	{
		cerr << "Error: No element found!!" << endl;
	}
	else
	{
		int mini = _obj[0].getval();
		string mini_key = _obj[0].getkey();
		vector<JsonElem>::iterator it;
		for (it=_obj.begin();it!=_obj.end();it++)
		//for (int i = 0; i < _obj.size(); i++)
		{
			if ((*it).getval() < mini)
			{
				mini = (*it).getval();
				mini_key = (*it).getkey();
			}
		}
		cout << " " << _obj.size();
		cout << "The minimum element is: { \"" << mini_key << "\" : " << mini << " }." << endl;
	}
}

void
Json::add()
{
	string key;
	int value;
	cin >> key >> value;
	JsonElem j(key, value);
	_obj.push_back(j);	
}

ostream&
operator << (ostream& os, const JsonElem& j)
{
   return (os << "\"" << j._key << "\" : " << j._value);
}