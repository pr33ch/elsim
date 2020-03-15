#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sim.h"
#include "MUX4to1.h"

using namespace std;
void testMUX4to1(unsigned int sel0, unsigned int sel1) // magnitude of sel0 and sel1 must be <= 4 bits
{
	MUX4to1 mux4to1(2);
	mux4to1("A") <= 0;
	mux4to1("B") <= 1;
	mux4to1("C") <= 2;
	mux4to1("D") <= 3;

	// set inputs and simulate
	mux4to1("SEL", 0) <= sel0;
	mux4to1("SEL",1) <= sel1;
	cout<< "input sel0: " << mux4to1("SEL",0) << endl;
	cout<< "input sel1: " << mux4to1("SEL",1) << endl;
	mux4to1.simulate();
	cout << "selected value: " << mux4to1("Z") << endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "Usage: " << argv[0] << " sel0 sel1" << endl;
		return -1;
	}

	// read input values
	unsigned int sel0 = atoi(argv[1]);
	unsigned int sel1 = atoi(argv[2]);

	// test MUX4to1
	testMUX4to1(sel0, sel1);

}