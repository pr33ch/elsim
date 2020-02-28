#ifdef DEBUG
#include <iostream>
#endif
#include <cstdarg>
#include "Module.h"
#include "Wire.h"
#include "SimQueue.h"
#include "param.h"

////////////////////////////////////////////////////////////
// Constructor/destructor/misc
////////////////////////////////////////////////////////////

// defafult constructor
Module::Module()
#ifdef MOD_EXTRA
	: tag(-1), parent(NULL)
#endif
{
}

// copy constructor
Module::Module(const Module& other) :
	 nameMap_(other.nameMap_)
	,classname_(other.classname_)
#ifdef MOD_EXTRA
	,tag(-1)
	,parent(NULL)
#endif
{
	int Nin = other.numInputs();
	int Nout = other.numOutputs();
#ifdef DEBUG
std::cout << "copy constructing " << classname_ << " " << &other
          << " " << Nin << "in " << Nout << "out" << std::endl;
#endif

	inWires_.resize(Nin);
	outWires_.resize(Nout);

	// other should be `free module` not connected to anything
	for (int i = 0; i < Nin; i++)
		assert(!other.inWires_[i]);

	for (int i = 0; i < Nout; i++)
		assert(!other.outWires_[i]);

	// set `module` pointer in `nameMap_` Ports to `this`
	for (SIGNAMEMAP_T::iterator iter = nameMap_.begin(); iter != nameMap_.end(); iter++)
		(*iter).second.module = this;
}

Module::~Module()
{
	WIREITER_T iter;
	// release input wires
	for (iter = inWires_.begin(); iter != inWires_.end(); iter++)
	{
		Wire* w = *iter;
		if (w) w->release();
	}
	// release output wires
	for (iter = outWires_.begin(); iter != outWires_.end(); iter++)
	{
		Wire* w = *iter;
		if (w) w->release();
	}
}

// get classname
const std::string& Module::classname() const
{
	assert(classname_.length() > 0);
	return classname_;
}

bool Module::isSystem() const
{
	return false;
}

area_t Module::area() const
{
	return 0; // default implementation -- subclasses should override
}

delay_t Module::load(int inum) const
{
	return 0; // default implementation -- subclasses should override
}

energy_t Module::energy(int onum) const
{
	return 0; // default implemetnation -- subclasses should override
}

////////////////////////////////////////////////////////////
// Set up input/output signals
////////////////////////////////////////////////////////////

void Module::addInputs(int N)
{
	assert(N > 0);
	int curN = inWires_.size();
	inWires_.resize(curN + N);
}

void Module::addOutputs(int N)
{
	assert(N > 0);
	int curN = outWires_.size();
	outWires_.resize(curN + N);
}

void Module::defineInput(const std::string& name, int high, int low)
{
	assert(low >= 0 && high >= low);
	assert(high < numInputs());
	assert(name.length() > 0);
	assert(nameMap_.count(name) == 0);

	nameMap_.insert(NAMEPAIR_T(name, Port(this,low,high,false)));
}

void Module::defineInput(const std::string& name, int i)
{
	defineInput(name, i, i);
}

void Module::defineOutput(const std::string& name, int high, int low)
{
	assert(high >= 0 && low >= 0 && high >= low);
	assert(high < numOutputs());
	assert(name.length() > 0);
	assert(nameMap_.count(name) == 0);

	nameMap_.insert(NAMEPAIR_T(name, Port(this,low,high,true)));
}

void Module::defineOutput(const std::string& name, int i)
{
	defineOutput(name, i, i);
}


void Module::addInput(const std::string& name, int N)
{
	int Nin = numInputs();
	addInputs(N);
	defineInput(name, Nin+N-1, Nin);
}

void Module::addOutput(const std::string& name, int N)
{
	int Nout = numOutputs();
	addOutputs(N);
	defineOutput(name, Nout+N-1, Nout);
}

Module::NamedPort Module::nameOfInput(int inum) const
{
	assert(inum >= 0 && inum < numInputs());

	// lookup this input number in the name map
	for (NAMEITER_T iter = nameMap_.begin(); iter != nameMap_.end(); iter++)
	{
		const Port& p = (*iter).second;
		if (!p.isOutput && inum >= p.low && inum <= p.high)
			return NamedPort( (*iter).first , (p.low==p.high) ? -1 : (inum-p.low) , false );
	}

	return NamedPort("",inum,false);
}

