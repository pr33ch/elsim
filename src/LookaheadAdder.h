#ifndef LOOKAHEADADDER_H_
#define LOOKAHEADADDER_H_

#include "Adder.h"
#include "GAP.h"
#include "LookaheadGenerator.h"
#include "MetaGate.h"

// Carry-lookahead adder
// The sub-adders can be any Adder that provides 1-bit G,P (generate,propagate) outputs.
// The basic case does a full lookahead computation (no sub-adders).

class LookaheadAdder : public Adder
{
private:
	GAP gap;
	LookaheadGenerator clg;
	MetaGate<XOR> xorsum;
	MetaGate<AND> cgenAnd;
	MetaGate<OR> cgenOr;

	void init()
	{
		int N = width();

		// X,Y inputs go to GAP
		IN("X") >> gap("X");
		IN("Y") >> gap("Y");

		// g,a go to carry lookahead
		gap("g") >> clg("g");
		gap("a") >> clg("p");

		// final G,P are outputs (for hierarchical CLA)
		OUT("G") << clg("G",N-2);
		OUT("P") << clg("P",N-2);

		// cumulative Ps are ANDed with Ci
		gap("p",0) >> cgenAnd("0",0);
		clg("P")   >> cgenAnd("0",N-1,1);

		for (int i = 0; i < N; i++)
			IN("Ci") >> cgenAnd("1",i);

		// AND goes to OR
		cgenAnd >> cgenOr("0");
		// to be ORed with cumulative Gs
		gap("g",0) >> cgenOr("1",0);
		clg("G")   >> cgenOr("1",N-1,1);

		// final sums are `p`s XORed with final carries
		gap("p") >> xorsum("0");

		IN("Ci") >> xorsum("1",0);
		cgenOr("Z",N-2,0) >> xorsum("1",N-1,1);

		OUT("S") << xorsum("Z");

		// final carry-out comes from last carry generation
		OUT("Co") << cgenOr("Z",N-1);
	}

	void init2()
	{
		int Ngrp = adders_.size();
		int gsz = width() / Ngrp;
		int L = 0, H = gsz-1;

		for (int i = 0; i < Ngrp; i++, L+=gsz, H+=gsz)
		{
			Adder& a = *adders_[i];
			IN("X",H,L) >> a("X");
			IN("Y",H,L) >> a("Y");
			OUT("S",H,L) << a("S");

			// group adders G,P goes to 2nd-level lookahead
			a("G") >> clg("g",i);
			a("P") >> clg("p",i);

			// 2nd-level G,P go to carry-generation
			if (i) { clg("G",i-1) >> cgenOr("1",i);
			         clg("P",i-1) >> cgenAnd("0",i); }
			else   { a("G") >> cgenOr("1",0);
			         a("P") >> cgenAnd("0",0); }

			// global carry-in goes to each AND
			IN("Ci") >> cgenAnd("1",i);
			// group adders carry-in comes from OR output (1st is system carry-in)
			if (i) cgenOr("Z",i-1) >> a("Ci");
			else   IN("Ci") >> a("Ci");
		}

		// AND outputs go to OR 0-inputs
		cgenAnd >> cgenOr("0");
		// final carry out from last carry-generation
		OUT("Co") << cgenOr("Z",Ngrp-1);
	}

	bool hierarchical() const
	{
		return adders_.size() > 0;
	}

public:
	// Basic CLA which does full lookahead for all N bits.
	LookaheadAdder(int N) : 
		Adder(N),
		gap(N),
		clg(N),
		xorsum(2,N),
		cgenAnd(2,N),
		cgenOr(2,N)
	{
		// register submodules
		submodule(&gap);
		submodule(&clg);
		submodule(&xorsum);
		submodule(&cgenAnd);
		submodule(&cgenOr);

		// add final G,P outputs for hierarchical CLA
		addOutput("G", 1);
		addOutput("P", 1);

		// generate classname
		std::stringstream ss;
		ss << "LookaheadAdder<" << N << ">";
		classname_ = ss.str();

		// connect submodules together
		init();
	}

	// copy constructor
	LookaheadAdder(const LookaheadAdder& other) :
		Adder(other.width()),
		gap(other.hierarchical() ? 1 : other.width()), // unused for hierarchical, so save space
		clg(other.hierarchical() ? other.adders_.size() : other.width()),
		xorsum(2,other.hierarchical() ? 1 : other.width()), // unused for hierarchical, so save space
		cgenAnd(2,other.width()),
		cgenOr(2,other.width())
	{
		int Ngrp = other.adders_.size();
		bool hierarchical = Ngrp > 0;

		// register submodules
		submodule(&clg);
		submodule(&cgenAnd);
		submodule(&cgenOr);
		if (!hierarchical)
		{
			submodule(&gap);
			submodule(&xorsum);
		}

		// add final G,P outputs for hierarchical CLA
		addOutput("G", 1);
		addOutput("P", 1);

		// copy classname from other
		classname_ = other.classname_;

		// add subadders if hierarchical
		if (hierarchical)
		{
			for (int i = 0; i < Ngrp; i++)
				addAdder(other.adders_[i]->clone());
		}

		// connect submodules together
		hierarchical ? init2() : init();
	}

	// 2-level CLA using copies of `grpadder`
	// `grpadder` doesn't have to be CLA, but must have 1-bit outputs "G" and "P"
	LookaheadAdder(int N, const Adder& grpadder) : 
		Adder(N),
		gap(1), // unused
		clg(N/grpadder.width()),
		xorsum(2,1), // unused
		cgenAnd(2,N/grpadder.width()),
		cgenOr(2,N/grpadder.width())
	{
		// compute group size and number of groups
		int gsz = grpadder.width();
		assert(gsz <= N);
		assert(N % gsz == 0);
		int Ngrp = N / gsz;

		// register submodules
		submodule(&clg);
		submodule(&cgenAnd);
		submodule(&cgenOr);

		// add final G,P outputs for hierarchical CLA
		addOutput("G",1);
		addOutput("P",1);

		// generate classname
		std::stringstream ss;
		ss << "LookaheadAdder<" << N << "," << grpadder.classname() << ">";
		classname_ = ss.str();

		// create and register adders
		for (int i = 0; i < Ngrp; i++)
			addAdder(grpadder.clone());

		// connect adders
		init2();
	}

	LookaheadAdder* clone() const
	{
		return new LookaheadAdder(*this);
	}
};

#endif // LOOKAHEADADDER_H_
