#ifndef RIPPLEADDER_H_
#define RIPPLEADDER_H_

#include "Adder.h"
#include "FullAdder.h"

// Carry-ripple adder
// With N-bit inputs, consists of N/M M-bit sub-adders.
// In the basic case, the sub-adders are 1-bit FullAdders.

class RippleAdder : public Adder
{
private:
	// connect all submodules together
	void init()
	{
		int Ngrp = adders_.size();
		int gsz = width() / Ngrp;
		int H = gsz - 1;
		int L = 0;

		for (int i = 0; i < Ngrp; i++, H+=gsz, L+=gsz)
		{
			Adder& a = *adders_[i];
#ifdef MOD_EXTRA
			a.tag = i;
#endif
			IN("X",H,L)  >> a("X");
			IN("Y",H,L)  >> a("Y");
			OUT("S",H,L) << a("S");

			if (i) (*adders_[i-1])("Co") >> a("Ci");
			else                IN("Ci") >> a("Ci");
		}

		Adder& a = *adders_[Ngrp-1];
		OUT("Co") << a("Co");
	}

public:
	// hierarchical (N bits total, groups defined by given adder)
	RippleAdder(int N, const Adder& grpadder=FullAdder()) : Adder(N)
	{
		int gsz = grpadder.width();
		assert(N % gsz == 0);
		int Ngrp = N / gsz;
		adders_.reserve(Ngrp);

		// create group adders
		for (int i = 0; i < Ngrp; i++)
			addAdder(grpadder.clone());

		// generate classname based on group adder type
		std::stringstream ss;
		ss << "RippleAdder<" << N << "," << grpadder.classname() << ">";
		classname_ = ss.str();

		// connect submodules together
		init();
	}

	// copy constructor
	RippleAdder(const RippleAdder& other) : Adder(other.width())
	{
		int Ngrp = other.adders_.size();
		adders_.reserve(Ngrp);

		// create group adders
		for (int i = 0; i < Ngrp; i++)
			addAdder(other.adders_[i]->clone());

		// copy classname from other
		classname_ = other.classname_;

		// connect submodules together
		init();
	}

	RippleAdder* clone() const
	{
		return new RippleAdder(*this);
	}
};

#endif // RIPPLEADDER_H_
