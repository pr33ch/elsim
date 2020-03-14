#ifndef Parallel_Recoder_H_
#define Parallel_Recoder_H_

#include "Module.h"
#include "param.h"

// recodes rad 2 multipliers into rad 4 without carry propagation

class ParallelRecoder : public Module
{
	private:
		int N_;
	public: 
		ParallelRecoder(int N)
		{
			classname_ = "ParallelRecoder";
			addInput("y2j-1"); // bit to the right of the two multiplier bits
			addInput("y2j"); 
			addInput("y2j+1");
			// addInput("Xi", N);

			addOutput("sign");
			addOutput("c");
			addOutput("one");
			addOutput("two");
			// addOutput("Xo", N);

			N_ = N;
		}

		void propagate()
		{
			Bit btr = IN("y2j-1");
			Bit y0 = IN("y2j");
			Bit y1 = IN("y2j+1");
			// BitVector x = IN("Xi");
			// std::cout << "y2j-1 " << btr << std::endl;
			// std::cout << "y2j " << y0 << std::endl;
			// std::cout << "y2j+1 " << y1 << std::endl;
			// std::cout << "Xi " << x << std::endl;
			OUT("sign") <= y1;
			OUT("c") <= y1;
			OUT("one") <= (y0 ^ btr); 
			OUT("two") <= ((y1 & ~y0 & ~btr) | (~y1 & y0 & btr));
			// OUT("Xo") <= x;

			// for (int i = 0; i < N_; i++)
			// {
			// 	OUT("Xo", i) <= x.get(i);
			// }

			// std::cout << "sign " << OUT("sign") << std::endl;
			// std::cout << "c " << OUT("c") << std::endl;
			// std::cout << "one " << OUT("one") << std::endl;
			// std::cout << "two " << OUT("two") << std::endl;
			// std::cout << "Xo " << OUT("Xo") << std::endl;
		}

		delay_t delay(int inum, int onum)
		{
			// DELAY_XOR(2); // critical path delay is delay of 2 XOR gates
			
			// return DELAY_AND(3) + DELAY_OR(2);
			return DELAY_XOR(2);
		}

		delay_t load(int inum) const
		{
			// for c: 3 AND
			if (inum == 0)
			{
				return 2*LOAD_AND;
			}
			else if (inum == 1 || inum == 2)
			{
				return 2*LOAD_AND + LOAD_XOR;
			}
		}

		area_t area() const
		{
			return AREA_XOR(2) + 2*AREA_AND(3) + AREA_OR(2);	
		}
};
#endif