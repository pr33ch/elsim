#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sim.h"
#include "QSel.h"

using namespace std;
void testQSel(unsigned int ws, unsigned int wc) // magnitude of ws and wc must be <= 4 bits
{
	QSel qsel;
	// set inputs and simulate
	qsel("WS") <= ws;
	qsel("WC") <= wc;
	cout<< "input WS: " << qsel("WS") << endl;
	cout<< "input WC: " << qsel("WC") << endl;
	qsel.simulate();
	cout << "magnitude: " << qsel("qm") << endl;
	cout << "sign: " << qsel("qs") << endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "Usage: " << argv[0] << " ws wc" << endl;
		return -1;
	}

	// read input values
	unsigned int ws = atoi(argv[1]);
	unsigned int wc = atoi(argv[2]);

	// test qsel
	testQSel(ws, wc);

}