Module::NamedPort Module::nameOfOutput(int onum) const
{
	assert(onum >= 0 && onum < numOutputs());

	// lookup this output number in the name map
	for (NAMEITER_T iter = nameMap_.begin(); iter != nameMap_.end(); iter++)
	{
		const Port& p = (*iter).second;
		if (p.isOutput && onum >= p.low && onum <= p.high)
			return NamedPort( (*iter).first , (p.low==p.high) ? -1 : (onum-p.low) , true );
	}

	return NamedPort("",onum,true);
}

bool Module::hasInput(const std::string& name) const
{
	NAMEITER_T iter = nameMap_.find(name);
	if (iter == nameMap_.end()) return false;
	return !(*iter).second.isOutput;
}

bool Module::hasOutput(const std::string& name) const
{
	NAMEITER_T iter = nameMap_.find(name);
	if (iter == nameMap_.end()) return false;
	return (*iter).second.isOutput;
}

////////////////////////////////////////////////////////////
// Get and set input/output signals
////////////////////////////////////////////////////////////

Bit Module::getInput(int i, delay_t T) const
{
	assert(i >= 0 && i < numInputs());
	Wire* w = inWires_[i];
	return w ? w->get(T) : Bit(UNDEF);
}

Bit Module::getOutput(int i, delay_t T) const
{
	assert(i >= 0 && i < numOutputs());
	Wire* w = outWires_[i];
	return w ? w->get(T) : Bit(UNDEF);
}

bool Module::setInput(int i, Bit b, delay_t T)
{
	assert(i >= 0 && i < numInputs());
	Wire* w = inWires_[i];

	if (w == NULL)
	{
		inWires_[i] = w = new Wire();
		if (!isSystem()) w->addReader(this, i);
	}

	return w->set(b,T);
}

bool Module::setOutput(int i, Bit b, delay_t T)
{
	assert(i >= 0 && i < numOutputs());
	Wire* w = outWires_[i];

	if (w == NULL)
	{
		outWires_[i] = w = new Wire();
		if (!isSystem()) w->setWriter(this, i);
	}

	// IMPORTANT FOR SIMULATION
	// during simulation we must call wireDidChange() callback
	bool changed = w->set(b,T);
	if (changed && simqueue)
		wireDidChange(w,T);
	return changed;
}

BitVector Module::getVector(const std::string& name, delay_t T) const
{
	const Port& p = port(name);
	int N = p.width();
	BitVector vec(N);

	for (int i = 0; i < N; i++)
	{
		Bit b = p.isOutput ? getOutput(i+p.low,T) : getInput(i+p.low,T);
		vec.set(i,b); 
	}

	return vec;
}

void Module::setVector(const std::string& name, const BitVector& vec, delay_t T)
{
	const Port& p = port(name);
	int N = p.width();
	assert(vec.width() == N);

	for (int i = 0; i < N; i++)
	{
		Bit b = vec.get(i);
		p.isOutput ? setOutput(p.low+i,b,T) : setInput(p.low+i,b,T);
	}
}

value_t Module::getValue(const std::string& name, delay_t T) const
{
	const Port& p = port(name);
	assert(p.width() <= VALUE_T_BITS);
	value_t value = 0;

	for (int i = p.high; i >= p.low; i--, value <<= 1)
	{
		Bit b = p.isOutput ? getOutput(i,T) : getInput(i,T);
		assert(b.isDefined());
		value |= (value_t)b.bit;
	}

	return value;
}

void Module::setValue(const std::string& name, value_t value, delay_t T)
{
	const Port& p = port(name);
	assert(p.width() <= VALUE_T_BITS);

	for (int i = p.low; i <= p.high; i++, value >>= 1)
	{
		Bit b(value & 1);
		p.isOutput ? setOutput(i,b,T) : setInput(i,b,T);
	}
}

// reset all wires
void Module::reset()
{
	int N = numInputs();
	for (int i = 0; i < N; i++)
		inWires_[i]->clear();

	N = numOutputs();
	for (int i = 0; i < N; i++)
		outWires_[i]->clear();
}

const Wire* Module::inputWire(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	return inWires_[inum];
}

const Wire* Module::outputWire(int onum) const
{
	assert(onum >= 0 && onum < numOutputs());
	return outWires_[onum];
}

void Module::holdInput(int i)
{
	assert(i >= 0 && i < numInputs());
	inWires_[i]->hold();
}

void Module::holdOutput(int i)
{
	assert(i >= 0 && i < numOutputs());
	outWires_[i]->hold();
}

