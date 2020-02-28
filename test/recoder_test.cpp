#include <iostream>
#include <fstream>
#include <cstdlib>
#include "Rad4Multiplier12b.h"
using namespace std;

void testRecoder(Rad4Multiplier12b& recoder, value_t x, value_t y, Bit c)
{
	// set inputs and simulate
	recoder("X") <= x;
	cout << recoder("X") << endl;
	recoder("Y") <= y;
	cout << recoder("Y") << endl;
	recoder("Ci") <= c;
	cout << recoder("Ci") << endl;
	recoder.simulate();
	cout << x << " * " << y << " = " << "(" << recoder("Co") << ")" << recoder("P") << endl;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "Usage: " << argv[0] << " X Y Cin" << endl;
		return -1;
	}

	// read input values
	value_t x = atoi(argv[1]);
	value_t y = atoi(argv[2]);
	Bit c = Bit(!!atoi(argv[3]));

	Rad4Multiplier12b recoder(12);

	// test adders
	testRecoder(recoder, x, y, c);

}