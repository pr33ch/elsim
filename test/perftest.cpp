#include <iostream>
#include "sim.h"
using namespace std;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " levels" << endl;
		return -1;
	}

	int L = atoi(argv[1]);
	RandomTree tree(L);

	cout << "Simulating random gate tree with " << L << " levels..." << endl;

	struct timespec ts1,ts2;
	clock_gettime(CLOCK_REALTIME,&ts1);

	tree.simulate();

	clock_gettime(CLOCK_REALTIME,&ts2);

	cout << "Simulated output at T = " << tree.lastOutputTime(0) << endl;

	double t1 = (double)ts1.tv_sec + 1e-9*(double)ts1.tv_nsec;
	double t2 = (double)ts2.tv_sec + 1e-9*(double)ts2.tv_nsec;

	cout << "Simulation time: " << (t2-t1) << " seconds." << endl;

	return 0;
}
