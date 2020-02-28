#include <iostream>
#include "sim.h"
using namespace std;

// Simulate an adder 1024 times with random inputs.
// Report the delay, area, and power stats.
void testAdder(Adder& adder)
{
	// Static computations of area and critical path.
	area_t area = adder.area();
	delay_t Tcrit = adder.criticalPath();
	delay_t Tmax = 0;
	delay_t Ttot = 0;
	energy_t Etot = 0;
	energy_t Ppeak = 0;

	cout << adder << endl;
	cout << "WIDTH,CRIT_DELAY,MAX_DELAY,AREA,AVG_POW,PEAK_POW" << endl;

	// Simulate with random inputs
	for (int i = 0; i < 1024; i++)
	{
		value_t x = random();
		value_t y = random();
		Bit c = random() & 1;

		adder("X") <= x;
		adder("Y") <= y;
		adder("Ci") <= c;
		adder.simulate();

		// record delay
		delay_t T = adder.lastOutputTime();
		Ttot += T;
		if (T > Tmax) Tmax = T;

		// record power
		energy_t avgpow, peakpow;
		adder.simPowerStats(&avgpow, &peakpow);
		Etot += avgpow * T;
		if (peakpow > Ppeak) Ppeak = peakpow;

		adder.reset();
	}

	// print stats
	energy_t avgpow = Etot / Ttot;
	cout << adder.width() << "," << Tcrit << "," << Tmax << "," << area << "," << avgpow << "," << Ppeak << endl;
}

int main(int argc, char** argv)
{
	srandom(time(NULL));

	// simulate various adder types with increasing input width
	for (int i = 8; i <= 64; i *= 2)
	{
		RippleAdder cra(i);
		testAdder(cra);

		SkipAdder csk(i,i/4);
		testAdder(csk);

		SelectAdder csel(i,LookaheadAdder(i/4));
		testAdder(csel);

		LookaheadAdder cla(i,LookaheadAdder(i/4));
		testAdder(cla);

		PrefixAdder pre(i);
		testAdder(pre);
	}

	return 0;
}
