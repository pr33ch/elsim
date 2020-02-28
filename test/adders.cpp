#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sim.h"
using namespace std;

// Test an adder with given inputs.
void testAdder(Adder& adder, value_t x, value_t y, Bit c)
{
	// set inputs and simulate
	adder("X") <= x;
	adder("Y") <= y;
	adder("Ci") <= c;
	adder.simulate();

	// print the adder name
	cout << adder << endl;;

	// print the results and the times
	cout << x << " + " << y << " + " << c << " = " << "(" << adder("Co") << ")" << adder("S") << endl;
	cout << "Sum   at T = " << adder.lastTime("S")  << endl;
	cout << "Carry at T = " << adder.lastTime("Co") << endl;

	// compute static critical path
	int i, o;
	delay_t T = adder.criticalPath(&i, &o);
	cout << "Static critical path: ";
	if (T < 0) cout << "no path" << endl;
	else cout << adder.nameOfInput(i) << " -> "
	          << adder.nameOfOutput(o) << " (T=" << T << ")" << endl;

	// compute area
	cout << "Area: " << adder.area() << endl;

	// compute average/peak power of the simulation
	energy_t avgpow, maxpow;
	adder.simPowerStats(&avgpow, &maxpow);
	cout << "Avg. power: " << avgpow << endl;
	cout << "Peak power: " << maxpow << endl;
	cout << endl;
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

	// create several adder types
	RippleAdder ripadder(8);
	SkipAdder   skpadder(8, 2);
	// SelectAdder seladder(8, RippleAdder(4));
	SelectAdder seladder(8, RippleAdder(1));
	LookaheadAdder claadder(8);
	LookaheadAdder hcladder(8,LookaheadAdder(2));
	PrefixAdder preadder(8);

	// test adders
	testAdder(ripadder, x, y, c);
	testAdder(skpadder, x, y, c);
	testAdder(seladder, x, y, c);
	testAdder(claadder, x, y, c);
	testAdder(hcladder, x, y, c);
	testAdder(preadder, x, y, c);

	// write a waveform file for the last adder's outputs
	VCDWriter vcdw;
	vcdw.addSignal(preadder, "S");
	vcdw.addSignal(preadder, "Co");
	ofstream ofs("dump.vcd");
	vcdw.write(ofs);

	return 0;
}
