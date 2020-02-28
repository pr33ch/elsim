#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <map>
#include <set>
#include <utility>
#include <string>
#include <sstream>
#include <ostream>
#include <cassert>

#include "BitHistory.h"
#include "BitVector.h"

class Wire;		// Wire.h
struct Port;	// this file
class Module;	// this file
class SimQueue;	// SimQueue.h

// area estimate
typedef float area_t;

// energy/power estimate
typedef float energy_t;

// set of modules
typedef std::set<Module*> MSET_T;
typedef MSET_T::const_iterator MITER_T;

// The abstract base class for all modules.
// A module has an arbitrary number of 1-bit inputs and outputs.
// Ranges of inputs/outputs can have (string) names.
// Subclasses must define propagate() and delay().
// They may optionally define load(), area(), and energy().
class Module
{
private:
	// input/output Wires (lazily created)
	std::vector<Wire*> inWires_;
	std::vector<Wire*> outWires_;
	typedef std::vector<Wire*>::iterator WIREITER_T;

	// map from signal names to ports
	typedef std::map<std::string,Port> SIGNAMEMAP_T;
	typedef std::pair<std::string,Port> NAMEPAIR_T;
	typedef SIGNAMEMAP_T::const_iterator NAMEITER_T;
	SIGNAMEMAP_T nameMap_;

	// simulation state
	static SimQueue* simqueue;
	static delay_t simtime;

protected:
	// uniquely identifies class+configuration
	// must be set by subclasses
	std::string classname_;

	// add some number of input/output bits
	void addInputs(int N);
	void addOutputs(int N);

	// define an input/output signal as a named range of bits
	// the input/output bits must already exist
	void defineInput(const std::string& name, int high, int low);
	void defineInput(const std::string& name, int i);
	void defineOutput(const std::string& name, int high, int low);
	void defineOutput(const std::string& name, int i);
	
	// add N input/output bits and give them a name
	void addInput(const std::string& name, int N=1);
	void addOutput(const std::string& name, int N=1);

	// replace our input wire with given one, merging readers
	virtual void mergeInputWire(int inum, Wire* w);

	// set output bits from input bits
	// this is the where the simulation of functionality happens
	virtual void propagate() = 0;

	// get fanout load of 1 output bit
	delay_t fanout(int onum) const;
	delay_t fanout(const std::string& name) const;

	// check if input has an edge at the current sim time
	bool edge(int inum) const;
	bool edge(const std::string& name) const;

	bool posedge(int inum) const;
	bool posedge(const std::string& name) const; // must be 1-bit

	bool negedge(int inum) const;
	bool negedge(const std::string& name) const; // must be 1-bit

	// copy constructor (rarely used)
	Module(const Module& other);

public:
	// constructor
	Module();

	// destructor
	virtual ~Module();

	// get classname
	const std::string& classname() const;

#ifdef MOD_EXTRA
	// user-defined tag for printing/debugging
	int tag;
	// parent for hierarchical modules (irrelevent to simulations)
	Module* parent;
#endif

	// is this a SystemModule?
	virtual bool isSystem() const;

	// get area estimate
	virtual area_t area() const;

	// get delay from input to output
	virtual delay_t delay(int inum, int onum) = 0;

	// get the load factor for input `inum`
	// (expressed as a delay since it's directly added to delay)
	virtual delay_t load(int inum) const;

	// get energy to produce output
	virtual energy_t energy(int onum) const;

	// get static critical path (and optionally the input/output pair)
	delay_t criticalPath(int* inum_p=NULL, int* onum_p=NULL);

	// get power (average,peak)
	void simPowerStats(energy_t* avgpow, energy_t* peakpow) const;

	// get number of input bits
	int numInputs() const;

	// get number of output bits
	int numOutputs() const;

	// get input/output i at time T
	Bit getInput(int i, delay_t T=DELAY_T_MAX) const;
	Bit getOutput(int i, delay_t T=DELAY_T_MAX) const;

	// set input/output i at time T
	// (returns true if the value actually changed)
	bool setInput(int i, Bit b, delay_t T);
	bool setOutput(int i, Bit b, delay_t T);

	// lookup input/output name from number
	struct NamedPort; // defined at the end of this file
	NamedPort nameOfInput(int inum) const;
	NamedPort nameOfOutput(int onum) const;
	bool hasInput(const std::string& name) const;
	bool hasOutput(const std::string& name) const;

	// get last update time of a signal
	delay_t lastTime(const std::string& name) const;
	delay_t lastInputTime(int inum) const;
	delay_t lastOutputTime(int onum) const;
	delay_t lastInputTime() const; // over all inputs
	delay_t lastOutputTime() const; // over all outputs

	// check if named input/output signal is fully defined at a given time
	bool isDefined(const std::string& name, delay_t T=DELAY_T_MAX) const;

	// get/set named input/output signal as BitVector
	BitVector getVector(const std::string& name, delay_t T=DELAY_T_MAX) const;
	void setVector(const std::string& name, const BitVector& vec, delay_t T);

