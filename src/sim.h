// Top-level include file for the simulator.

// derived from Module
#include "Gate.h"	// AND,NAND,OR,NOR,XOR,XNOR
#include "INV.h"	// Inverter,Buffer
#include "FA.h"		// Full-Adder
#include "HA.h"		// Half-Adder
#include "MUX.h"	// Multiplexer
#include "CLK.h"	// Clock
#include "REG.h"	// Register,Latch
#include "GAP.h"	// Generate-Alive-Propagate for N-bit X and Y
#include "GP.h"		// Prefix computation of Generate and Propagate
#include "GroupPropagate.h" // `propagate` computation for a multi-bit group

// derived from SystemModule
#include "MetaGate.h"			// composite gate with arbitrary-width inputs
#include "FullAdder.h"			// full-adder (uses 2 HAs)
#include "RandomTree.h"			// tree of random gates
#include "LookaheadGenerator.h"	// carry-lookahead generation
#include "SaveAdder.h"			// carry-save adder (array of FAs)

// derived from Adder
#include "RippleAdder.h"		// Carry-ripple adder
#include "SkipAdder.h"			// Carry-skip adder
#include "SelectAdder.h"		// Carry-select adder
#include "LookaheadAdder.h"		// Carry-lookahead adder
#include "PrefixAdder.h"		// Prefix adder

// miscellaneous
#include "VCDWriter.h"			// Write VCD (waveform) files
