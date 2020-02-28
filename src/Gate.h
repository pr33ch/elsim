#ifndef GATE_H_
#define GATE_H_

#include "Module.h"
#include "param.h"

// Defines 6 gate classes: AND, NAND, OR, NOR, XOR, XNOR
// Gates have N 1-bit inputs and one 1-bit output.
// Delay/area/energy/load parameters are taken from param.h

#define DEFINE_GATE_CLASS(OP)             \
class OP : public Module                  \
{                                         \
public:                                   \
	OP(int Nin=2)                         \
	{                                     \
		assert(Nin > 1);                  \
		addInputs(Nin);                   \
		addOutputs(1);                    \
		if (Nin == 2)                     \
			classname_ = #OP;             \
		else                              \
		{                                 \
			std::stringstream ss;         \
			ss << #OP "<" << Nin << ">";  \
			classname_ = ss.str();        \
		}                                 \
	}                                     \
	void propagate()                      \
	{                                     \
		int Nin = numInputs();            \
		Bit z = IN(0);                    \
		for (int i = 1; i < Nin; i++)     \
		{                                 \
			Bit b = IN(i);                \
			z.OP(b);                      \
		}                                 \
		OUT(0) <= z;                      \
	}                                     \
	delay_t delay(int inum, int onum)     \
	{                                     \
		return DELAY_##OP(numInputs());   \
	}                                     \
	delay_t load(int inum) const          \
	{                                     \
		return LOAD_##OP;                 \
	}                                     \
	area_t area() const                   \
	{                                     \
		return AREA_##OP(numInputs());    \
	}                                     \
	energy_t energy(int onum) const       \
	{                                     \
		return numInputs() * ENERGY_##OP; \
	}                                     \
}

DEFINE_GATE_CLASS(AND);
DEFINE_GATE_CLASS(NAND);
DEFINE_GATE_CLASS(OR);
DEFINE_GATE_CLASS(NOR);
DEFINE_GATE_CLASS(XOR);
DEFINE_GATE_CLASS(XNOR);

#endif // GATE_H_