////////////////////////////////////////////////////////////
// Get signal metadata
////////////////////////////////////////////////////////////

int Module::numInputs() const
{
	return (int)inWires_.size();
}

int Module::numOutputs() const
{
	return (int)outWires_.size();
}

delay_t Module::lastTime(const std::string& name) const
{
	const Port& p = port(name);
	delay_t T = DELAY_T_MIN;

	// get last time over all bits
	for (int i = p.low; i <= p.high; i++)
	{
		delay_t Tbit = p.isOutput ? lastOutputTime(i) : lastInputTime(i);
		if (Tbit > T) T = Tbit;
	}

	return T;
}

delay_t Module::lastInputTime(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	Wire* w = inWires_[inum];
	return w ? w->lastTime() : DELAY_T_MIN;
}

delay_t Module::lastOutputTime(int onum) const
{
	assert(onum >= 0 && onum < numOutputs());
	Wire* w = outWires_[onum];
	return w ? w->lastTime() : DELAY_T_MIN;
}

delay_t Module::lastInputTime() const
{
	delay_t T, Tmax = DELAY_T_MIN;
	int N = numInputs();

	// get last time over all inputs
	for (int i = 0; i < N; i++)
	{
		Wire* w = inWires_[i];
		if (w && (T = w->lastTime()) > Tmax)
			Tmax = T;
	}

	return Tmax;
}

delay_t Module::lastOutputTime() const
{
	delay_t T, Tmax = DELAY_T_MIN;
	int N = numOutputs();

	// get last time over all outputs
	for (int i = 0; i < N; i++)
	{
		Wire* w = outWires_[i];
		if (w && (T = w->lastTime()) > Tmax)
			Tmax = T;
	}

	return Tmax;
}

bool Module::isDefined(const std::string& name, delay_t T) const
{
	const Port& p = port(name);

	// make sure all bits are defined
	for (int i = p.low; i <= p.high; i++)
	{
		if ( (  p.isOutput && !getOutput(i,T).isDefined() ) || 
		     ( !p.isOutput && !getInput(i,T).isDefined()  )    )
			return false;
	}

	return true;
}

bool Module::edge(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	Wire* w = inWires_[inum];
	if (w)
	{
		const HPAIR_T& p = w->getPair(Module::simtime);
		if (p.first == Module::simtime)
			return true;
	}
	return false;
}

bool Module::edge(const std::string& name) const
{
	const Port& p = port(name);
	assert(!p.isOutput);

	for (int i = p.low; i <= p.high; i++)
		if (edge(i)) return true;

	return false;
}

bool Module::posedge(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	Wire* w = inWires_[inum];
	if (w)
	{
		const HPAIR_T& p = w->getPair(Module::simtime);
		if (p.first == Module::simtime && p.second == HIGH)
			return true;
	}
	return false;
}

bool Module::posedge(const std::string& name) const
{
	const Port& p = port(name);
	assert(!p.isOutput);
	assert(p.low == p.high);

	return posedge(p.low);
}

bool Module::negedge(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	Wire* w = inWires_[inum];
	if (w)
	{
		const HPAIR_T& p = w->getPair(Module::simtime);
		if (p.first == Module::simtime && p.second == LOW)
			return true;
	}
	return false;
}

bool Module::negedge(const std::string& name) const
{
	const Port& p = port(name);
	assert(!p.isOutput);
	assert(p.low == p.high);

	return negedge(p.low);
}

delay_t Module::fanout(int onum) const
{
	assert(onum >= 0 && onum < numOutputs());
	Wire* w = outWires_[onum];
	if (!w) return 0;
	delay_t f = 0;

	// sum all load factors for connected inputs
	for (PORTITER_T iter = w->beginReaders(); iter != w->endReaders(); iter++)
	{
		Module* m = (*iter).first;
		int inum = (*iter).second;
		f += m->load(inum);
	}

	return f;
}

delay_t Module::fanout(const std::string& name) const
{
	const Port& p = port(name);
	assert(p.isOutput);
	assert(p.low == p.high);

	return fanout(p.low);
}

