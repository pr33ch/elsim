#ifndef SAVEADDER_H_
#define SAVEADDER_H_

#include "SystemModule.h"
#include "FA.h"

// Carry-save adder (3-to-2)
// N-bit inputs: X,Y,Ci
// N-bit outputs: S,Co

class SaveAdder : public SystemModule
{
private:
	FA* FAs_;
public:
	SaveAdder(int N)
	{
		addInput("X", N);
		addInput("Y", N);
		addInput("Ci", N);

		addOutput("S", N);
		addOutput("Co", N);

		std::stringstream ss;
		ss << "SaveAdder<" << N << ">";
		classname_ = ss.str();

		// create and connect FAs
		FAs_ = new FA[N];
		for (int i = 0; i < N; i++)
		{
			FA& fa = FAs_[i];
			submodule(&fa);
			IN("X",i) >> fa("X");
			IN("Y",i) >> fa("Y");
			IN("Ci",i) >> fa("Ci");
			OUT("S",i) << fa("S");
			OUT("Co",i) << fa("Co");
		}
	}

	~SaveAdder() { delete [] FAs_; }

	int width() const { return numOutputs() / 2; }
};

#endif // SAVEADDER_H_
