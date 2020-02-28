#ifndef INV_H_
#define INV_H_

#include "Module.h"
#include "param.h"

// Inverter
// N-bit input X
// N-bit output Z

class INV : public Module
{
public:
	INV(int N)
	{
		addInput("X", N);
		addOutput("Z", N);

		std::stringstream ss;
		ss << "INV<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		BitVector vec = IN("X");
		OUT("Z") <= vec.NOT();
	}

	delay_t delay(int inum, int onum)
	{
		if (inum != onum) return DELAY_T_MIN;
		return DELAY_INV;
	}

	delay_t load(int onum) const
	{
		return LOAD_INV;
	}

	area_t area() const
	{
		return numInputs() * AREA_INV;
	}

	energy_t energy(int onum) const
	{
		return ENERGY_INV;
	}
};

// Buffer
// N-bit input X
// N-bit output Z

class BUF : public Module
{
public:
	BUF(int N)
	{
		addInput("X", N);
		addOutput("Z", N);

		std::stringstream ss;
		ss << "BUF<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		OUT("Z") <= IN("X");
	}

	delay_t delay(int inum, int onum)
	{
		if (inum != onum) return DELAY_T_MIN;
		return 2*DELAY_INV;
	}

	delay_t load(int onum) const
	{
		return LOAD_INV;
	}

	area_t area() const
	{
		return numInputs() * 2 * AREA_INV;
	}

	energy_t energy(int onum) const
	{
		return 2*ENERGY_INV;
	}
};

#endif // INV_H_
