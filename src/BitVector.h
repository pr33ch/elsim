#ifndef BITVECTOR_H_
#define BITVECTOR_H_

#include <vector>
#include "Bit.h"

// defined in Module.h
struct Port;

// array of Bits
class BitVector
{
private:
	std::vector<Bit> bits_;

public:
	// create vector with specified width (bit values are UNDEF)
	BitVector(int width);
	// create width-1 vector with given bit
	BitVector(Bit b);
	// create vector with specified width from int value
	BitVector(int width, value_t val);
	// create vector with string (allowed chars are '0','1','x')
	BitVector(const std::string& s);
	// read vector from Module input port
	BitVector(const Port& p);
	BitVector& operator=(const Port& p);

	// get random vector (0s/1s only)
	static BitVector random(int N);

	// get width
	int width() const { return bits_.size(); }
	// get bit i
	Bit get(int i = 0) const { return bits_.at(i); }
	// set bit i
	void set(int i, const Bit& b) { bits_.at(i) = b; }
	// set bit 0
	void set(const Bit& b) { set(0, b); }

	// equality operators (using Bit comparators)
	bool operator==(const BitVector& other) const;
	bool operator!=(const BitVector& other) const;

	// bitwise operators that produce a new BitVector
	BitVector operator&(const BitVector& other) const;
	BitVector operator|(const BitVector& other) const;
	BitVector operator^(const BitVector& other) const;
	BitVector operator~() const;

	// bitwise methods that change current BitVector
	BitVector&  AND(const BitVector& other);
	BitVector& NAND(const BitVector& other);
	BitVector&   OR(const BitVector& other);
	BitVector&  NOR(const BitVector& other);
	BitVector&  XOR(const BitVector& other);
	BitVector& XNOR(const BitVector& other);
	BitVector&  NOT();
};

// write vector to stream
std::ostream& operator<<(std::ostream& out, const BitVector& vec);

#endif // BITVECTOR_H_
