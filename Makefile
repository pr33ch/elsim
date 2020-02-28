.PHONY: all opt debug clean

# MAINSRC := test/adders.cpp
#MAINSRC := test/addtests.cpp
#MAINSRC := test/addtests2.cpp
#MAINSRC := test/perftest.cpp
#MAINSRC := test/prefix8to128.cpp
#MAINSRC := test/cla8to128.cpp
MAINSRC := test/mult_test.cpp
# MAINSRC := test/multgen_test.cpp

SRC := src/Bit.cpp src/BitVector.cpp src/Module.cpp src/SystemModule.cpp

SIM := elsim

FLAGS := -Isrc -Itest -Wall -o $(SIM)

# perftest needs real-time clock library
ifeq ($(MAINSRC),test/perftest.cpp)
MAINSRC += -lrt
endif

all:
	g++ $(FLAGS) -DMOD_EXTRA $(SRC) $(MAINSRC)
opt:
	g++ $(FLAGS) -DNDEBUG -O3 $(SRC) $(MAINSRC)
debug:
	g++ $(FLAGS) -DMOD_EXTRA -DDEBUG -g $(SRC) $(MAINSRC)
clean:
	rm -f ./$(SIM)
