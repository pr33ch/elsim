#ifndef InputCtrl_H_
#define InputCtrl_H_

#include "Module.h"

// input control for Radix2 Carry Save Divider. Outputs LOW for the first rising clock edge, otherwise HIGH

class InputCtrl : public Module
{
private:
	bool first_clk_ = true;
	delay_t T_;
public:
	InputCtrl(delay_t clk_period)
	{
		T_ = clk_period;
		addInput("CLK");

		addOutput("CTRL");

		std::stringstream ss;
		ss << "InputCtrl";
		classname_ = ss.str();
	}

	void propagate()
	{
		if (posedge("CLK"))
		{
			if (first_clk_)
			{
				// OUT("CTRL") <= Bit(LOW);
				setOutput(0, Bit(LOW), 1);
				first_clk_ = false;
			}
			else
			{
				// OUT("CTRL") <= Bit(HIGH);
				setOutput(0, Bit(HIGH), 1);
			}
		}
	}

	delay_t delay(int inum, int onum)
	{
		// int N = numOutputs();
		// // only A[i]->Z[i] and B[i]->Z[i] paths exist
		// if (inum < N)
		// {
		// 	if (inum != onum) return DELAY_T_MIN;
		// }
		// else
		// {
		// 	int tmp = inum - N;
		// 	if (tmp < N && tmp != onum) return DELAY_T_MIN;
		// }
		return DELAY_T_MIN;	// fan-in 2
	}

	delay_t load(int inum) const
	{
		return 1;
	}

	area_t area() const
	{
		return 0;	// fan-in 2
	}

	energy_t energy(int onum) const
	{
		return 0;
	}
};

#endif // InputCtrl_H_
