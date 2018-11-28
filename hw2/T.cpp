#include <iomanip>
#include <iostream>
using namespace std;


int main()
{
	cout << "cmd> ";
	cout << "abcde";
	cout << '\b' << '\b';
	//insert "k"
	cout << 'k';

	cout << "de\b\b";


	//if backspace is pressed
	cout << "\bde \b\b\b";

	cout.flush();
	while (1);
}


//array to memorize what is on the screen
//pointer
//
