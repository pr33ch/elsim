#ifndef EDGEPROPERTIES_H_
#define EDGEPROPERTIES_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

struct VertexProps { std::string name; };
struct EdgeProps   { std::string name; };
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
class EdgeProperties
{
	public:
		// // constructor
		EdgeProperties(std::string source_port, std::string dest_port, int source_i, int source_j, int dest_i, int dest_j);

		// source and destination ports
		std::string source_port_;
		std::string dest_port_;

		// bit positions i through j for source port, inclusive
		int si_;
		int sj_;

		// bit positions i through j for dest port, inclusive
		int di_;
		int dj_;

		// time stamp at which signals arrive at dest
		int t_ = 0;

		// marked red in dot file if true
		bool critical_ = false;

		// returns the string representation of the edge properties
		std::string to_str();
};

#endif //EDGEPROPERTIES_H_