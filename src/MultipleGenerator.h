#ifndef Multiple_Generator_H_
#define Multiple_Generator_H_

#include "Module.h"
#include "param.h"

// takes as inputs the multiplicand, x, and the output of the parallel recoder. Undefined behavior for bit widths close to the OS limit
class MultipleGenerator : public Module
{
	private:
		int x_width_;
		int mth_pp_;
		int sim_calls_;
	public: 
		MultipleGenerator(int N, int m) // N-bit multiplicand, mth partial product
		{
			std::stringstream ss;
			ss << "MultipleGenerator<" << m << ">";
			classname_ = ss.str();
			x_width_ = N;
			mth_pp_ = m;
			sim_calls_ = 0;
			addInput("X", N); // multiplicand
			addInput("sign");
			addInput("one");
			addInput("two");

			addOutput("pp", 2*N); // partial product

		}

		void propagate()
		{
			sim_calls_++;
			std::cout << "simulated " << sim_calls_ << " times for " << classname_ << std::endl;
			Bit sign = IN("sign");
			Bit one = IN("one");
			Bit two = IN("two");
			BitVector X(2*x_width_,0);
			BitVector Xshifted(2*x_width_,0);
			// BitVector NotX(2*x_width_,0);
			// BitVector NotXshifted(2*x_width_,0);
			int j = 0;
			for (int i = mth_pp_*2; i < mth_pp_*2+x_width_; i++)
			{
				Bit n = IN("X",j);
				// std::cout << n;
				X.set(i, n);
				Xshifted.set(i+1, n);
				j++;
			}

			BitVector NotX = ~X;
			BitVector NotXshifted = ~Xshifted;

			for (int i = 0; i < mth_pp_*2; i++)
			{
				NotX.set(i,0);
				NotXshifted.set(i,0);
			}
			// std::cout << std::endl;

			// std::cout << "sign " << sign << std::endl;
			// std::cout << "one " << one << std::endl;
			// std::cout << "two " << two << std::endl;
			// std::cout << "X " << X << std::endl;
			// std::cout << "2X " << Xshifted << std::endl;
			// std::cout << "~X " << (NotX) << std::endl;
			// std::cout << "~2X " << (NotXshifted) << std::endl;


			if (one.bit == 1 && sign.bit == 0)
			{
				// std::cout <<"same" << std::endl;
				OUT("pp") <= X;
			}
			else if(one.bit == 1 && sign.bit == 1)
			{
				OUT("pp") <= NotX;
			}
			else if(two.bit == 1 && sign.bit == 0)
			{
				OUT("pp") <= Xshifted;
			}
			else if (two.bit == 1 && sign.bit == 1)
			{
				OUT("pp") <= NotXshifted;
			}
			else
			{
				OUT("pp") <= 0;
			}
			// unsigned int x = 0;
			// for (int i = x_width_-1; i >= 0; i--)
			// {
			// 	std::cout << x << " or "<< (unsigned int) Bit(IN("X", i)) <<std::endl;
			// 	x = ((unsigned int)Bit(IN("X", i)).bit) | x;
				
			// 	if (i != 0)
			// 	{
			// 		x = x<<1;
			// 	}
			// }
			// std::cout << std::endl;
			
			// std::cout << "multiplicand " << x << std::endl;
			// if (one.bit == 0 && two.bit == 0)
			// {
			// 	std::cout << "uh oh" << std::endl;
			// 	x = 0;
			// }
			// else if(two.bit == 1)
			// {
			// 	x = 2*x;
			// }

			// if (sign.bit == 1)
			// {
			// 	x = x;
			// }

			// std::cout << "multiplicand " << x << std::endl;

			// int mask = 1;
			// for (int i = 0; i < 2*x_width_; i++)
			// {
			// 	// std::cout << i << std::endl;
			// 	if (i >= mth_pp_*2)
			// 	{
			// 		// std::cout << (mask&x) << std::endl;
			// 		OUT("pp", i) <= Bit(mask & x);
			// 		x = x>>1;
			// 	}
			// 	else
			// 	{
			// 		OUT("pp", i) <= Bit(0);
			// 	}
			// }
			std::cout << "partial product: " << OUT("pp") << std::endl;
		}

		delay_t delay(int inum, int onum)
		{
			return DELAY_AND(2) + DELAY_OR(2);
		}

		delay_t load(int inum) const
		{
			return 2*LOAD_AND;
		}

		area_t area() const
		{
			return 2*AREA_AND(2) + AREA_OR(2);	
		}
};

#endif