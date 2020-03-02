#ifdef DEBUG
#include <iostream>
#endif
#include <cstdarg>
#include <queue>
#include "SystemModule.h"
#include "Wire.h"
#include "param.h"

void SystemModule::visualize()
{
	connect_submodule_vertices();
	write_graphviz(std::cout, g_,
	            make_label_writer(get(&VertexProps::name, g_)),
	            make_label_writer(get(&EdgeProps::name, g_)));
}

void SystemModule::connect_submodule_vertices()
{
 
	// // Iterate over the map using c++11 range based for loop
	// for (std::pair<Module*, vertex_t> element : vertex_descriptor_of_) {
	// 	// Accessing KEY from element
	// 	std::cout << "Module Address: "<< element.first << " maps to: ";
	// 	// Accessing VALUE from element.
	// 	std::cout << "Vertex Descriptor: "<< element.second << std::endl;
	// }
	// std::cout << "Making module connections... " << std::endl;
	for (auto src_module : submodules_)
	{
		for(int onum = 0; onum < src_module->numOutputs(); onum++) // output ports
		{
			if (src_module->local_connections_.count(onum) != 0)
			{
				for (auto dest : src_module->local_connections_[onum])
				{
					int src_port_num = onum;
					std::string src_port_name = src_module->nameOfOutput(src_port_num).name;
					// std::cout << "Source module: "<< src_module << " port " << src_port_name << " connects with ";

					Module* dest_module = dest.first;
					int dest_port_num = dest.second;
					std::string dest_port_name = dest.first->nameOfInput(dest_port_num).name;
					// std::cout << "Dest module: " << dest_module << " port " << dest_port_name << std::endl;

					std::tuple<Module*, std::string, Module*, std::string> key = std::make_tuple(src_module, src_port_name, dest_module, dest_port_name);

					if (edge_properties_of_components_.count(key) == 0)
					{
						auto e = add_edge(vertex_descriptor_of_[src_module], vertex_descriptor_of_[dest_module], {"unset properties"}, g_).first;

						EdgeProperties* ep = new EdgeProperties(src_port_name, dest_port_name);
						edge_properties_of_descriptor_[e] = ep;
						edge_properties_of_components_[key] = ep;
					}
					else
					{
						edge_properties_of_components_[key]->define_bit_position_connection(src_port_num, dest_port_num);
					}
				}
			}
		}
	}
}

void SystemModule::reset()
{
	// reset all submodules
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
		(*iter)->reset();
}

void clearDelayTables()
{
	SystemModule::delayTables.clear();
}

// should only be called if `this` is root module at the start of a simulation
void SystemModule::propagate()
{
#ifdef DEBUG
std::cout << "system propagate " << *this << " T=" << simtime << std::endl;
#endif
	assert(simtime == 0);
	MSET_T modules;

	// gather all submodules connected to inputs
	for (int i = 0; i < numInputs(); i++)
		if (Wire* w = inWires_[i])
			for (PORTITER_T iter = w->beginReaders(); iter != w->endReaders(); iter++)
				modules.insert((*iter).first);

	// propagate all submodules
	for (MITER_T iter = modules.begin(); iter != modules.end(); iter++)
		(*iter)->propagate();
}

////////////////////////////////////////////////////////////
// Module connections
////////////////////////////////////////////////////////////

// NOTE: a SystemModule should never be added as reader/writer of Wire

