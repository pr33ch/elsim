#ifndef WIRE_H_
#define WIRE_H_

#include <set>
#include <utility>
#include <cassert>
#include "BitHistory.h"

class Module;

// Port is a (Module,i/o num) pair
typedef std::pair<Module*,int> PORT_T;
typedef std::set<PORT_T> PORTSET_T;
typedef PORTSET_T::const_iterator PORTITER_T;

// A Wire represents a 1-bit connection between a writer Module and reader Modules.
// The signal histories are saved in Wires.
// Wire objects are shared between the connected Modules.
class Wire
{
private:
	// one possible writer
	PORT_T writer_;
	// many possible readers
	PORTSET_T readers_;
	// history of Bit values
	BitHistory history_;
	// hard-wired to a constant value
	bool constant_;
	// reference count
	int refcnt_;

public:
	// constructor
	Wire() : writer_(NULL,-1), constant_(false), refcnt_(1)
	{
	}

	// add reader
	void addReader(Module* m, int inum)
	{
		assert(m);
		assert(inum >= 0 && inum < m->numInputs());
		readers_.insert(PORT_T(m,inum));
	}

	// add reader
	void addReader(const PORT_T& p)
	{
		assert(p.first);
		assert(p.second >= 0 && p.second < p.first->numInputs());
		readers_.insert(p);
	}

	// set writer
	void setWriter(Module* m, int onum)
	{
		assert(m);
		assert(onum >= 0 && onum < m->numOutputs());
		writer_.first = m;
		writer_.second = onum;
	}

	// set writer
	void setWriter(const PORT_T& p)
	{
		assert(p.first);
		assert(p.second >= 0 && p.second < p.first->numOutputs());
		writer_ = p;
	}

	// mark as constant (hard-wired) value
	void hold()
	{
		// sanity check: only set once, after setting one bit value
		assert(!constant_);
		assert(history_.size() <= 1);
		constant_ = true;
	}

	// access to readers and writer
	inline PORT_T& getWriter() { return writer_; }
	inline PORTITER_T beginReaders() const { return readers_.begin(); }
	inline PORTITER_T endReaders() const { return readers_.end(); }

	// check status
	inline bool hasWriter() const { return writer_.first != NULL; }
	inline int numReaders() const { return (int)readers_.size(); }

	// get/set bit values
	inline Bit get(delay_t T=DELAY_T_MAX) const { return history_.get(T); }
	inline const HPAIR_T& getPair(delay_t T=DELAY_T_MAX) const { return history_.getPair(T); }
	inline bool set(Bit b, delay_t T) { assert(!constant_); return history_.set(b,T); }

	// query history
	inline delay_t lastTime() const { return history_.lastTime(); }
	inline int numEdges() const { return history_.size(); }
	inline void clear() { if (!constant_) history_.clear(); }
	inline HITER_T histBegin() const { return history_.begin(); }
	inline HITER_T histEnd() const { return history_.end(); }

	// reference counting
	Wire* retain() { refcnt_++; return this; }
	void release() { if (!(--refcnt_)) delete this; }
};

#endif // WIRE_H_
