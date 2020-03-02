#include "EdgeProperties.h"
EdgeProperties::EdgeProperties(std::string source_port, std::string dest_port):
	source_port_(source_port),
	dest_port_(dest_port)
{}

void EdgeProperties::define_bit_position_connection(int source, int dest)
{
	source_bit_positions_.push_back(source);
	dest_bit_positions_.push_back(dest);
}