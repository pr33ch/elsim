#include <iostream>
#include <fstream>
#include <cstdlib>
#include "MultipleGenerator.h"
using namespace std;

void testGenerator(MultipleGenerator& g, value_t x)
{
	// set inputs and simulate
	g("sign") <= 0;
	cout << g("sign") << endl;
	g("one") <= 0;
	cout << g("one") << endl;
	g("two") <= 0;
	cout << g("two") << endl;
	g("X") <= x;
	cout << g("X") << endl;
	g.simulate();
	cout << "partial product " << g("pp") << endl;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " X" << endl;
		return -1;
	}

	// read input values
	value_t x = atoi(argv[1]);

	MultipleGenerator g(12, 1);

	// test adders
	testGenerator(g, x);

}