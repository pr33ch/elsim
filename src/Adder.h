#ifndef ADDER_H_
#define ADDER_H_

#include "SystemModule.h"

// Abstract class for N-bit adder
// N-bit inputs X,Y
// 1-bit input Ci
// N-bit output S
// 1-bit output Co

class Adder : public SystemModule
{
protected:
	// input width
	int width_;
	// sub-module adders (freed in destructor)
	std::vector<Adder*> adders_;

public:
	Adder(int N)
	{
		width_ = N;
		addInput("X", N);
		addInput("Y", N);
		addInput("Ci");
		addOutput("S", N);
		addOutput("Co");
	}

	~Adder()
	{
		// free adders
		std::vector<Adder*>::iterator iter;
		for (iter = adders_.begin(); iter != adders_.end(); iter++)
			delete *iter;
	}

	// built-in array of sub-adders used for convenience
	void addAdder(Adder* a)
	{
		assert(a);
		adders_.push_back(a);
		submodule(a);
	}

	int width() const { return width_; }

	// All Adders must be able to clone() themselves.
	// This is so an arbitrary Adder can be passed to a constructor and then copied for internal use.
	virtual Adder* clone() const = 0;
};

#endif // ADDER_H_
