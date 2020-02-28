#ifndef MUX_H_
#define MUX_H_

#include "Module.h"

// Multiplexer
// Z = (A AND NOT SEL) OR (B AND SEL)
//
// N-bit inputs A,B
// 1-bit input SEL
// N-bit output Z

class MUX : public Module
{
public:
	MUX(int N=1)
	{
		addInput("A", N);
		addInput("B", N);
		addInput("SEL");

		addOutput("Z", N);

		std::stringstream ss;
		ss << "MUX<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		Bit sel = IN("SEL");

		if (sel == LOW)
			OUT("Z") <= IN("A");
		else if (sel == HIGH)
			OUT("Z") <= IN("B");
		else
			OUT("Z") <= BitVector(numOutputs()); // undefined bits
	}

	delay_t delay(int inum, int onum)
	{
		int N = numOutputs();
		// only A[i]->Z[i] and B[i]->Z[i] paths exist
		if (inum < N)
		{
			if (inum != onum) return DELAY_T_MIN;
		}
		else
		{
			int tmp = inum - N;
			if (tmp < N && tmp != onum) return DELAY_T_MIN;
		}
		return DELAY_AND(2) + DELAY_OR(2);	// fan-in 2
	}

	delay_t load(int inum) const
	{
		int N = numOutputs();
		// A,B inputs
		if (inum < 2*N) return LOAD_AND;
		// SEL input
		return N*2*LOAD_AND;
	}

	area_t area() const
	{
		return numOutputs() * (2*AREA_AND(2) + AREA_OR(2));	// fan-in 2
	}

	energy_t energy(int onum) const
	{
		return 2*2*ENERGY_AND + 2*ENERGY_OR;
	}
};

#endif // MUX_H_
