#include <iostream>
#include "sim.h"
using namespace std;

void testAdder(Adder& adder)
{
	int N = adder.width();
	area_t area = adder.area();
	delay_t Tcrit = adder.criticalPath();
	delay_t Ttot = 0;
	energy_t Etot = 0, Pmax = 0;

	for (int i = 0; i < 256; i++)
	{
		adder("X") <= BitVector::random(N);
		adder("Y") <= BitVector::random(N);
		adder("Ci") <= Bit::random();
		adder.simulate();

		delay_t T = adder.lastOutputTime();
		Ttot += T;

		energy_t avgpow, peakpow;
		adder.simPowerStats(&avgpow,&peakpow);
		Etot += avgpow * T;
		if (peakpow > Pmax) Pmax = peakpow;

		adder.reset();
	}
	energy_t Pavg = Etot / Ttot;
	cout << N << "," << area << "," << Tcrit << "," << (Ttot/256.0) << "," << Pavg << "," << Pmax << endl;
}

int main(void)
{
	srandom(time(NULL));
	cout << "WIDTH,AREA,CRITPATH,AVGDELAY,AVGPOWER,PEAKPOWER" << endl;

	// 2 groups of 4
	LookaheadAdder a1(8, LookaheadAdder(4));
	testAdder(a1);

	// 4 groups of 4
	LookaheadAdder a2(16, LookaheadAdder(4));
	testAdder(a2);

	// 2 groups of 16
	// each 16 is 4 groups of 4
	LookaheadAdder a3(32, a2);
	testAdder(a3);

	// 4 groups of 16
	// each 16 is 4 groups of 4
	LookaheadAdder a4(64, a2);
	testAdder(a4);

	// 2 groups of 64
	// each 64 is 4 groups of 16
	// each 16 is 4 groups of 4
	LookaheadAdder a5(128, a4);
	testAdder(a5);

	return 0;
}
