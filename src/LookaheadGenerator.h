#ifndef LOOKAHEADGENERATOR_H_
#define LOOKAHEADGENERATOR_H_

#include "SystemModule.h"
#include "Gate.h"

// Carry-lookahead Generator
// Inputs are N `generate` and `propagate` signals.
// Outputs are (N-1) "cumulative" `generate` and `propagate` signals.
// N-1 because G0 = g0 and P0 = p0 so nothing needs to be computed.

class LookaheadGenerator : public SystemModule
{
public:
	LookaheadGenerator(int N)
	{
		addInput("g", N);
		addInput("p", N);

		addOutput("G", N-1);
		addOutput("P", N-1);

		std::stringstream ss;
		ss << "LookaheadGenerator<" << N << ">";
		classname_ = ss.str();

		// generate N-1 P bits
		// each is a single AND of fanin 2,3,4,...N
		for (int n = 2; n <= N; n++)
		{
			AND* gate = new AND(n);
			AND& a = *gate;
			submodule(gate);
			for (int i = 0; i < n; i++)
				IN("p",i) >> a.IN(i);
			OUT("P",n-2) << a;
		}

		// generate N-1 Gs : each is a single OR of widths 2,3,4,...N
		//                   and 1,2,3,...N-1 ANDs, of widths (2) (2,3) (2,3,4) ... (2,3,...N)
		for (int n = 2; n <= N; n++)
		{
			// group n (computing G[n-2])
			OR* gate = new OR(n);
			OR& orG = *gate;
			submodule(gate);

			// group has n-1 AND gates
			for (int i = 2; i <= n; i++)
			{
				// AND gate i (i inputs)
				AND* gate = new AND(i);
				AND& a = *gate;
				submodule(gate);
				// first input is g[n-i]
				IN("g",n-i+0) >> a.IN(0);
				// remaining inputs are p[n-i+k]
				for (int k = 1; k < i; k++)
					IN("p",n-i+k) >> a.IN(k);
				// AND output goes to OR
				a >> orG.IN(i-2);
			}

			// last input to OR gate is g[n-1]
			IN("g",n-1) >> orG.IN(n-1);
			// OR gate produces `G` output
			OUT("G",n-2) << orG;
		}
	}

	~LookaheadGenerator()
	{
		// we dynamically allocated submodules and saved them in `submodules_`
		// so we must free them (SystemModule will not do it)
		for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
			delete (*iter);
	}
};

#endif // LOOKAHEADGENERATOR_H_
