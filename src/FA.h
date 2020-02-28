#ifndef FA_H_
#define FA_H_

#include "Module.h"
#include "param.h"

// Full-Adder
// 1-bit inputs X, Y, Ci
// 1-bit outputs S, Co

class FA : public Module
{
public:
	FA()
	{
		classname_ = "FA";

		addInput("X");
		addInput("Y");
		addInput("Ci");

		addOutput("S");
		addOutput("Co");
	}

	void propagate()
	{
		Bit x = IN(0);
		Bit y = IN(1);
		Bit c = IN(2);
		// std::cout << x << "\t" << y << "\t" << c << std::endl;
		OUT(0) <= (x ^ y ^ c);

		OUT(1) <= ((x & y) | (x & c) | (y & c));
	}

	delay_t delay(int inum, int onum)
	{
		if (onum == 0)		// all fan-in 2, except OR
			return DELAY_XOR(2) + ((inum == 2) ? 0 : DELAY_XOR(2));
		return DELAY_OR(3) + DELAY_AND(2);
	}

	delay_t load(int inum) const
	{
		// for c: 3 AND
		if (inum == 2) return 3*LOAD_AND;
		// for x and y: 1 XOR, 2 AND
		return 2*LOAD_AND + LOAD_XOR;
	}

	area_t area() const
	{
		return 2*AREA_XOR(2) + 3*AREA_AND(2) + AREA_OR(2);	// fan-in 2, except OR
	}

	energy_t energy(int onum) const
	{
		if (onum == 0) return 2*2*ENERGY_XOR;
		return 3*ENERGY_OR + 3*2*ENERGY_AND;
	}
};

#endif // FA_H_
