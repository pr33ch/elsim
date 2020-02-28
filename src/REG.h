#ifndef REG_H_
#define REG_H_

#include "param.h"

// Register
// Edge-triggered -- output can only change on rising edge of CLK
// 
// N-bit input D
// 1-bit input CLK
// N-bit output Q

class REG : public Module
{
public:
	REG(int N)
	{
		addInput("D", N);
		addInput("CLK");
		addOutput("Q", N);

		std::stringstream ss;
		ss << "REG<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		if (posedge("CLK"))
			OUT("Q") <= IN("D");
	}

	delay_t delay(int inum, int onum)
	{
		if (inum == numInputs() - 1)
			return REG_CLK2Q;
		return DELAY_T_MIN;
	}

	delay_t load(int inum) const
	{
		int N = numInputs();
		if (inum == N - 1)
			return N*2*LOAD_NAND;
		return LOAD_NAND;
	}

	area_t area() const
	{
		return REG_AREA * numInputs();
	}

	energy_t energy(int onum) const
	{
		return REG_ENERGY;
	}
};

// Latch
// Not edge-triggered -- transparent when CLK is high.
//
// N-bit input D
// 1-bit input CLK
// N-bit output Q

class LATCH : public Module
{
public:
	LATCH(int N)
	{
		addInput("D", N);
		addInput("CLK");
		addOutput("Q", N);

		std::stringstream ss;
		ss << "LATCH<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		if (Bit(IN("CLK")) == HIGH)
			OUT("Q") <= IN("D");
	}

	delay_t delay(int inum, int onum)
	{
		if (inum == onum || inum == numOutputs())
			return LATCH_CLK2Q;
		return DELAY_T_MIN;
	}

	delay_t load(int inum) const
	{
		int N = numInputs();
		if (inum == N - 1)
			return N*2*LOAD_NAND;
		return LOAD_NAND;
	}

	area_t area() const
	{
		return LATCH_AREA * numInputs();
	}

	energy_t energy(int onum) const
	{
		return LATCH_ENERGY;
	}
};

#endif // REGISTER_H_