void SystemModule::connect(int onum, Module* other, int inum)
{
#ifdef DEBUG
std::cout << "connect " << *this << " output " << onum << " to " << *other << " input " << inum << std::endl;
#endif
	assert(onum >= 0 && onum < numOutputs());
	assert(other);
	assert(inum >= 0 && inum < other->numInputs());

	// connect our output to other module
	Wire* w1 = outWires_[onum];
	Wire* w2 = other->inWires_[inum];
	if (!w1) return; // if SystemModule output wire doesn't exist, then no submodule is connected to it

	if (w2 == NULL)
	{
		// setup wire with other module
		other->inWires_[inum] = w1->retain();
		assert(!other->isSystem());
		w1->addReader(other, inum);
	}
	else
	{
		// both wires already exist
		assert(!w2->hasWriter());
		other->mergeInputWire(inum, w1);
	}

	local_connections_[onum].push_back(PORT_T(other, inum));
	// for (std::pair<Module*, vertex_t> element : vertex_descriptor_of_) {
	// 	// Accessing KEY from element
	// 	std::cout << "Module Address: "<< element.first << " maps to: ";
	// 	// Accessing VALUE from element.
	// 	std::cout << "Vertex Descriptor: "<< element.second << std::endl;
	// }

	// std::cout << "connect " << this << " output " << onum << " to " << other << " input " << inum << std::endl;
	// std::string src_port_name = this->nameOfOutput(onum).name;

	// std::string dest_port_name = other->nameOfInput(inum).name;

	// std::tuple<Module*, std::string, Module*, std::string> key = std::make_tuple(this, src_port_name, other, dest_port_name);
	// // std::cout << "count of key: " << edge_properties_of_components_.count(key) << std::endl;
	// if (edge_properties_of_components_.count(key) == 0)
	// {
	// 	std::cout << "new edge: " <<vertex_descriptor_of_[this] << "->" << vertex_descriptor_of_[other] << std::endl;
	// 	auto e = add_edge(vertex_descriptor_of_[this], vertex_descriptor_of_[other], {"unset properties"}, g_).first;

	// 	EdgeProperties* ep = new EdgeProperties(src_port_name, dest_port_name);
	// 	edge_properties_of_descriptor_[e] = ep;
	// 	edge_properties_of_components_[key] = ep;
	// }
	// else
	// {
	// 	edge_properties_of_components_[key]->define_bit_position_connection(onum, inum);
	// }
}

void SystemModule::mergeInputWire(int inum, Wire* w)
{
	assert(w);
	assert(inum >= 0 && inum < numInputs());
	assert(inWires_[inum]);

	// Problem: we want to call merge on direct submodules, but wire readers are only `leaf` modules.
	// Solution: keep track of direct submodules and recursively merge on them.
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
	{
		Module* m = (*iter);
		int Nin = m->numInputs();
		for (int i = 0; i < Nin; i++)
			if (m->inWires_[i] == inWires_[inum])
				m->mergeInputWire(i, w);
	}

	// replace in our own wire array
	inWires_[inum]->release();
	inWires_[inum] = w->retain();
}

void SystemModule::connectSystemInput(int i, Module* m, int inum)
{
	assert(i >= 0 && i < numInputs());
	assert(m);
	assert(inum >= 0 && inum < m->numInputs());
#ifdef DEBUG
std::cout << "system " << *this << " input" << i << " " << nameOfInput(i) << " goes to "
          << *m << " input" << inum << " " << m->nameOfInput(inum) << std::endl;
#endif
	Wire* w1 = inWires_[i];
	Wire* w2 = m->inWires_[inum];

	if (w1 == NULL)
	{
		if (w2 == NULL)
		{
			// neither wire exists yet
			inWires_[i] = w1 = new Wire();
			m->inWires_[inum] = w1->retain();
			assert(!m->isSystem());
			w1->addReader(m, inum);
		}
		else
		{
			// other wire exists but ours doesn't
			inWires_[i] = w2->retain();
		}
	}
	else if (w2 == NULL)
	{
		// our wire exists but other doesn't
		m->inWires_[inum] = w1->retain();
		assert(!m->isSystem());
		w1->addReader(m, inum);
	}
	else
	{
		// both wires already exist
		// remove other wire, merge into ours
#ifdef DEBUG
if (w2->hasWriter()) 
std::cout << "w2 has writer! " << *(w2->getWriter().first) << std::endl;
#endif
		assert(!w2->hasWriter());
		m->mergeInputWire(inum, w1);
	}

	// update graph representation of circuit
	std::string label = "SRC: " + nameOfInput(i).name + "[" + std::to_string(i) + "]\n" + "DEST: " + m->nameOfInput(inum).name + "[" + std::to_string(inum) + "]\n" + "T: 0";
	add_edge(iovertex_descriptor_of_[nameOfInput(i).name], vertex_descriptor_of_[m], {label}, g_);
}

