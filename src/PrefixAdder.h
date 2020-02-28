#ifndef PREFIXADDER_H_
#define PREFIXADDER_H_

#ifdef DEBUG
#include <iostream>
#endif

#include "Adder.h"
#include "MetaGate.h"
#include "Gate.h"
#include "GAP.h"
#include "INV.h"

// Kogge-Stone prefix adder
// Minimum number of levels, with small fanouts, but larger area than strictly needed.

class PrefixAdder : public Adder
{
private:
	GAP gap_;
	MetaGate<XOR> sumxor_;
	BUF buf_;
	typedef std::pair<int,int> RANGE_T;
	typedef std::map<RANGE_T,Port> GPMAP_T;
	typedef std::pair<RANGE_T,Port> GPPAIR_T;
	GPMAP_T Gmap_, Pmap_; // for inputs h to l, get port that represents G(h..l) or P(h..l)

	inline Port& G(int h, int l)
	{
		GPMAP_T::iterator iter = Gmap_.find(RANGE_T(h,l));
		assert(iter != Gmap_.end());
		return (*iter).second;
	}

	inline Port& P(int h, int l)
	{
		GPMAP_T::iterator iter = Pmap_.find(RANGE_T(h,l));
		assert(iter != Pmap_.end());
		return (*iter).second;
	}

	inline void setGmap(int h, int l, const Port& port)
	{
		assert(h >= l);
		Gmap_.insert(GPPAIR_T(RANGE_T(h,l),port));
	}

	inline void setPmap(int h, int l, const Port& port)
	{
		assert(h >= l);
		Pmap_.insert(GPPAIR_T(RANGE_T(h,l),port));
	}

	void init()
	{
		int N = width();
		submodules(3, &gap_, &sumxor_, &buf_);

		std::stringstream ss;
		ss << "PrefixAdder<" << N << ">";
		classname_ = ss.str();

		// setup initial g and p
		for (int i = 0; i < N; i++)
		{
			IN("X",i) >> gap_("X",i);
			IN("Y",i) >> gap_("Y",i);
			setGmap(i, i, gap_("g",i));
			setPmap(i, i, gap_("a",i));
		}

		// carry-in goes to buffer
		IN("Ci") >> buf_.IN(0);
		setGmap(-1,-1, buf_.OUT(0));
		setPmap(-1,-1, buf_.OUT(0)); // never actually used

		// create and connect levels of GP modules
		for (int lvl = 1; ; lvl++)
		{
			int delta = 1 << (lvl - 1);
			int ncell = N - (delta - 1);
			if (ncell < 1) break;
#ifdef DEBUG
std::cout << "level=" << lvl << " delta=" << delta << " ncell=" << ncell << std::endl;
#endif
			for (int j = 0; j < ncell; j++)
			{
				int h = N - 1 - j;
				int h1 = h - delta;
				int l1 = h1 + 1;
				int l = l1 - delta;
				if (l < -1) l = -1;
				assert(h1 >= -1);
#ifdef DEBUG
std::cout << "level=" << lvl << " GP(" << h << "," << l << ") has inputs "
          << "GP(" << h << "," << l1 << ") and GP(" << h1 << "," << l << ")" << std::endl;
#endif
				GP* module = new GP();
				submodule(module);
				GP& gp = *module;

				G(h,l1) >> gp("g");
				P(h,l1) >> gp("p");

				G(h1,l) >> gp("gPrev");
				P(h1,l) >> gp("pPrev");

				setGmap(h, l, gp("G"));
				setPmap(h, l, gp("P"));
			}
		}

		// setup final sum bits
		for (int i = 0; i < N; i++)
		{
			gap_("p",i) >> sumxor_("0",i);

			G(i-1,-1) >> sumxor_("1",i);

			OUT("S",i) << sumxor_("Z",i);
		}

		// final carry-out
		OUT("Co") << G(N-1,-1);
	}

public:
	PrefixAdder(int N) :
		 Adder(N)
		,gap_(N)
		,sumxor_(2,N)
		,buf_(1)
	{
		init();
	}

	PrefixAdder(const PrefixAdder& other) :
		 Adder(other.width())
		,gap_(other.width())
		,sumxor_(2,other.width())
		,buf_(1)
	{
		init();
	}

	PrefixAdder* clone() const
	{
		return new PrefixAdder(*this);
	}

	~PrefixAdder()
	{
		// free submodules (GPs)
		for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
		{
			Module* m = *iter;
			if (m->classname() == "GP") delete m;
		}
	}
};

#endif // PREFIXADDER_H_
