#include "EdgeProperties.h"
EdgeProperties::EdgeProperties(std::string source_port, std::string dest_port, 
	int source_i, int source_j, int dest_i, int dest_j):
	source_port_(source_port),
			dest_port_(dest_port),
			si_(source_i),
			sj_(source_j),
			di_(dest_i),
			dj_(dest_j)
{}