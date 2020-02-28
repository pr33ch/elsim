#ifndef METAGATE_H_
#define METAGATE_H_

#include "SystemModule.h"

// MetaGate is a gate with some number of arbitrary-width inputs
// The template parameter must be a Module which has one output bit
// and a constructor taking an int parameter which is the number of
// 1-bit inputs.

template<class GATE>
class MetaGate : public SystemModule
{
public:
	// Constructor parameters:
	//   Number of inputs.
	//   Width (in bits) of each input.
	MetaGate(int Nin, int width)
	{
		assert(Nin > 1);
		assert(width > 0);

		std::stringstream ss;
		ss << GATE(Nin).classname() << "<" << Nin << "x" << width << "bit>";
		classname_ = ss.str();

		for (int i = 0; i < Nin; i++)
		{
			std::stringstream ss;
			ss << i;
			addInput(ss.str(), width);
		}

		addOutput("Z",width);

		// create and connect gates
		for (int i = 0; i < width; i++)
		{
			Module* gate = new GATE(Nin);
			submodule(gate);
			for (int j = 0; j < Nin; j++)
				IN(j*width+i) >> gate->IN(j);
			OUT("Z",i) << (*gate);
		}
	}

	~MetaGate()
	{
		// we dynamically allocated submodules and saved them in `submodules_`
		// so we must free them (SystemModule will not do it)
		for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
			delete (*iter);
	}
};

#endif // METAGATE_H_
