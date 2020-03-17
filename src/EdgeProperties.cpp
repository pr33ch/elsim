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

void EdgeProperties::normalize_bit_positions()
{
	if (!source_bit_positions_.empty() && !dest_bit_positions_.empty())
	{
		int sm = *std::min_element(source_bit_positions_.begin(), source_bit_positions_.end());
		for (auto& element : source_bit_positions_)
		{
			element -= sm;
		}

		int dm = *std::min_element(dest_bit_positions_.begin(), dest_bit_positions_.end());
		for (auto& element : dest_bit_positions_)
		{
			element -= dm;
		}
	}	
}

std::string EdgeProperties::to_str()
{	
	// normalize_bit_positions();
	std::ostringstream sbp; 
	if (!source_bit_positions_.empty()) 
	{ 
	    // Convert all but the last element to avoid a trailing "," 
	    std::copy(source_bit_positions_.begin(), source_bit_positions_.end()-1, 
	        std::ostream_iterator<int>(sbp, ", ")); 
	  
	    // Now add the last element with no delimiter 
	    sbp << source_bit_positions_.back(); 
	}

	std::ostringstream dbp; 
	if (!dest_bit_positions_.empty()) 
	{ 
	    // Convert all but the last element to avoid a trailing "," 
	    std::copy(dest_bit_positions_.begin(), dest_bit_positions_.end()-1, 
	        std::ostream_iterator<int>(dbp, ", ")); 
	  
	    // Now add the last element with no delimiter 
	    dbp << dest_bit_positions_.back(); 
	}

	std::string s = "SRC: " + source_port_ + "[" + sbp.str() + "]" + "\n";
	s += "DEST: " + dest_port_ + "[" + dbp.str() + "]" + "\n";

	if (t_ >= 0)
	{
		s += "T: " + std::to_string(t_);
	}
	else
	{
		s += "T: Cyclic";
	}

	return s;
}