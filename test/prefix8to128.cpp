#include <iostream>
#include "sim.h"
using namespace std;

int main(void)
{
	srandom(time(NULL));
	cout << "WIDTH,AREA,CRITPATH,AVGDELAY,AVGPOWER,PEAKPOWER" << endl;
	for (int N = 8; N <= 128; N *= 2)
	{
		PrefixAdder adder(N);
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
	return 0;
}
