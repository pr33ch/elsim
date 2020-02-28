#ifndef Recorder_H_
#define Recorder_H_

#include "Module.h"
#include "param.h"

// recodes rad 2 numbers into rad 4

class Recoder : public Module
{
	public: 
		Module()
		{
			classname_ = "Recoder";
			addInput("M0");
			addInput("M1");
			addInput("C");

			addOutput("one");
			addOutput("neg");
			addOutput("zero");
			addOutput("Cnext");
		}

		void propagate()
		{
			Bit m0 = IN(0);
			Bit m1 = IN(1);
			Bit c = IN(2);

			OUT(0) = m0 ^ c;
			OUT(1) = (m1 & c) | (m1 & m0);
			OUT(2) = (m0 & m1 & c) | (~m0 & ~m1 & ~c); // second input to or gate is the same as ~(a|b|c)
			OUT(3) = (m1 & m0) | (m1 & c);
		}

		delay_t delay(int inum, int onum)
		{
			// DELAY_XOR(2); // critical path delay is delay of 2 XOR gates
			if (onum == 0)
			{
				return DELAY_XOR(2);
			}
			else if (onum == 1)
			{
				return DELAY_AND(2) + DELAY_OR(2);
			}
			else if (onum == 2)
			{
				return DELAY_AND(3) + DELAY_OR(2);
			}
			else if (onum ==3)
			{
				return DELAY_AND(2) + DELAY_OR(2);
			}
		}

		delay_t load(int inum) const
		{
			// for c: 3 AND
			if (inum == 0)
			{
				return LOAD_XOR + 3*LOAD_AND + LOAD_NOR;
			}
			else if (inum == 1)
			{
				return 5*LOAD_AND + LOAD_NOR;
			}
			else if (inum == 2)
			{
				return LOAD_XOR + 3*LOAD_AND + LOAD_NOR;
			}
		}

		area_t area() const
		{
			return AREA_XOR(2) + 4*AREA_AND(2) + AREA_AND(3) + AREA_NOR(3);	
		}

}