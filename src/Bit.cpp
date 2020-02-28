#include "Bit.h"
#include "Module.h"

Bit::Bit(bit_t b) : bit(b)
{
}

Bit::Bit(uint8_t b) : bit(b)
{
	assert(b == LOW || b == HIGH || b == UNDEF);
}

Bit::Bit(const Port& p)
{
	assert(p.module);
	assert(p.low == p.high);
	Bit b = p.module->getInput(p.low, Module::simTime());
	bit = b.bit;
}

Bit& Bit::operator=(const Port& p)
{
	assert(p.module);
	assert(p.low == p.high);

	return *this = p.module->getInput(p.low, Module::simTime());
}

std::ostream& operator<<(std::ostream &out, const Bit& b)
{
	switch (b.bit)
	{
	case LOW:   out << '0'; break;
	case HIGH:  out << '1'; break;
	case UNDEF: out << 'x'; break;
	default:   	assert(0); out << '?'; break;
	}
	return out;
}
