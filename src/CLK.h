#ifndef CLK_H_
#define CLK_H_

#include "Module.h"

// Clock pulse generator
// No inputs.
// 1-bit output CLK.

class CLK : public Module
{
private:
	bool propagated_;
	int Ncycle_;
	delay_t highT_;
	delay_t lowT_;
	delay_t Tstart_;

public:
	// Ncycle : number of cycles
	// highT  : time that the clock is high (value 1)
	// lowT   : time that the clock is low (value 0)
	// Tstart : time that the first cycle (high) starts
	CLK(int Ncycle, delay_t highT, delay_t lowT, delay_t Tstart=0)
	{
		assert(Ncycle > 0);
		assert(highT > 0);
		assert(lowT > 0);
		assert(Tstart >= 0);

		propagated_ = false;
		Ncycle_     = Ncycle;
		highT_      = highT;
		lowT_       = lowT;
		Tstart_     = Tstart;

		std::stringstream ss;
		ss << "CLK<" << Ncycle << "," << highT << "," << lowT << ">";
		classname_ = ss.str();

		addOutputs(1);
	}

	// must be called exactly once at the start of a simulation
	void propagate()
	{
		// will only be propagated once (as a root module) because there are no inputs
		assert(!propagated_);
		propagated_ = true;

		assert(simTime() <= Tstart_);
		delay_t period = highT_ + lowT_;

		// generate all output edges
		for (int c = 0; c < Ncycle_; c++)
		{
			setOutput(0, Bit(HIGH), Tstart_ + c*period         );
			setOutput(0, Bit(LOW ), Tstart_ + c*period + highT_);
		}
	}

	// should never be called (clock has no inputs)
	delay_t delay(int inum, int onum)
	{
		assert(0);
		return DELAY_T_MIN;
	}

	// should never be called (clock has no inputs)
	delay_t load(int inum) const
	{
		assert(0);
		return 0;
	}

	// can be nonzero if you want to consider clock as a physical module
	area_t area() const
	{
		return 0;
	}

	energy_t energy(int onum) const
	{
		// can be whatever you want
		return 1;
	}
};

#endif // CLOCK_H_
