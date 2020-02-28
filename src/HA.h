#ifndef HA_H_
#define HA_H_

#include "Module.h"
#include "param.h"

// Half-Adder
// 1-bit inputs X and Y
// 1-bit outputs S and C

class HA : public Module
{
public:
	HA()
	{
		classname_ = "HA";

		addInput("X");
		addInput("Y");

		addOutput("S");
		addOutput("C");
	}

	void propagate()
	{
		Bit x = IN("X");
		Bit y = IN("Y");

		OUT("S") <= (x ^ y);
		OUT("C") <= (x & y);
	}

	delay_t delay(int inum, int onum)
	{
		if (onum == 0)
			return DELAY_XOR(2); // fan-in 2
		return DELAY_AND(2);
	}

	delay_t load(int inum) const
	{
		return LOAD_AND + LOAD_XOR;
	}

	area_t area() const
	{
		return AREA_XOR(2) + AREA_AND(2); // fan-in 2
	}

	energy_t energy(int onum) const
	{
		if (onum == 0) return 2*ENERGY_XOR;
		return 2*ENERGY_AND;
	}
};

#endif // HA_H_
