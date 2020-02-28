#ifndef BIT_H_
#define BIT_H_

#include <ostream>
#include <cassert>
#include <cstdlib>
#include <stdint.h>

// bit vector as an integer
typedef uint32_t value_t;
#define VALUE_T_BITS (int(8*sizeof(value_t)))

// bit value constants
enum bit_t {LOW=0,HIGH=1,UNDEF=0xFF};
#define INVERT(b) (((b)==UNDEF) ? UNDEF : !(b))

// defined in Module.h
struct Port;

// abstract bit class that uses 3-state logic
struct Bit
{
	// the bit value
	uint8_t bit;

	// create bit from int/enum
	Bit(bit_t b=UNDEF);
	Bit(uint8_t b);
	// read bit from Module input port
	Bit(const Port& p);
	Bit& operator=(const Port& p);

	// get random bit (HIGH/LOW)
	static inline Bit random()
	{
		return Bit((::random() & 1) ? HIGH : LOW);
	}

	// check if bit is defined
	inline bool isDefined() const
	{
		return bit != UNDEF;
	}

	// standard bitwise operators that return a new Bit

	inline Bit operator&(const Bit& other) const
	{
		if (bit == LOW || other.bit == LOW)
			return Bit(LOW);
		if (bit == UNDEF || other.bit == UNDEF)
			return Bit(UNDEF);
		return Bit(HIGH);
	}

	inline Bit operator|(const Bit& other) const
	{
		if (bit == HIGH || other.bit == HIGH)
			return Bit(HIGH);
		if (bit == UNDEF || other.bit == UNDEF)
			return Bit(UNDEF);
		return Bit(LOW);
	}

	inline Bit operator^(const Bit& other) const
	{
		if (bit == UNDEF || other.bit == UNDEF)
			return Bit(UNDEF);
		return Bit(bit ^ other.bit);
	}

	inline Bit operator~() const
	{
		return Bit(INVERT(bit));
	}

	// equality operators
	// NOTE: UNDEF is treated like any other value.
	// This helps with simulation when we check if a Bit changed.
	// When UNDEF -> UNDEF, the Bit effectively didn't change and we dont propagate further.

	inline bool operator==(const Bit& other) const
	{
		return bit == other.bit;
	}

	inline bool operator!=(const Bit& other) const
	{
		return bit != other.bit;
	}

	// bitwise methods that modify this Bit

	inline Bit& AND(const Bit& other)
	{
		if (bit == HIGH)
			bit = other.bit;
		else if (other.bit == LOW)
			bit = LOW;
		return *this;
	}

	inline Bit& OR(const Bit& other)
	{
		if (bit == LOW)
			bit = other.bit;
		else if (other.bit == HIGH)
			bit = HIGH;
		return *this;
	}

	inline Bit& XOR(const Bit& other)
	{
		if (bit == LOW)
			bit = other.bit;
		else if (bit == HIGH)
			bit = INVERT(other.bit);
		return *this;
	}

	inline Bit& NOT()
	{
		bit = INVERT(bit);
		return *this;
	}

	inline Bit& NAND(const Bit& other)
	{
		if (bit == LOW || other.bit == LOW)
			bit = HIGH;
		else if (bit == HIGH)
			bit = INVERT(other.bit);
		return *this;
	}

	inline Bit& NOR(const Bit& other)
	{
		if (bit == HIGH || other.bit == HIGH)
			bit = LOW;
		else if (bit == LOW)
			bit = INVERT(other.bit);
		return *this;
	}

	inline Bit& XNOR(const Bit& other)
	{
		if (bit == LOW)
			bit = INVERT(other.bit);
		else if (bit == HIGH)
			bit = other.bit;
		return *this;
	}
};

// print bit to stream
std::ostream& operator<<(std::ostream &out, const Bit& b);

#undef INVERT

#endif // BIT_H_
