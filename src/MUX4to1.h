#ifndef MUX4to1_H_
#define MUX4to1_H_

#include "Module.h"

// 4:1 Multiplexer
//
// N-bit inputs A,B,C,D
// 2-bit input SEL
// N-bit output Z

class MUX4to1 : public Module
{
public:
	MUX4to1(int N=1)
	{
		addInput("A", N);
		addInput("B", N);
		addInput("C", N);
		addInput("D", N);
		addInput("SEL", 2);

		addOutput("Z", N);

		std::stringstream ss;
		ss << "4:1MUX<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		BitVector sel = IN("SEL");

		if (sel.get(0) == LOW && sel.get(1) == LOW)
			OUT("Z") <= IN("A");
		else if (sel.get(0) == LOW && sel.get(1) == HIGH)
			OUT("Z") <= IN("B");
		else if (sel.get(0) == HIGH && sel.get(1) == LOW)
			OUT("Z") <= IN("C");
		else if (sel.get(0) == HIGH && sel.get(1) == HIGH)
			OUT("Z") <= IN("D");
	}

	delay_t delay(int inum, int onum)
	{
		int N = numOutputs();
		// only A[i]->Z[i], B[i]->Z[i], C[i]->Z[i], D[i]->Z[i], SEL->Z[i] paths exist
		if (inum > 4*N)
		{
			return DELAY_INV + DELAY_AND(3) + DELAY_OR(2); // select signal delay
		}
		else if ((int)inum/N != (int)onum/N)
		{
			return DELAY_T_MIN; 
		}
		return DELAY_AND(3) + DELAY_OR(2);
	}

	delay_t load(int inum) const
	{
		int N = numOutputs();
		// A,B,C,D inputs
		if (inum < 4*N) return LOAD_AND;
		// SEL input
		return N*(3*LOAD_AND + 2*LOAD_INV);
	}

	area_t area() const
	{
		return numOutputs() * (4*AREA_AND(3) + 3*AREA_OR(2) + 2*AREA_INV);	// fan-in 2
	}
};

#endif // MUX4to1_H_
