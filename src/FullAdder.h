#ifndef FULLADDER_H_
#define FULLADDER_H_

#include "Adder.h"
#include "HA.h"
#include "Gate.h"

// Full-adder that derives from Adder.
// Can be used as a sub-adder within a larger adder.
// Modeled as two HAs and an OR.

class FullAdder : public Adder
{
private:
	OR orC;
	HA ha0, ha1;

public:
	FullAdder() : Adder(1)
	{
		classname_ = "FullAdder";
		submodules(3, &orC, &ha0, &ha1);

		IN("X") >> ha0("X");
		IN("Y") >> ha0("Y");

		ha0("S") >> ha1("X");
		IN("Ci") >> ha1("Y");

		ha0("C") >> orC.IN(0);
		ha1("C") >> orC.IN(1);

		OUT("S") << ha1("S");
		OUT("Co") << orC;
	}

	FullAdder* clone() const
	{
		return new FullAdder();
	}
};

#endif // FULLADDER_H_