delay_t Module::criticalPath(int* inum_p, int* onum_p)
{
	delay_t Tmax = DELAY_T_MIN;
	int i_max = -1, o_max = -1;

	int Nin = numInputs();
	int Nout = numOutputs();

	// find the max delay for all (input,output) pairs
	for (int inum = 0; inum < Nin; inum++)
	for (int onum = 0; onum < Nout; onum++)
	{
		delay_t T = delay(inum, onum);
#if USE_FANOUT_DELAY
		// not that important, since this is likely a top-level module
		// with nothing connected to its outputs
		T += fanout(onum);
#endif
		if (T > Tmax)
		{
			Tmax = T;
			i_max = inum;
			o_max = onum;
		}
	}

	if (inum_p) *inum_p = i_max;
	if (onum_p) *onum_p = o_max;
	return Tmax;
}

void Module::simPowerStats(energy_t* avgpow, energy_t* peakpow) const
{
	std::map<delay_t,energy_t> energyTable;
	std::map<delay_t,energy_t>::const_iterator iter;

	// record energies at each time step
	recordEnergies(energyTable);

	energy_t emax = 0;
	energy_t etotal = 0;
	delay_t dT = (*energyTable.rbegin()).first;

	// average power is total energy / total time
	// peak power is max energy for 1 timestep
	for (iter = energyTable.begin(); iter != energyTable.end(); iter++)
	{
		energy_t E = (*iter).second;
		etotal += E;
		if (E > emax) emax = E;
	}

	if (avgpow) *avgpow = etotal / dT;
	if (peakpow) *peakpow = emax;
}

void Module::recordEnergies(std::map<delay_t,energy_t>& energyTable) const
{
	int N = numOutputs();
	// for each output
	for (int i = 0; i < N; i++)
	{
		assert(outWires_[i]);
		// calculate energy to produce output
		energy_t E = energy(i);
		// add energy to table for each time that the output changed
		for (HITER_T iter = outWires_[i]->histBegin(); iter != outWires_[i]->histEnd(); iter++)
		{
			delay_t T = (*iter).first;
			energyTable[T] += E;
		}
	}
}

////////////////////////////////////////////////////////////
// Connections between modules
////////////////////////////////////////////////////////////

void Module::mergeInputWire(int inum, Wire* w)
{
	assert(w);
	assert(inum >= 0 && inum < numInputs());
	assert(inWires_[inum]);

#ifdef DEBUG
std::cout << "mergeInputWire " << *this << " input " << inum << " wire " << inWires_[inum] 
          << ", with wire " << w << std::endl;
#endif

	w->addReader(this, inum);

	inWires_[inum]->release();
	inWires_[inum] = w->retain();
}

void Module::connect(int onum, Module* other, int inum)
{
	assert(onum >= 0 && onum < numOutputs());
	assert(other);
	assert(inum >= 0 && inum < other->numInputs());

	Wire* w1 = outWires_[onum];
	Wire* w2 = other->inWires_[inum];

	if (w1 == NULL)
	{
		if (w2 == NULL)
		{
			// neither wire exists so create one
			outWires_[onum] = w1 = new Wire();
			other->inWires_[inum] = w1->retain();
			// set ourself as writer, add other as reader
			assert(!isSystem());
			assert(!other->isSystem());
			w1->setWriter(this, onum);
			w1->addReader(other, inum);
		}
		else
		{
			// our wire doesn't exist, other one does
			assert(!w2->hasWriter());
			assert(!isSystem());
			outWires_[onum] = w2->retain();
			w2->setWriter(this, onum);
		}
	}
	else if (w2 == NULL)
	{
		// our wire exists, other one doesn't
		assert(!other->isSystem());
		other->inWires_[inum] = w1->retain();
		w1->addReader(other, inum);
	}
	else
	{
		// both wires already exist
		// other wire should be "alone" only connected to other
		assert(!w2->hasWriter());
		assert(w2->numReaders() == 1);
		other->mergeInputWire(inum, w1);
	}
}

////////////////////////////////////////////////////////////
// Simulation methods
////////////////////////////////////////////////////////////

// global simulation state
SimQueue*      Module::simqueue = NULL;
delay_t         Module::simtime = 0;
const SimQueue::QItem* simqitem = NULL;

