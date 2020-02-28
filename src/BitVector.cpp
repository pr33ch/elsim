#include "BitVector.h"
#include "Module.h"

BitVector::BitVector(int width)
{
	assert(width > 0);
	bits_.resize(width);
}

BitVector::BitVector(Bit b)
{
	bits_.push_back(b);
}

BitVector::BitVector(int width, value_t val)
{
	assert(width > 0 && width <= VALUE_T_BITS);
	bits_.reserve(width);

	for (int i = 0; i < width; i++, val >>= 1)
		bits_.push_back(Bit(val & 1));
}

BitVector::BitVector(const std::string& s)
{
	int L = s.length();
	assert(L > 0);
	bits_.reserve(L);

	for (int i = L-1; i >= 0; i--)
	{
		switch (s[i])
		{
		case '0': bits_.push_back(Bit(LOW));  break;
		case '1': bits_.push_back(Bit(HIGH)); break;
		case 'x': bits_.push_back(Bit());     break;
		default:  assert(0);
		}
	}
}

BitVector::BitVector(const Port& p)
{
	assert(p.module);
	int N = p.width();
	assert(N > 0);

	bits_.reserve(N);

	for (int i = p.low; i <= p.high; i++)
	{
		Bit b = p.module->getInput(i, Module::simTime());
		bits_.push_back(b);
	}
}

// static method
BitVector BitVector::random(int N)
{
	BitVector v(N);
	const int chunksize = sizeof(long int) * 8;
	for (int i = 0; N; )
	{
		long int r = ::random();
		int M = N > chunksize ? chunksize : N;
		for (int imax = i+M; i < imax; i++, r>>=1)
			v.set(i, (r&1)?HIGH:LOW);
		N -= M;
	}
	return v;
}

BitVector& BitVector::operator=(const Port& p)
{
	assert(p.module);
	int N = p.width();
	assert(N > 0);
	assert(N == width());

	for (int i = 0; i <= N; i++)
		bits_[i] = p.module->getInput(p.low+i, Module::simTime());

	return *this;
}

bool BitVector::operator==(const BitVector& other) const
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		if (bits_[i] != other.get(i))
			return false;

	return true;
}


bool BitVector::operator!=(const BitVector& other) const
{
	return !(*this == other);
}

BitVector BitVector::operator&(const BitVector& other) const
{
	int N = other.width();
	assert(N == width());

	BitVector vec(N);
	for (int i = 0; i < N; i++)
		vec.set(i, bits_[i] & other.get(i));
	return vec;
}

BitVector BitVector::operator|(const BitVector& other) const
{
	int N = other.width();
	assert(N == width());

	BitVector vec(N);
	for (int i = 0; i < N; i++)
		vec.set(i, bits_[i] | other.get(i));
	return vec;
}

BitVector BitVector::operator^(const BitVector& other) const
{
	int N = other.width();
	assert(N == width());

	BitVector vec(N);
	for (int i = 0; i < N; i++)
		vec.set(i, bits_[i] ^ other.get(i));
	return vec;
}

BitVector BitVector::operator~() const
{
	int N = width();
	BitVector vec(N);
	for (int i = 0; i < N; i++)
		vec.set(i, ~bits_[i]);
	return vec;
}

BitVector& BitVector::AND(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].AND(other.get(i));

	return *this;
}

BitVector& BitVector::NAND(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].NAND(other.get(i));

	return *this;
}

BitVector& BitVector::OR(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].OR(other.get(i));

	return *this;
}

BitVector& BitVector::NOR(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].NOR(other.get(i));

	return *this;
}

BitVector& BitVector::XOR(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].XOR(other.get(i));

	return *this;
}

BitVector& BitVector::XNOR(const BitVector& other)
{
	int N = other.width();
	assert(N == width());

	for (int i = 0; i < N; i++)
		bits_[i].XNOR(other.get(i));

	return *this;
}

BitVector& BitVector::NOT()
{
	int N = width();

	for (int i = 0; i < N; i++)
		bits_[i].NOT();

	return *this;
}

std::ostream& operator<<(std::ostream &out, const BitVector& vec)
{
	int N = vec.width();
	for (int i = N-1; i >= 0; i--)
		out << vec.get(i);
	return out;
}