// can only be called once for a given i!
void SystemModule::connectSystemOutput(int i, Module* m, int onum)
{
	assert(i >= 0 && i < numOutputs());
	assert(m);
	assert(onum >= 0 && onum < m->numOutputs());
#ifdef DEBUG
std::cout << "system " << *this << " output" << i << " " << nameOfOutput(i) << " comes from "
          << *m << " output" << onum << " " << m->nameOfOutput(onum) << std::endl;
#endif

	// connect system output i with module output
	Wire* w1 = outWires_[i];
	Wire* w2 = m->outWires_[onum];
	assert(w1 == NULL);

	if (w2 == NULL)
	{
		// neither wire exists
		outWires_[i] = w1 = new Wire();
		m->outWires_[onum] = w1->retain();
		assert(!m->isSystem());
		w1->setWriter(m, onum);
	}
	else
	{
		// other wire exists but ours doesn't
		outWires_[i] = w2->retain();
	}

	//update the graph representation of the circuit
	std::string label = "SRC: " + m->nameOfOutput(onum).name + "[" + std::to_string(onum) + "]\n" + "DEST: " + nameOfOutput(i).name + "[" + std::to_string(i) + "]\n" + "T: 0";
	add_edge(vertex_descriptor_of_[m], iovertex_descriptor_of_[nameOfOutput(i).name], {label}, g_);
}

void operator<<(const Port& p1, const Port& p2)
{
	assert(p1.module);

	int N = p1.width();
	assert(N == p2.width());

	assert(p1.isOutput);

	if (!p2.isOutput)
	{
		// set output from input of same module
		p1 <= p2;
		return;
	}

	// connect system output with module output
	assert(p1.module->isSystem());
	SystemModule* sys = static_cast<SystemModule*>(p1.module);	

	for (int i = 0; i < N; i++)
		sys->connectSystemOutput(p1.low+i, p2.module, p2.low+i);
}

void operator<<(const Port& p, Module& m)
{
	// system output << all module outputs
	int N = m.numOutputs();
	p << m.OUT(N-1,0);
}

void operator>>(const Port& p1, const Port& p2)
{
	// connect module output to module input
	// OR connect system input to module input
	// OR connect module output to system output
	assert(p1.module);
	assert(p2.module);
	assert(p1.low <= p1.high);
	assert(p2.low <= p2.high);

	int N = p1.width();
	assert(N == p2.width());
	assert(N > 0);

	if (!p1.isOutput)
	{
		// system input >> module input
		assert(p1.module->isSystem());
		assert(!p2.isOutput);
		SystemModule* sys = static_cast<SystemModule*>(p1.module);
		for (int i = 0; i < N; i++)
			sys->connectSystemInput(p1.low+i, p2.module, p2.low+i);
	}
	else
	{
		if (p2.isOutput)
		{
			// module output >> system output
			assert(p2.module->isSystem());
			SystemModule* sys = static_cast<SystemModule*>(p2.module);
			for (int i = 0; i < N; i++)
				sys->connectSystemOutput(p2.low+i, p1.module, p1.low+i);
		}
		else
		{
			// module output >> module input
			for (int i = 0; i < N; i++)
				p1.module->connect(p1.low+i, p2.module, p2.low+i);
		}
	}
}

void operator>>(Module& m1, const Port& p2)
{
	// all module outputs >> module inputs
	int N = m1.numOutputs();
	m1.OUT(N-1,0) >> p2;
}

////////////////////////////////////////////////////////////
// Automatic delay/area/energy computations
////////////////////////////////////////////////////////////

void SystemModule::submodule(Module* m)
{
	assert(m);
	submodules_.insert(m);
	vertex_t u = add_vertex(g_);
	g_[u].name = m->classname_;
	vertex_descriptor_of_[m] = u;
#ifdef MOD_EXTRA
	assert(!m->parent);
	m->parent = this;
#endif
}

void SystemModule::submodules(int Nmod, Module* m, ...)
{
	va_list vlist;
	va_start(vlist, m);

	assert(Nmod > 0);
	submodule(m);

	for (int i = 1; i < Nmod; i++)
	{
		Module* m = va_arg(vlist, Module*);
		submodule(m);
	}

	va_end(vlist);
}

// definition for static variable
SystemModule::DELAYMAP_T SystemModule::delayTables;

