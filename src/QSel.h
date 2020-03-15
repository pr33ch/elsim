#ifndef QSel_H_
#define QSel_H_
#include <math.h>
#include "Module.h"
#include "param.h"

class QSel : public Module
// implements the selection function module for Radix-2 divider with carry-save residual
{
	private:
	size_t width_ = 0;

	public: 
		QSel() // N-bit carry-save representation of y_hat
		{
			size_t N = 4;
			std::stringstream ss;
			ss << "Qsel<" << N << ">";
			classname_ = ss.str();
			width_ = N;

			addInput("WC", N);
			addInput("WS", N);
			addOutput("qs"); // q's sign
			addOutput("qm"); // q's magnitude
		}

		void propagate()
		{
			// from example 5.2, section 5.5 in Digital Arithmetic:
			BitVector c = IN("WC");
			BitVector s = IN("WS");
			BitVector p = c^s;
			BitVector g = c&s;
			OUT("qs") <= (p.get(0) ^ (g.get(1) | (p.get(1) & g.get(2)) | (p.get(1) & p.get(2) & g.get(3))));
			OUT("qm") <= ~(p.get(1) & p.get(2) & p.get(3));
		}

		delay_t delay(int inum, int onum)
		{
			if (onum == 0)
			{
				return DELAY_XOR(width_) + DELAY_XOR(2) + DELAY_AND(3) + DELAY_OR(3);
			}
			else if(onum == 1)
			{
				return DELAY_XOR(width_) + DELAY_AND(3) + DELAY_INV;
			}
		}

		delay_t load(int inum) const
		{
			return LOAD_XOR + LOAD_AND;
		}

		area_t area() const
		{
			return AREA_XOR(width_) + AREA_AND(width_) + 2*AREA_AND(3) + AREA_AND(2) + AREA_OR(3) + AREA_XOR(2) + AREA_INV;	
		}
};

#endif