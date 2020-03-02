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
		EdgeProperties(std::string source_port, std::string dest_port);

		// source and destination ports
		std::string source_port_;
		std::string dest_port_;

		// source_bit_positions_[i] is connected to dest_bit_positions[i]
		std::vector<int> source_bit_positions_;
		std::vector<int> dest_bit_positions_;

		// time stamp at which signals arrive at dest
		int t_ = 0;

		// marked red in dot file if true
		bool critical_ = false;

		// returns the string representation of the edge properties
		std::string to_str();

		// define a new bit position connection from source_port_ to dest_port. Modifies the bit position vectors
		void define_bit_position_connection(int source, int dest);
};

#endif //EDGEPROPERTIES_H_