// instance method
delay_t Module::delayToOutput(int onum)
{
	assert(onum >= 0 && onum < numOutputs());
	delay_t maxdelay = DELAY_T_MIN;

	if (simqitem)
	{
		// currently simulating this module...
		// get max of delay(i,onum) for each i in simqitem->inputs
		assert(simqitem->module == this);
		assert(simqitem->inputs.size() > 0);

		for (std::set<int>::iterator iter = simqitem->inputs.begin(); iter != simqitem->inputs.end(); iter++)
		{
			delay_t T = delay(*iter, onum);
			if (T > maxdelay) maxdelay = T;
		}
	}
	else
	{
		// called propagate() at start of simulation...
		// get max of delay(i,onum) for each i that was set for T=0
		int Nin = numInputs();
		for (int i = 0; i < Nin; i++)
		{
			if (!edge(i)) continue;
			delay_t T = delay(i, onum);
			if (T > maxdelay) maxdelay = T;
		}
		// if no input had edge, then no input was set, so simulation will do nothing
	}

#if USE_FANOUT_DELAY
	// include fanout load in delay calculation
	if (maxdelay >= 0)
		maxdelay += fanout(onum);
#endif

	// We call this method before we know if an output changed.
	// We can't know if an output will change before we actually find out WHEN it's being set
	// (using the return value from this method).
	// A value < 0 means the inputs that changed do not affect the given output.
	return maxdelay;
}

// static method
void Module::wireDidChange(Wire* w, delay_t T)
{
	assert(w);
	assert(T >= 0);
	assert(simqueue);

#ifdef DEBUG
PORT_T& wp = w->getWriter();
std::cout << "wireDidChange T=" << T << 
             " writer=" << *wp.first << "[" << wp.second << "] " << 
             w->numReaders() << "readers" << std::endl;
#endif

	// add all readers of this Wire to sim queue
	for (PORTITER_T iter = w->beginReaders(); iter != w->endReaders(); iter++)
	{
		Module* m = (*iter).first;
		int inum = (*iter).second;
		simqueue->push(m, T, inum);
	}
}

// static method
delay_t Module::simTime()
{
	return Module::simtime;
}

// instance method
void Module::simulate()
{
	// wrapper for static methods
	MSET_T roots;
	roots.insert(this);
	Module::simStart(roots, DELAY_T_MAX);
	Module::simReset();
}

// static method
void Module::simStart(delay_t steps, int Nmod, Module* m, ...)
{
	va_list vlist;
	va_start(vlist, m);

	assert(Nmod > 0);
	MSET_T roots;
	assert(m);
	roots.insert(m);

	// add all given modules into the roots set
	for (int i = 1; i < Nmod; i++)
	{
		Module* m = va_arg(vlist, Module*);
		assert(m);
		roots.insert(m);
	}

	va_end(vlist);
	Module::simStart(roots, steps);
}

// static method
void Module::simStep(delay_t steps)
{
	// continue an already-running simulation
	assert(simqueue);
	assert(steps > 0);
	delay_t Tend = (DELAY_T_MAX - simtime <= steps) ? DELAY_T_MAX : (simtime + steps);

	// MAIN SIMULATION LOOP
	// propagate until there are no more wire updates, or end time is reached
	while (!simqueue->empty())
	{
		// get the next item in the queue
		simqitem = simqueue->top();
		// stop if its time is past our ending time
		if (simqitem->T >= Tend) break;
#ifdef DEBUG
std::cout << "propagating: " << *simqitem->module << ", T=" << simqitem->T << std::endl;
#endif
		// set the current time
		simtime = simqitem->T;
		// propagate the inputs to outputs
		simqitem->module->propagate();
		// remove the item from the queue
		simqueue->pop();
	}
}

// static method
void Module::simStart(const MSET_T& roots, delay_t steps)
{
	// setup global simulation state
	simReset();
	assert(!roots.empty());
	simqueue = new SimQueue();

	// propagate from root modules
	for (MITER_T iter = roots.begin(); iter != roots.end(); iter++)
		(*iter)->propagate();

	// simulate for requested number of time steps
	simStep(steps);
}

// see SystemModule.cpp
void clearDelayTables();

// static method
void Module::simReset()
{
	if (simqueue != NULL)
	{
		delete simqueue;
		simqueue = NULL;
	}
	simtime = 0;
	simqitem = NULL;
	clearDelayTables();
}

////////////////////////////////////////////////////////////
// Methods to get Ports
////////////////////////////////////////////////////////////

Port Module::IN(int i)
{
	assert(i >= 0 && i < numInputs());
	return Port(this, i, i, false);
}

Port Module::IN(int high, int low)
{
	assert(low >= 0 && low <= high && high < numOutputs());
	return Port(this, low, high, false);
}

Port Module::IN(const std::string& name)
{
	const Port& p = port(name);
	assert(!p.isOutput);
	return p;
}

Port Module::IN(const std::string& name, int i)
{
	const Port& p = port(name);
	assert(!p.isOutput);
	return p(i);
}

