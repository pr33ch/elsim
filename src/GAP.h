#ifndef GAP_H_
#define GAP_H_

#include "Module.h"
#include "param.h"

// Generate/Alive/Propagate
// N-bit inputs X,Y
// N-bit outputs g,a,p

class GAP : public Module
{
public:
	GAP(int N)
	{
		addInput("X", N);
		addInput("Y", N);

		addOutput("g", N);
		addOutput("a", N);
		addOutput("p", N);

		std::stringstream ss;
		ss << "GAP<" << N << ">";
		classname_ = ss.str();
	}

	void propagate()
	{
		BitVector X = IN("X");
		BitVector Y = IN("Y");
		OUT("g") <= (X & Y);
		OUT("a") <= (X | Y);
		OUT("p") <= (X ^ Y);
	}

	delay_t delay(int inum, int onum)
	{
		int N = numInputs() / 2;
		if (inum >= N) inum -= N;

		// if its a `g` output
		if (onum < N)
			{ if (inum == onum) return DELAY_AND(2); }
		// if its a `a` output
		if (onum < 2*N)
			{ if (inum == (onum-N)) return DELAY_OR(2); }
		// if its a `p` output
		else
			{ if (inum == (onum-2*N)) return DELAY_XOR(2); }

		// inum and onum are unrelated
		return DELAY_T_MIN;
	}

	delay_t load(int inum) const
	{
		return LOAD_AND + LOAD_OR + LOAD_XOR;
	}

	area_t area() const
	{
		int N = numInputs() / 2;
		return N*(AREA_AND(2) + AREA_OR(2) + AREA_XOR(2));
	}

	energy_t energy(int onum) const
	{
		int N = numInputs() / 2;
		if (onum < N) return 2*ENERGY_AND;
		if (onum < 2*N) return 2*ENERGY_OR;
		return 2*ENERGY_XOR;
	}
};

#endif // GAP_H_