// generate and return a table for this module
SystemModule::DELAYTBL_T* SystemModule::generateDelayTable()
{
	// classname must be defined
	assert(classname_.length() > 0);
#ifdef DEBUG
std::cout << "generateDelayTable " << classname_ << std::endl;
std::cout << "we have " << submodules_.size() << " submodules" << std::endl;
#endif

	DELAYTBL_T *dtbl = new DELAYTBL_T();

	// map is delay from system input to Wire
	typedef std::pair<int,Wire*> IWPAIR_T;
	std::map<IWPAIR_T,delay_t> delaytowire;
	// queue is wires to process
	std::queue<IWPAIR_T> wirequeue;

	// create map from submodule input wires to submodules
	typedef std::pair<Wire*,PORT_T> WMPAIR_T;
	typedef std::multimap<Wire*,PORT_T> WMMAP_T;
	typedef WMMAP_T::iterator WMITER_T;
	WMMAP_T wiretosubmod;
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
	{
		Module* m = (*iter);
		int N = m->numInputs();
		for (int i = 0; i < N; i++)
		{
			Wire* w = m->inWires_[i]; // dont know for sure if this is a system input wire
			if (w) wiretosubmod.insert(WMPAIR_T(w,PORT_T(m,i)));
		}
	}

#ifdef DEBUG
std::cout << "    " << wiretosubmod.size() << " mappings from (possible) input wire to submodule port" << std::endl;
#endif

	// start with input wires
	int Nin = numInputs();
	for (int inum = 0; inum < Nin; inum++)
	{
		Wire* w = inWires_[inum];
		if (!w) continue;

		// if system input is not connected to anything
		if (wiretosubmod.count(w) < 1) continue;

		// record delay to input wire (0)
		IWPAIR_T iwpair(inum,w);
		delaytowire[iwpair] = 0;
		wirequeue.push(iwpair);
	}

#ifdef DEBUG
std::cout << "    added " << wirequeue.size() << " input wires to queue" << std::endl;
#endif

	// process as DAG where wires are nodes
	while (!wirequeue.empty())
	{
		// get next (sysinum,wire) pair
		const IWPAIR_T& p = wirequeue.front();
		int sysinum = p.first;
		Wire* w = p.second;
		delay_t TtoW = delaytowire[p];
		wirequeue.pop();

#ifdef DEBUG
std::cout << "    input " << sysinum << " can reach wire " << w << " (" << w->numReaders() 
          << " readers) in T=" << TtoW << std::endl;
#endif

		// get all submodules that read from this wire
		std::pair<WMITER_T,WMITER_T> range = wiretosubmod.equal_range(w);
		for (WMITER_T iter = range.first; iter != range.second; iter++)
		{
			const PORT_T& p = (*iter).second;
			Module* m = p.first;
			int inum = p.second;
			assert(submodules_.count(m) == 1);
#ifdef DEBUG
std::cout << "        processing delays through submodule " << *m << " input " << inum << std::endl;
#endif
			// for each output of submodule
			int Nout = m->numOutputs();
			for (int onum = 0; onum < Nout; onum++)
			{
				// get delay to this output
				delay_t dT = m->delay(inum,onum);
				if (dT <= 0) continue;
#if USE_FANOUT_DELAY
				dT += m->fanout(onum);
#endif
				// add output wire to be processed
				Wire* ow = m->outWires_[onum];
				if (!ow) continue;
				IWPAIR_T iwpair(sysinum,ow);
				wirequeue.push(iwpair);
				// wire `w` is reachable from system input `sysinum`
				// therefore wire `ow` is also reachable
				// input `sysinum` can reach `ow` in time `dT + delaytowire[inum,w]`
				// set delaytowire[inum,ow] to that time (if its bigger than existing)
				delay_t& TtoOW = delaytowire[iwpair]; // (item created with value 0 if doesn't exist yet)
				delay_t T = TtoW + dT;
				if (T > TtoOW) TtoOW = T;
			}
		}
	}

	// build map from output Wire -> onum for fast checks
	std::map<Wire*,int> outwiremap;
	int Nout = numOutputs();
	for (int onum = 0; onum < Nout; onum++)
	{
		Wire* w = outWires_[onum];
		if (w) outwiremap[w] = onum;
	}

#ifdef DEBUG
std::cout << "    " << outwiremap.size() << " wires in outwiremap" << std::endl;
std::cout << "    " << delaytowire.size() << " (i,w) pairs in delaytowire" << std::endl;
#endif

	// add all items in `delaytowire` to `dtbl`
	std::map<IWPAIR_T,delay_t>::iterator iter;
	for (iter = delaytowire.begin(); iter != delaytowire.end(); iter++)
	{
		// get wire
		Wire* w = (*iter).first.second;
		// if wire is a system output, get its number
		std::map<Wire*,int>::iterator w_iter = outwiremap.find(w);
		if (w_iter == outwiremap.end()) continue;
		int onum = (*w_iter).second;
		// get system input, and total delay
		int inum = (*iter).first.first;
		delay_t T = (*iter).second;
#if USE_FANOUT_DELAY
		// fanout was added each time in main loop above
		// but delay() never should include (final) fanout
		PORT_T& outport = w->getWriter();
		T -= (outport.first)->fanout(outport.second);
#endif
		// add to delay table
		(*dtbl)[IOPAIR_T(inum,onum)] = T;
	}
#ifdef DEBUG
std::cout << "    " << dtbl->size() << " (I,O) pairs in delay table" << std::endl;
#endif
	// save delay table in global cache, and return it
	delayTables[classname_] = dtbl;
	return dtbl;
}