Port Module::IN(const std::string& name, int high, int low)
{
	const Port& p = port(name);
	assert(!p.isOutput);
	return p(high,low);
}

Port Module::OUT(int i)
{
	assert(i >= 0 && i < numOutputs());
	return Port(this, i, i, true);
}

Port Module::OUT(int high, int low)
{
	assert(low >= 0 && low <= high && high < numOutputs());
	return Port(this, low, high, true);
}

Port Module::OUT(const std::string& name)
{
	const Port& p = port(name);
	assert(p.isOutput);
	return p;
}

Port Module::OUT(const std::string& name, int i)
{
	const Port& p = port(name);
	assert(p.isOutput);
	return p(i);
}

Port Module::OUT(const std::string& name, int high, int low)
{
	const Port& p = port(name);
	assert(p.isOutput);
	return p(high,low);
}

const Port& Module::port(const std::string& name) const
{
	NAMEITER_T iter = nameMap_.find(name);
	assert(iter != nameMap_.end());
	const Port& p = (*iter).second;
	assert(p.module == this);
	return p;
}

Port Module::operator()(const std::string& name)
{
	return port(name);
}

Port Module::operator()(const std::string& name, int i)
{
	return port(name)(i);
}

Port Module::operator()(const std::string& name, int high, int low)
{
	return port(name)(high,low);
}

////////////////////////////////////////////////////////////
// Methods to set input/output bits
////////////////////////////////////////////////////////////

void operator<=(const Port& p, Bit b)
{
	// set a single input/output bit
	assert(p.module);
	assert(p.low == p.high);
	if (p.isOutput)
	{
		delay_t dT = p.module->delayToOutput(p.low);
		if (dT <= 0) return;
		p.module->setOutput(p.low, b, Module::simtime+dT);
	}
	else p.module->setInput(p.low, b, Module::simtime);
}

void operator<=(const Port& p, const BitVector& vec)
{
	// set a range of input/output bits
	assert(p.module);
	int N = p.width();
	assert(N == vec.width());

	for (int i = 0; i < N; i++)
	{
		if (p.isOutput)
		{
			delay_t dT = p.module->delayToOutput(p.low+i);
			if (dT <= 0) continue;
			p.module->setOutput(p.low+i, vec.get(i), Module::simtime+dT);
		}
		else p.module->setInput(p.low+i, vec.get(i), Module::simtime);
	}
}

void operator<=(const Port& p, value_t val)
{
	// set a range of output bits
	assert(p.module);
	assert(p.low <= p.high);
	assert(p.width() <= VALUE_T_BITS);

	for (int i = p.low; i <= p.high; i++, val>>=1)
	{
		if (p.isOutput)
		{
			delay_t dT = p.module->delayToOutput(i);
			if (dT <= 0) continue;
			p.module->setOutput(i, Bit(val & 1), Module::simtime+dT);
		}
		else p.module->setInput(i, Bit(val & 1), Module::simtime);
	}
}

void operator<=(const Port& po, const Port& pi)
{
	// set output bits from input bits
	Module* m = po.module;
	assert(m);
	assert(m == pi.module);
	int N = po.width();
	assert(N == pi.width());
	assert(po.isOutput);
	assert(!pi.isOutput);

	for (int i = 0; i < N; i++)
	{
		delay_t dT = m->delayToOutput(po.low+i);
		if (dT <= 0) continue;
		Bit b = m->getInput(pi.low+i, Module::simtime);
		m->setOutput(po.low+i, b, Module::simtime+dT);
	}
}

////////////////////////////////////////////////////////////
// Methods to print names/values to a stream
////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& out, const Port& p)
{
	// print current input/output bits to stream
	assert(p.module);
	assert(p.low <= p.high);

	if (p.isOutput)
		for (int i = p.high; i >= p.low; i--)
			out << p.module->getOutput(i);
	else
		for (int i = p.high; i >= p.low; i--)
			out << p.module->getInput(i);

	return out;
}

std::ostream& operator<<(std::ostream& out, const Module& m)
{
	// print module name/tag to stream
	out << m.classname();
#ifdef MOD_EXTRA
	if (m.tag >= 0)
		out << "#" << m.tag;
#endif
	return out;
}

std::ostream& operator<<(std::ostream& out, const Module::NamedPort& p)
{
	// print nice port name
	if (p.name.length() < 1)
		out << (p.isOutput ? "OUT" : "IN");
	else
		out << p.name;

	if (p.port >= 0)
		out << "[" << p.port << "]";

	return out;
}
