#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sim.h"
#include "QConv.h"
#include "CLK.h"

using namespace std;
void testQConv(unsigned int ws, unsigned int wc, unsigned int q) // magnitude of ws and wc must be <= 4 bits
{
	QConv qconv(2, true);

	// set inputs and simulate
	qconv("WS") <= ws;
	qconv("WC") <= wc;
	qconv("q") <= q;
	cout<< "input WS: " << qconv("WS") << endl;
	cout<< "input WC: " << qconv("WC") << endl;
	cout<< "input q: " << qconv("q") << endl;

	qconv.simulate();
	cout << "Q: " << qconv("Q") << endl;
	cout << "QN: " << qconv("QN") << endl;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "Usage: " << argv[0] << " ws wc q" << endl;
		return -1;
	}

	// read input values
	unsigned int ws = atoi(argv[1]);
	unsigned int wc = atoi(argv[2]);
	unsigned int q = atoi(argv[3]);

	// test QConv
	testQConv(ws, wc, q);

}