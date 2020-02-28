#ifndef VCDWRITER_H_
#define VCDWRITER_H_

#include <ostream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "Module.h"
#include "Wire.h"

// Write a VCD file to a stream.
// A VCD file is a text file that lists signals and their values at the times when they changed.
// VCDs can be opened in waveform viewer applications.
class VCDWriter
{
private:
	// list of signal names
	std::vector<std::string> names_;
	// list of signal ports (i.e. Module and i/o numbers)
	std::vector<Port> mports_;
	// map from global time to set of inputs that changed at that time
	typedef std::set<int> INTSET_T;
	std::map<delay_t,INTSET_T> changelist_;

	// add all changes to `changelist_`
	void processNewSignal(std::string name, const Port& p)
	{
		int N = mports_.size();
		mports_.push_back(p);
		name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
		names_.push_back(name);

		for (int i = p.low; i <= p.high; i++)
		{
			const Wire* w = p.isOutput ? p.module->outputWire(i) : p.module->inputWire(i);
			if (!w) continue;
			for (HITER_T iter = w->histBegin(); iter != w->histEnd(); iter++)
			{
				delay_t T = (*iter).first;
				changelist_[T].insert(N);
			}
		}
	}

	// write signal value in VCD format
	void writeSignal(std::ostream& os, int i, delay_t T)
	{
		Port& p = mports_[i];
		if (p.width() > 1) os << 'b';
		for (int j = p.high; j >= p.low; j--)
		{
			Bit b = p.isOutput ? p.module->getOutput(j,T) : p.module->getInput(j,T);
			os << b;
		}
		if (p.width() > 1) os << ' ';
		os << char('!'+i) << '\n';
	}

public:
	// add a named signal to be written
	void addSignal(Module& m, const std::string& name)
	{
		std::stringstream ss;
		ss << m << "." << name;
		processNewSignal(ss.str(), m(name));
	}

	// add a 1-bit output to be written
	void addOutput(Module& m, int onum)
	{
		std::stringstream ss;
		ss << m << ".OUT" << onum;
		processNewSignal(ss.str(), m.OUT(onum));
	}

	// add a 1-bit input to be written
	void addInput(Module& m, int inum)
	{
		std::stringstream ss;
		ss << m << ".IN" << inum;
		processNewSignal(ss.str(), m.IN(inum));
	}

	// write the VCD file to the given stream
	void write(std::ostream& os)
	{
		int N = mports_.size();
		assert(N < ('~'-'!'+1)); // up to 94 different signals allowed

		// write header
		time_t t = time(NULL);
		os << "$date\n" << ctime(&t) << "$end\n";
		os << "$version\nDigital Simulator by Marco Vitanza\n$end\n";
		os << "$comment\n$end\n";
		os << "$timescale 1ps $end\n";
		os << "$scope module logic $end\n";
		for (int i = 0; i < N; i++)
			os << "$var wire " << mports_[i].width() << " " << char('!'+i) << " " << names_[i] << " $end\n";
		os << "$upscope $end\n";
		os << "$enddefinitions $end\n";

		// dump initial values
		os << "$dumpvars\n";
		for (int i = 0; i < N; i++)
			writeSignal(os, i, 0);
		os << "$end\n";

		// dump signal changes
		std::map<delay_t,INTSET_T>::const_iterator iter;
		for (iter = changelist_.begin(); iter != changelist_.end(); iter++)
		{
			delay_t T = (*iter).first;
			const INTSET_T& sigs = (*iter).second;
			os << '#' << T << '\n';
			INTSET_T::const_iterator i_iter;
			for (i_iter = sigs.begin(); i_iter != sigs.end(); i_iter++)
				writeSignal(os, *i_iter, T);
		}
	}
};

#endif // VCDWRITER_H_
