#ifndef BITHISTORY_H_
#define BITHISTORY_H_

#include <set>
#include <utility>
#include <climits>
#include "Bit.h"

// time type, used in simulation
typedef int delay_t;
#define DELAY_T_MAX INT_MAX
#define DELAY_T_MIN INT_MIN

// history element is a (time,value) pair
typedef std::pair<delay_t,Bit> HPAIR_T;
struct HPAIRcmp
{
	// less-than comparison on HPAIR_T (use times)
	inline bool operator()(const HPAIR_T& p1, const HPAIR_T& p2) const
	{
		return p1.first < p2.first;
	}
};
typedef std::set<HPAIR_T,HPAIRcmp> HSET_T;
typedef HSET_T::const_iterator HITER_T;

// BitHistory is a record of changes (time,value) pairs for some Bit signal
class BitHistory
{
private:
	HSET_T history_;

public:
	// Get the (time,value) pair for time T.
	// Note that the actual time returned can be < T.
	const HPAIR_T& getPair(delay_t T) const
	{
		static const Bit bitundef;
		static const HPAIR_T undef(DELAY_T_MIN,bitundef);

		// iter points to some time >= T
		HITER_T iter = history_.lower_bound(HPAIR_T(T,bitundef));

		// T was past the biggest recorded time
		if (iter == history_.end())
		{
			// no values present
			if (history_.size() < 1)
				return undef;
			// return latest bit
			return *(history_.rbegin());
		}

		// found exact T
		if ((*iter).first == T)
			return *iter;

		// we found first element and it's > T
		if (iter == history_.begin())
			return undef;

		// return previous bit (time < T)
		iter--;
		return *iter;
	}

	// Get the value at time T
	Bit get(delay_t T) const
	{
		return getPair(T).second;
	}

	// Set the value at time T
	bool set(Bit b, delay_t T)
	{
		// get the value at time T
		const HPAIR_T& p = getPair(T);

		// if new value is different
		if (p.second != b)
		{
			// if record was actually at time T, remove it
			if (p.first == T)
				history_.erase(p);
			// add new value
			history_.insert(HPAIR_T(T,b));
		}
		// value did not change
		else return false;

		// bit was changed for time T
		return true;
	}

	// Does the history have a value at exactly time T?
	bool hasTime(delay_t T) const
	{
		return getPair(T).first == T;
	}

	// Last (latest) time in the history
	delay_t lastTime() const
	{
		return (history_.size() > 0) ? (*(history_.rbegin())).first : DELAY_T_MIN;
	}

	// Number of entries in the history
	int size() const { return (int)history_.size(); }

	// Clear the history
	void clear() { history_.clear(); }

	// Get iterators to the beginning and end of the history
	HITER_T begin() const { return history_.begin(); }
	HITER_T end() const { return history_.end(); }
};

#endif // BITHISTORY_H_
