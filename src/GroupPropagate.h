#ifndef GROUPPROPAGATE_H_
#define GROUPPROPAGATE_H_

#include "Gate.h"

// This module computes `Propagate` over a group of N bits.
// P = AND{i=0..N-1} Xi XOR Yi
//
// N-bit inputs X,Y
// 1-bit output P

class GroupPropagate : public Module
{
public:
	GroupPropagate(int N)
	{
		addInput("X", N);
		addInput("Y", N);

		addOutput("P");

		std::stringstream ss;
		ss << "GroupPropagate<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		Bit p(HIGH);
		int N = numInputs() / 2;

		for (int i = 0; i < N; i++)
		{
			Bit x = IN("X",i);
			Bit y = IN("Y",i);
			p.AND(x ^ y);
		}

		OUT("P") <= p;
	}

	delay_t delay(int inum, int onum)
	{
		// 2-input XOR, N-input AND
		int N = numInputs() / 2;
		return DELAY_XOR(2) + DELAY_AND(N);
	}

	delay_t load(int inum) const
	{
		return LOAD_XOR;
	}

	area_t area() const
	{
		// N 2-input XORs, 1 N-input AND
		int N = numInputs() / 2;
		return N*AREA_XOR(2) + AREA_AND(N);
	}

	energy_t energy(int onum) const
	{
		// N 2-input XORs, 1 N-input AND
		int N = numInputs() / 2;
		return N*2*ENERGY_XOR + N*ENERGY_AND;
	}
};

#endif // GROUPPROPAGATE_H_
