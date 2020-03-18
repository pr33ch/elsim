#include <iostream>
#include <fstream>
#include <cstdlib>
#include "sim.h"
#include "Rad2CarrySaveDivider.h"

using namespace std;
void testDivider(unsigned int x, int d) // magnitude of ws and wc must be <= 4 bits
{
	Rad2CarrySaveDivider divider(4, d, 1000);

	// set inputs and simulate
	divider("d") <= d;
	divider("WC") <= 0;
	divider("WS") <= (int) x/2;
	cout<< "input WS: ." << divider("WS") << endl;
	cout<< "input WC: ." << divider("WC") << endl;
	cout<< "input d: ." << divider("d") << endl;
	divider.clk_->simulate();
	divider.visualize();
	cout << "Area: " << divider.area() << endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "Usage: " << argv[0] << " x d" << endl;
		return -1;
	}

	// read input values
	unsigned int x = atoi(argv[1]);
	unsigned int d = atoi(argv[2]);

	// test divider
	testDivider(x,d);

}