	// get/set named input/output signal as integer value
	// (all the bits must be defined, and the bits must be able to fit into value_t)
	value_t getValue(const std::string& name, delay_t T=DELAY_T_MAX) const;
	void setValue(const std::string& name, value_t value, delay_t T);

	// get Wire (likely to inspect history)
	const Wire* inputWire(int inum) const;
	const Wire* outputWire(int onum) const;
	// mark input/output as constant-valued
	void holdInput(int i);
	void holdOutput(int i);

	// get an input/output port for use with special operators
	Port IN(int i);
	Port IN(int high, int low);
	Port IN(const std::string& name);
	Port IN(const std::string& name, int i);
	Port IN(const std::string& name, int high, int low);

	Port OUT(int i);
	Port OUT(int high, int low);
	Port OUT(const std::string& name);
	Port OUT(const std::string& name, int i);
	Port OUT(const std::string& name, int high, int low);

	Port operator()(const std::string& name);
	Port operator()(const std::string& name, int i);
	Port operator()(const std::string& name, int high, int low);

	const Port& port(const std::string& name) const;

	// connect this module's output to another module's input
	// (usually called indirectly from a >> operator)
	virtual void connect(int onum, Module* other, int inum);

	// reset (clear history) all input/output wires
	virtual void reset();

	// simulate this module until all signals are stable
	void simulate();

	// STATIC SIMULATION METHODS
	// get current simulation time
	static delay_t simTime();
	// reset simulation state
	static void simReset();
	// simulate starting from the given set of Modules
	static void simStart(const MSET_T& roots, delay_t steps=DELAY_T_MAX);
	// simulate by giving root modules as args
	static void simStart(delay_t steps, int Nmod, Module* m,...);
	// continue an existing simulation
	static void simStep(delay_t steps=DELAY_T_MAX);

private:
	// Called by setOutput() when a wire value changed.
	// During simulation, this will add items to the sim queue.
	static void wireDidChange(Wire* w, delay_t T);
	// called by setOutput() to determine delay to a given output
	// (uses the input set from the sim queue)
	delay_t delayToOutput(int onum);
	// called by power() to determine power
	// (map from time to total output energy at that time)
	virtual void recordEnergies(std::map<delay_t,energy_t>& energyTable) const;

	// necessary so SystemModule has protected access to another Module
	friend class SystemModule;

	// operators need protected access
	friend void operator<=(const Port&, Bit);
	friend void operator<=(const Port&, const BitVector&);
	friend void operator<=(const Port&, value_t);
	friend void operator<=(const Port&, const Port&);
	friend void operator<<(const Port&, const Port&);
};

////////////////////////
// SEE SYSTEMMODULE.H
// connect output bit range to input bit range
// OR connect system input to module input
// OR connect output to system output
void operator>>(const Port&, const Port&);
// connect all outputs to input range
void operator>>(Module&    , const Port&);
// connect output range to system output
// OR set output from input of same module
void operator<<(const Port&, const Port&);
// connect all outputs to system output
void operator<<(const Port&, Module&    );
////////////////////////

// set input/output bits at current time
void operator<=(const Port&, Bit             );
void operator<=(const Port&, const BitVector&);
void operator<=(const Port&, value_t         );
void operator<=(const Port&, const Port&     ); // set output from input of same module

// print input/output bits to stream
std::ostream& operator<<(std::ostream&, const Port&);
// print module classname (and tag) to stream
std::ostream& operator<<(std::ostream&, const Module&);
// print input/output name and sub-bit to stream
std::ostream& operator<<(std::ostream&, const Module::NamedPort&);

// collection of info for printing purposes
struct Module::NamedPort
{
	std::string name;
	int port; // port < 0 means the named port has only 1 bit (i.e. don't print subscript 0)
	bool isOutput;
	// constructor for easy creation
	NamedPort(const std::string& n, int p, bool b) : name(n), port(p), isOutput(b) {}
};

// represents an input/output bit range of a module
struct Port
{
	Module* module;
	int low, high;
	bool isOutput;
	// constructor for easy creation
	Port(Module* m, int l, int h, bool o) : module(m), low(l), high(h), isOutput(o) {}
	// for referencing a particular bit
	inline Port operator()(int i) const
	{
		assert(i >= 0 && i < width());
		return Port(module, low+i, low+i, isOutput);
	}
	// for referencing a range of bits
	inline Port operator()(int r_high, int r_low) const
	{
		assert(r_low <= r_high);
		assert(r_low >= 0);
		assert(low + r_high <= high);
		return Port(module, low+r_low, low+r_high, isOutput);
	}
	// get bit-width
	inline int width() const
	{
		return high - low + 1;
	}
	// set wire(s) as constant
	void hold()
	{
		for (int i = low; i <= high; i++)
			isOutput ? module->holdOutput(i) : module->holdInput(i);
	}
};

#endif // MODULE_H_
