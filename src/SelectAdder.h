#ifndef SELECTADDER_H_
#define SELECTADDER_H_

#include "Adder.h"
#include "MUX.h"

class SelectAdder : public Adder
{
private:
	// number of groups
	int Ngrp_;
	// MUX after each group
	std::vector<MUX> muxs_;

	// connect all submodules together
	void init()
	{
		int gsz = width() / Ngrp_;
		int H = gsz - 1;
		int L = 0;

		Adder& add0 = *adders_[0];
#ifdef MOD_EXTRA
		add0.tag = 0;
		muxs_[0].tag = 0;
#endif
		// global carry-in goes to first adder
		IN("Ci") >> add0("Ci");
		// lower `gsz` bits of global X and Y go to first adder
		IN("X",H,L) >> add0("X");
		IN("Y",H,L) >> add0("Y");
		// lower bits of global sum come from first adder sum
		OUT("S",H,L) << add0("S");
		// first adder carry out controls first mux
		add0("Co") >> muxs_[0]("SEL");

		// connect the rest of the adders and muxs
		for (int i = 1; i < Ngrp_; i++)
		{
			H += gsz;
			L += gsz;

			// get the two adders for this group
			Adder& a0 = *adders_[2*i-1];
			Adder& a1 = *adders_[2*i  ];
#ifdef MOD_EXTRA
			a0.tag = 2*i-1;
			a1.tag = 2*i;
			if (i < Ngrp_-1) muxs_[i].tag = i;
#endif
			// carry-ins are constant 0 and 1
			a0("Ci") <= 0; a0("Ci").hold();
			a1("Ci") <= 1; a1("Ci").hold();
			// both get a range of global X and Y bits
			IN("X",H,L) >> a0("X");
			IN("X",H,L) >> a1("X");
			IN("Y",H,L) >> a0("Y");
			IN("Y",H,L) >> a1("Y");
			// sum outputs go to the mux
			a0("S") >> muxs_[i-1]("A",gsz-1,0);
			a1("S") >> muxs_[i-1]("B",gsz-1,0);
			// carry-outs go to the mux
			a0("Co") >> muxs_[i-1]("A",gsz);
			a1("Co") >> muxs_[i-1]("B",gsz);
			// global sum bits come from mux output
			OUT("S",H,L) << muxs_[i-1]("Z",gsz-1,0);
			// actual carry-out controls next mux
			if (i < Ngrp_-1) muxs_[i-1]("Z",gsz) >> muxs_[i]("SEL");
			// or becomes the global carry-out
			else OUT("Co") << muxs_[i-1]("Z",gsz);
		}
	}

	// constructor helper
	void _SelectAdder(const Adder& grpadder)
	{
		int N = width();
		int gsz = grpadder.width();
		assert(gsz <= N);
		assert(N % gsz == 0);

		Ngrp_ = N / gsz;
		int Nadders = 2*Ngrp_ - 1;
		adders_.reserve(Nadders);

		// create group adders
		for (int i = 0; i < Nadders; i++)
			addAdder(grpadder.clone());

		// register MUXs as submodules
		for (int i = 0; i < Ngrp_-1; i++)
			submodule(&muxs_[i]);

		// generate classname based on group adder type
		std::stringstream ss;
		ss << "SelectAdder<" << N << "," << grpadder.classname() << ">";
		classname_ = ss.str();

		// connect submodules together
		init();
	}

public:
	// constructor
	SelectAdder(int N, const Adder& grpadder) :
		Adder(N),
		muxs_(N / grpadder.width() - 1, MUX(grpadder.width() + 1)) // Ngrp-1 MUXs, each of size gsz+1
	{
		_SelectAdder(grpadder);
	}

	// copy constructor
	SelectAdder(const SelectAdder& other) :
		Adder(other.width()),
		muxs_(other.Ngrp_ - 1, MUX(other.width() / other.Ngrp_ + 1)) // Ngrp-1 MUXs, each of size gsz+1
	{
		_SelectAdder(*other.adders_[0]);
	}

	SelectAdder* clone() const
	{
		return new SelectAdder(*this);
	}
};

#endif // SELECTADDER_H_
