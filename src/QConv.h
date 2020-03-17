#ifndef QConv_H_
#define QConv_H_

#define QCONV_DELAY 25*DELAY_NAND(2)

#include <math.h>
#include "Module.h"
#include "param.h"

class QConv : public Module
// implements the on-the-fly conversion module for Radix-2 divider with carry-save residual
{
	private:
		size_t width_;
		bool test_;
		unsigned int q_ = 0;

		bool neg_w(BitVector ws, BitVector wc) // check if w is negative
		{
			Bit c(LOW);
			Bit s;

			// add ws and wc together
			for (size_t i = 0; i < width_; i++)
			{
				s = (ws.get(i) ^ wc.get(i) ^ c);
				c = ((ws.get(i) & wc.get(i)) | (ws.get(i) & c) | (wc.get(i) & c));
			}

			if (s == HIGH || c == HIGH)
			{
				return true;
			}

			return false;
		}

		void connect_output(bool neg_w)
		{
			unsigned int temp = q_;
			
			unsigned int tempQN;
			if (neg_w)
			{
				tempQN = q_ - 1;
			}
			else
			{
				std::cout << "positive y" << std::endl;
				tempQN = q_;
			}
			std::cout << "stored Q: " << q_ << std::endl;
			std::cout << "stored QN: " << tempQN << std::endl;
			for (size_t i = 0; i < width_; i++)
			{
				if (temp & 1)
				{
					OUT("Q", i) <= Bit(HIGH);
				}
				else
				{
					OUT("Q", i) <= Bit(LOW);	
				}
				temp >>= 1;

				if (tempQN & 1)
				{
					OUT("QN", i) <= Bit(HIGH);
				}
				else
				{
					OUT("QN", i) <= Bit(LOW);	
				}
				tempQN >>= 1;
			}
			// std::cout << "iterative QN: " << OUT("QN") << std::endl;
		}
	public: 
		QConv(size_t N, bool test=false)
		{
			std::stringstream ss;
			ss << "QConv";
			classname_ = ss.str();
			width_ = N;

			test_ = test;

			addInput("CLK");
			addInput("q", 2); // sign and magnitude inputs for q: (sign, magnitude)
			addInput("WS", N); 
			addInput("WC", N);
			addOutput("Q", N);
			addOutput("QN", N); // output used for the last iteration of the division algorithm
		}

		void propagate()
		{
			if (posedge("CLK") || test_)
			{
				std::cout << "......qconv......" << std::endl;
				std::cout << "q: " << IN("q") << std::endl;
				std::cout << "WS: " << IN("WS") << std::endl;
				std::cout << "WC: " << IN("WC") << std::endl;

				Bit magnitude = IN("q", 0);
				Bit sign = IN("q", 1);
				BitVector ws = IN("WS");
				BitVector wc = IN("WC");

				if (magnitude == LOW) // q = 0
				{
					q_ <<= 1;
					connect_output(neg_w(ws, wc));
				}
				else if (magnitude == HIGH && sign == LOW) // q = 1
				{
					q_ <<= 1;
					q_ |= 1;
					connect_output(neg_w(ws, wc));
				}
				else if (magnitude == HIGH && sign == HIGH) // q = -1
				{
					q_ -= 1;
					q_ <<= 1;
					q_ |= 1;
					connect_output(neg_w(ws, wc));
				}
				std::cout << "................." << std::endl;
			}
		}

		delay_t delay(int inum, int onum)
		{
			// find the delay in book
			if (inum == 0) // delay for CLK input
			{
				return DELAY_T_MIN;
			}
			else if((inum == 1 || inum == 2) && onum < width_) // delay for q sel
			{
				return QCONV_DELAY;
			}
			else if (inum > 2 && onum >= width_) // delay for carry save result
			{
				return QCONV_DELAY;
			}
		}

		delay_t load(int inum) const
		{
			if (inum == 0)
			{
				return LOAD_NAND; // taken from REG.h for determining load of the CLK input
			}
			else if (inum == 1 || inum == 2) // q is connected to control ports of 3 MUX's
			{
				return 2*3*LOAD_AND;
			}
			else
			{
				return LOAD_XOR + LOAD_OR; // load for the carry save results
			}
		}

		area_t area() const
		{
			return 1360*AREA_NAND(2); // according to figure 5.5 in Digital Arithmetic, pp. 265 section 5.3.1	
		}
};

#endif