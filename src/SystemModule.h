#ifndef SYSTEMMODULE_H_
#define SYSTEMMODULE_H_

#include "Module.h"

// A SystemModule is a Module which contains other Modules.
// All cumulative delay/area/load/energy calculations are done here.
// Subclasses only need to create and connect submodules.
class SystemModule : public Module
{
protected:
	// registered submodules (not freed)
	MSET_T submodules_;

private:
	// lazily-created delay tables
	typedef std::pair<int,int> IOPAIR_T;
	typedef std::map<IOPAIR_T,delay_t> DELAYTBL_T;
	typedef DELAYTBL_T::iterator DTBLITER_T;
	typedef std::map<std::string,DELAYTBL_T*> DELAYMAP_T;
	typedef DELAYMAP_T::iterator DMAPITER_T;
	// cache of delays for all used module types
	static DELAYMAP_T delayTables;
	// helper function that generates table
	DELAYTBL_T* generateDelayTable();

	// helper function adds energies for submodules
	void recordEnergies(std::map<delay_t,energy_t>& energyTable) const;

public:
	// overridden from Module
	bool isSystem() const { return true; }

	// overridden from Module
	void connect(int onum, Module* other, int inum);

	// overridden from Module
	void reset();

	// overridden from Module
	delay_t delay(int inum, int onum);

	// overridden from Module
	delay_t load(int inum) const;

	// overridden from Module
	area_t area() const;

	// overridden from Module
	energy_t energy(int onum) const;

protected:
	// overridden from Module
	void propagate();

	// overridden from Module
	void mergeInputWire(int inum, Wire* w);

	// register submodule(s) for delay/area computations (and set `parent`)
	void submodule(Module* m);
	void submodules(int Nmod, Module* m, ...);

	// connect system input/output with sub-module
	void connectSystemInput(int i, Module* m, int inum);
	void connectSystemOutput(int i, Module* m, int onum);

	// operators need protected access
	friend void operator>>(const Port&, const Port&);
	friend void operator<<(const Port&, const Port&);
	friend void clearDelayTables();
};

#endif // SYSTEMMODULE_H_
