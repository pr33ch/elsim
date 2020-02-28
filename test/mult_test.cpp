#include <iostream>
#include <fstream>
#include <cstdlib>
#include "Rad4Multiplier12b.h"
// #include "Rad4Multiplier12bDesign2.h"
// #include "Rad4Multiplier12bDesign3.h"
using namespace std;

void testMultiplier(Rad4Multiplier12b& multiplier, value_t x, value_t y, Bit c)
{
	// set inputs and simulate
	multiplier("X") <= x;
	cout << multiplier("X") << endl;
	multiplier("Y") <= y;
	cout << multiplier("Y") << endl;
	multiplier("Ci") <= c;
	cout << multiplier("Ci") << endl;
	multiplier.visualize();
	multiplier.simulate();
	cout << x << " * " << y << " = " << "(" << multiplier("Co") << ")" << multiplier("P") << endl;

	// compute static critical path
	int i, o;
	delay_t T = multiplier.criticalPath(&i, &o);
	cout << "Static critical path: ";
	if (T < 0) cout << "no path" << endl;
	else cout << multiplier.nameOfInput(i) << " -> "
	          << multiplier.nameOfOutput(o) << " (T=" << T << ")" << endl;

	// compute area
	cout << "Area: " << multiplier.area() << endl;
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

	Rad4Multiplier12b multiplier(12);

	// test adders
	testMultiplier(multiplier, x, y, c);

}