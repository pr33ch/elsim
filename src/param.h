#ifndef PARAM_H_
#define PARAM_H_

////////////////////////////////////////
// Delay / Area / Energy parameters
////////////////////////////////////////
//
// Area and energy are floating-point.
// Delay is integer/fixed-point (for now).
//
// Delay type (delay_t) is defined in BitHistory.h
// Area type (area_t) is defined in Module.h
// Energy type (energy_t) is defined in Module.h
//

// Use fanout in delay calculations?
// i.e. Delay = delay(inum,onum) + fanout(onum)
#define USE_FANOUT_DELAY 1

// Inverter delay/load/area/energy
#define DELAY_INV  1
#define LOAD_INV   1
#define AREA_INV   1
#define ENERGY_INV 1

// Gate delay (n-input)
// Basically all the same since we aren't optimizing
// basic modules by using NAND/NOR instead of AND/OR.
#define DELAY_AND(n)  (11+(n)*4)
#define DELAY_NAND(n) (11+(n)*4)
#define DELAY_OR(n)   (11+(n)*4)
#define DELAY_NOR(n)  (11+(n)*4)
#define DELAY_XOR(n)  (30+(n)*16)
#define DELAY_XNOR(n) (30+(n)*16)

// Gate input load
// (used to compute fanout load)
#define LOAD_AND  3
#define LOAD_NAND 3
#define LOAD_OR   3
#define LOAD_NOR  3
#define LOAD_XOR  6
#define LOAD_XNOR 6

// Gate area (n-input)
#define AREA_AND(n)  (n)
#define AREA_NAND(n) (n)
#define AREA_OR(n)   (n)
#define AREA_NOR(n)  (n)
#define AREA_XOR(n)  (2*(n))
#define AREA_XNOR(n) (2*(n))

// Gate switching energy
// (will be multiplied by fanin for actual energy)
#define ENERGY_AND  1
#define ENERGY_NAND 1
#define ENERGY_OR   1
#define ENERGY_NOR  1
#define ENERGY_XOR  2
#define ENERGY_XNOR 2

// Latch clock-to-Q delay (assume 2 NANDs)
#define LATCH_CLK2Q (2*DELAY_NAND(2))
// Latch cell area (assume 4 NANDs)
#define LATCH_AREA (4*AREA_NAND(2))
// Latch cell energy (assume 4 NANDs)
#define LATCH_ENERGY (4*2*ENERGY_NAND)

// Register clock-to-Q delay (assume 3 NANDs)
#define REG_CLK2Q (3*DELAY_NAND(2))
// Register cell area (assume 6 NANDs)
#define REG_AREA (6*AREA_NAND(2))
// Register cell energy (assume 6 NANDs)
#define REG_ENERGY (6*2*ENERGY_NAND)

#endif