// only used for computing critical path
// never called during simulations
delay_t SystemModule::delay(int inum, int onum)
{
	assert(inum >= 0 && inum < numInputs());
	assert(onum >= 0 && onum < numOutputs());
#ifdef DEBUG
std::cout << "SystemModule::delay(" << inum << "," << onum << ") " << *this << std::endl;
#endif
	// get delay table for particular module type
	assert(classname_.length() > 0);
	DMAPITER_T iter_m = delayTables.find(classname_);
	DELAYTBL_T* dtbl = (iter_m == delayTables.end()) ? generateDelayTable() : (*iter_m).second;

	// look up I->O delay
	DTBLITER_T iter = dtbl->find(IOPAIR_T(inum,onum));
	if (iter == dtbl->end())
		return DELAY_T_MIN;
	return (*iter).second;
}

// sum the area of all submodules
area_t SystemModule::area() const
{
	area_t A = 0;
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
		A += (*iter)->area();
	return A;
}

// sum the input load of all connected submodules
delay_t SystemModule::load(int inum) const
{
	assert(inum >= 0 && inum < numInputs());
	Wire* w = inWires_[inum];
	if (!w) return 0;
	delay_t l = 0;

	for (PORTITER_T iter = w->beginReaders(); iter != w->endReaders(); iter++)
	{
		Module* m = (*iter).first;
		int inum = (*iter).second;
		l += m->load(inum);
	}

	return l;
}

// sum energy of all paths used to compute output `onum`
energy_t SystemModule::energy(int onum) const
{
	// probably won't be used that often...
	assert(onum >= 0 && onum < numOutputs());
	assert(outWires_[onum]);

	// create map from Wire to submodule output port
	std::map<Wire*,PORT_T> wiretomodout;
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
	{
		Module* m = (*iter);
		int Nout = m->numOutputs();
		for (int i = 0; i < Nout; i++)
			if (m->outWires_[i])
				wiretomodout[ m->outWires_[i] ] = PORT_T(m,i);
	}

	// add up energy for all submodules from `onum` to system inputs
	energy_t E = 0;
	std::set<Wire*> seenwires;
	std::queue<Wire*> wirequeue;
	seenwires.insert(outWires_[onum]);
	wirequeue.push(outWires_[onum]);

	while (!wirequeue.empty())
	{
		PORT_T p = wiretomodout[wirequeue.front()];
		wirequeue.pop();
		if (!p.first) continue;
		E += p.first->energy(p.second);

		int Nin = p.first->numInputs();
		for (int i = 0; i < Nin; i++)
		{
			Wire* w = p.first->inWires_[i];
			if (w && seenwires.count(w) < 1)
			{
				seenwires.insert(w);
				wirequeue.push(w);
			}
		}
	}

	return E;
}

void SystemModule::recordEnergies(std::map<delay_t,energy_t>& energyTable) const
{
	for (MITER_T iter = submodules_.begin(); iter != submodules_.end(); iter++)
		(*iter)->recordEnergies(energyTable);
}
