#ifndef SKIPADDER_H_
#define SKIPADDER_H_

#include "GroupPropagate.h"
#include "MUX.h"
#include "RippleAdder.h"

// Carry-skip adder
// Sub-adders can be arbitrary Adders that provide a P (group propagate) output.
// Basic case uses RippleAdder for sub-adders.

class SkipAdder : public Adder
{
private:
	// number of groups
	int Ngrp_;
	// `propagate` computation for each group
	std::vector<GroupPropagate> propagates_;
	// MUX after each group
	std::vector<MUX> muxs_;

	// connect all submodules together
	void init()
	{
		int gsz = width() / Ngrp_;
		int L = 0, H = gsz-1;

		for (int i = 0; i < Ngrp_; i++, H+=gsz, L+=gsz)
		{
			Adder& a = *adders_[i];
#ifdef MOD_EXTRA
			a.tag = i;
			propagates_[i].tag = i;
			muxs_[i].tag = i;
#endif
			// X,Y go to group adder
			IN("X",H,L) >> a("X");
			IN("Y",H,L) >> a("Y");
			// X,Y go to `propagate` module
			IN("X",H,L) >> propagates_[i]("X");
			IN("Y",H,L) >> propagates_[i]("Y");
			// adder sum goes to global sum
			OUT("S",H,L) << a("S");
			// adder Cout goes to MUX[A]
			a("Co") >> muxs_[i]("A");
			// EITHER previous MUX goes to MUX[B] and adder Cin
			if (i) { muxs_[i-1] >> muxs_[i]("B");
			         muxs_[i-1] >> a("Ci");       }
			// OR global Cin goes to MUX[B] and adder Cin
			else { IN("Ci") >> muxs_[i]("B");
			       IN("Ci") >> a("Ci");       }
			// MUX select comes from `propagate`
			propagates_[i]("P") >> muxs_[i]("SEL");
#ifdef DEBUG
std::cout << "SKIP INIT DONE " << i << std::endl;
#endif
		}

		// final MUX produced global Cout
		OUT("Co") << muxs_[Ngrp_-1];
	}

	// constructor helper
	void SkipAdder_helper(const Adder& grpadder)
	{
		int N = width();
		int gsz = grpadder.width();
		assert(gsz <= N);
		assert(N % gsz == 0);

		Ngrp_ = N / gsz;
		adders_.reserve(Ngrp_);

		// create group adders, register Propagates/MUXs
		for (int i = 0; i < Ngrp_; i++)
		{
			addAdder(grpadder.clone());
			submodule(&propagates_[i]);
			submodule(&muxs_[i]);
		}

		// generate classname based on group adder type
		std::stringstream ss;
		ss << "SkipAdder<" << N << "," << grpadder.classname() << ">";
		classname_ = ss.str();

		// connect submodules together
		init();
	}

public:
	// basic carry-skip adder with ripple groups
	SkipAdder(int N, int groupSize) : 
		Adder(N),
		propagates_(N / groupSize, GroupPropagate(groupSize)),
		muxs_(N / groupSize, MUX())
	{
		SkipAdder_helper(RippleAdder(groupSize));
	}

	// carry-skip adder with arbitrary adder for groups
	SkipAdder(int N, const Adder& grpadder) : 
		Adder(N),
		propagates_(N / grpadder.width(), GroupPropagate(grpadder.width())),
		muxs_(N / grpadder.width(), MUX())
	{
		SkipAdder_helper(grpadder);
	}

	// copy constructor
	SkipAdder(const SkipAdder& other) : 
		Adder(other.width()),
		propagates_(other.Ngrp_, GroupPropagate(other.width()/other.Ngrp_)),
		muxs_(other.Ngrp_, MUX())
	{
		SkipAdder_helper(*other.adders_[0]);
	}

	SkipAdder* clone() const
	{
		return new SkipAdder(*this);
	}
};

#endif // SKIPADDER_H_
