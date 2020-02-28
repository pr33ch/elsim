#ifndef SIMQUEUE_H_
#define SIMQUEUE_H_

#include <set>

class Module;

// Simulation queue used by Module methods.
// Essentially a priority queue of (Module,time) pairs ordered by time.
class SimQueue
{
public:
	// The objects stored in the queue
	struct QItem
	{
		Module* module;        // the module whose inputs were changed
		delay_t T;             // the time at which the inputs changed
		std::set<int> inputs;  // the input numbers that changed
		QItem(Module* m, delay_t t) : module(m), T(t) {}
		bool operator<(const QItem& other) const
		{
			if (T == other.T)
				return module < other.module;
			return T < other.T;
		}
	};
	typedef std::set<QItem> QUEUE_T;
	typedef QUEUE_T::iterator QITER_T;

private:
	// the actual queue
	QUEUE_T items_;

public:
	// is the queue empty?
	bool empty() const { return items_.empty(); }

	// get the next item
	const QItem* top() const { return &(*(items_.begin())); }

	// push an item
	void push(Module* m, delay_t T, int inum)
	{
		QItem item(m,T);
		QITER_T iter = items_.find(item);
		if (iter == items_.end())
		{
			item.inputs.insert(inum);
			items_.insert(item);
		}
		else
		{
			QItem& curitem = const_cast<QItem&>(*iter);
			curitem.inputs.insert(inum);
		}
	}

	// remove the next item
	void pop() { items_.erase(items_.begin()); }
};

#endif // SIMQUEUE_H_
