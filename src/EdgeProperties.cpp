#include "EdgeProperties.h"
EdgeProperties::EdgeProperties(std::string source_port, std::string dest_port, 
	int source_i, int source_j, int dest_i, int dest_j):
	source_port_(source_port),
			dest_port_(dest_port),
{}

void EdgeProperties::define_bit_position_connections(int source, int dest)
{
	source_bit_positions.push_back(source);
	dest_bit_positions.push_back(dest);
}