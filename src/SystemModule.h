#ifndef SYSTEMMODULE_H_
#define SYSTEMMODULE_H_

#include "Module.h"
#include "EdgeProperties.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

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
	
	// vector of edges that define the circuit's critical path
	std::vector<edge_t> critical_path_edges_;
	delay_t max_delay_ = 0; // only used for critical path delay analysis of graph representation of circuit 

	// map edges connecting root nodes to destination module port numbers
	std::map<edge_t, std::vector<int>> system_input_destination_ports_of_;

	// map module to vertex descriptor
	std::map<Module*, vertex_t> vertex_descriptor_of_;

	// map vertex descriptor to module
	std::map<vertex_t, Module*> module_of_descriptor_;

	// an edge in the graph is uniquely defined by source and destination modules + ports
	// map <source module, source port name, destination module, destination port name>to edge property
	std::map<std::tuple<Module*, std::string, Module*, std::string>, EdgeProperties*> edge_properties_of_components_;
	std::map<edge_t, EdgeProperties*> edge_properties_of_descriptor_;

	// connect vertices of system module submodules
	void connect_submodule_vertices();

	// update signal arrival times at each edge via dfs graph traversal
	void update_graph_timestamps();

	// label the connections between modules in the graph visualization
	void label_edges();

	// set all the timestamps in the graphical representation of the circuit
	void timestamps_dfs(vertex_t node, std::map<vertex_t, bool> path, std::vector<edge_t> &edge_path, int t, std::vector<int> inums);

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

	// export a dot file that visualizes the circuit via computational graph
	void visualize(); // TO-DO: argument should be a custom path?

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
