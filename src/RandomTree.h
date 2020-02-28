#ifndef RANDOMTREE_H_
#define RANDOMTREE_H_

#include <cstdlib>
#include <time.h>
#include "SystemModule.h"
#include "Gate.h"

// Create a random tree of standard 2-input gates.
// The tree is like an upside-down binary tree.
// For an L-level tree, there are (2^L - 1) total gates
// and 2^L top-level inputs.

class RandomTree : public SystemModule
{
private:
	Module* randomGate()
	{
		Module* m = NULL;
		switch (random() % 6)
		{
		case 0: m = new AND();  break;
		case 1: m = new NAND(); break;
		case 2: m = new OR();   break;
		case 3: m = new NOR();  break;
		case 4: m = new XOR();  break;
		case 5: m = new XNOR(); break;
		}
		return m;
	}

	inline Bit randomBit()
	{
		return Bit(random() % 2);
	}

public:
	// The number of levels in the tree.
	// For example, if levels==2:
	//
	//      | |      | |
	//     [gate]   [gate]
	//        |       |
	//        +--+ +--+
	//           | |
	//          [gate]
	//            |
	//
	RandomTree(int levels)
	{
		assert(levels > 0 && levels <= 24);
		time_t t = time(NULL);
		srandom(t);
		int N = 1 << (levels - 1);

		addInputs(N*2);
		addOutputs(1);

		std::stringstream ss;
		ss << "RandomTree<" << levels << ",seed=" << t << ">";
		classname_ = ss.str();

		std::vector<Module*> vecA, vecB;
		std::vector<Module*> *tmp, *curlvl = &vecA, *prevlvl = &vecB;

		for (int L = 0; L < levels; L++, N/=2)
		{
			assert(N > 0);
			for (int i = 0; i < N; i++)
			{
				Module* gate = randomGate();
				submodule(gate);
				curlvl->push_back(gate);

				if (L)
				{
					*(*prevlvl)[2*i  ] >> gate->IN(0);
					*(*prevlvl)[2*i+1] >> gate->IN(1);
				}
				else
				{
					IN(2*i  ) >> gate->IN(0);
					IN(2*i+1) >> gate->IN(1);

					IN(2*i  ) <= randomBit();	// set global inputs to random bits
					IN(2*i+1) <= randomBit();
				}
			}

			// swap gate arrays
			tmp = prevlvl;
			prevlvl = curlvl;
			curlvl = tmp;
			curlvl->clear();
		}
		assert(N == 0);

		OUT(0) << *(*prevlvl)[0];
	}
};

#endif // RANDOMTREE_H_
