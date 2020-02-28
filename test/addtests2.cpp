#include <iostream>
#include "sim.h"
using namespace std;

// Simulate an adder 1024 times with random inputs.
// Report the delay/area/power stats.
void testAdder(Adder& adder)
{
	area_t area = adder.area();
	delay_t Tcrit = adder.criticalPath();
	delay_t Ttot = 0;
	energy_t Etot = 0;

	cout << adder << endl;
	cout << "CRIT_DELAY,AREA,AVG_POW" << endl;

	for (int i = 0; i < 1024; i++)
	{
		value_t x = random();
		value_t y = random();
		Bit c = random() & 1;

		adder("X") <= x;
		adder("Y") <= y;
		adder("Ci") <= c;
		adder.simulate();

		delay_t T = adder.lastOutputTime();
		Ttot += T;

		energy_t avgpow, peakpow;
		adder.simPowerStats(&avgpow, &peakpow);
		Etot += avgpow * T;

		adder.reset();
	}

	energy_t avgpow = Etot / Ttot;
	cout << Tcrit << "," << area << "," << avgpow << endl << endl;
}

int main(int argc, char** argv)
{
	srandom(time(NULL));

	// 64-bit adders
	const int N = 64;

	// Carry-ripple (for comparison)
	{
	RippleAdder cra(N);
	testAdder(cra);
	}

	// Various adders using CLA sub-adders
	for (int m = 4; m <= 32; m *= 2)
	{
	LookaheadAdder subcla(N/m);

	RippleAdder cra(N,subcla);
	testAdder(cra);

	SelectAdder csel(N,subcla);
	testAdder(csel);

	LookaheadAdder cla(N,subcla);
	testAdder(cla);
	}

	// Prefix adder
	PrefixAdder pre(N);
	testAdder(pre);

	// Hierarchical CLAs

	// 32 groups of 2
	// 16 groups of 4
	//  8 groups of 8
	//  4 groups of 16
	//  2 groups of 32
	LookaheadAdder cla2(64, LookaheadAdder(32, LookaheadAdder(16,
						LookaheadAdder(8, LookaheadAdder(4, LookaheadAdder(2))))));
	testAdder(cla2);

	// 16 groups of 4
	//  8 groups of 8
	//  4 groups of 16
	LookaheadAdder cla4(64, LookaheadAdder(16, LookaheadAdder(4)));
	testAdder(cla4);

	// 8 groups of 8
	LookaheadAdder cla8(64, LookaheadAdder(8));
	testAdder(cla8);

	// 4 groups of 16
	LookaheadAdder cla16(64, LookaheadAdder(16));
	testAdder(cla16);

	return 0;
}
