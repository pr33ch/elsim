#ifndef GP_H_
#define GP_H_

#include "Module.h"
#include "param.h"

// Generate/Propagate
// Standard cell for prefix adders.
// 1-bit inputs g,gPrev,p,pPrev
// 1-bit outputs G,P
//
// P = p AND pPrev
// G = g OR (p AND gPrev)

class GP : public Module
{
public:
	GP()
	{
		addInput("g");
		addInput("gPrev");
		addInput("p");
		addInput("pPrev");

		addOutput("G");
		addOutput("P");

		classname_ = "GP";
	}

	void propagate()
	{
		Bit g = IN("g");
		Bit p = IN("p");
		Bit gPrev = IN("gPrev");
		Bit pPrev = IN("pPrev");
		OUT("G") <= (g | (p & gPrev));
		OUT("P") <= (p & pPrev);
	}

	delay_t delay(int inum, int onum)
	{
		if (onum == 0) // G output
		{
			if (inum <= 2)
				return DELAY_OR(2) + (inum ? DELAY_AND(2) : 0);
		}
		else // P output
		{
			if (inum >= 2)
				return DELAY_AND(2);
		}
		return DELAY_T_MIN;
	}

	delay_t load(int inum) const
	{
		if (inum == 0) return LOAD_OR;
		if (inum == 2) return 2*LOAD_AND;
		return LOAD_AND;
	}

	area_t area() const
	{
		return 2*AREA_AND(2) + AREA_OR(2);
	}

	energy_t energy(int onum) const
	{
		return 2*ENERGY_AND + (onum ? 0 : 2*ENERGY_OR);
	}
};

#endif // GAP_H_
