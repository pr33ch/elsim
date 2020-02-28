#ifndef Rad4Multiplier12bDesign2_H_
#define Rad4Multiplier12bDesign2_H_

#include "SystemModule.h"
#include "SaveAdder.h"
#include "RippleAdder.h"
#include "SelectAdder.h"
#include "MultipleGenerator.h"
#include "ParallelRecoder.h"

// multiplies two N bit positive ints together, performing reduction using a linear arrangement of 3:2 adders followed by a carry select adder
// multiplication done using radix 4 recoding

// only tested on N = 12
class Rad4Multiplier12b : public SystemModule
{
private:
	void initSubmodules(int N)
	{
		// create submodules
		sel_adder_ = new SelectAdder(N*2+3, RippleAdder(1));
		submodule(sel_adder_);
		
		save_adders_.push_back(new SaveAdder(N*2));
		save_adders_.push_back(new SaveAdder(N*2+1));
		save_adders_.push_back(new SaveAdder(N*2+2));
		save_adders_.push_back(new SaveAdder(N*2+3));

		for (int i = 0; i <N/3; i++)
		{
			submodule(save_adders_[i]);
		}
		
		for (int i = 0; i <N/2; i++)
		{
			MultipleGenerators_.push_back(new MultipleGenerator(N,i));
			submodule(MultipleGenerators_[i]);
			ParallelRecoders_.push_back(new ParallelRecoder(N));
			submodule(ParallelRecoders_[i]);
		}
	}

	void attachSubmodules(int N)
	{
		for (int i = 0; i <N/2; i++)
		{
			ParallelRecoder& parallel_recoder = *ParallelRecoders_[i];

			MultipleGenerator& multiple_generator = *MultipleGenerators_[i];

			if (i == 0)
			{
				IN("Ci") >> parallel_recoder("y2j-1");
			}
			else
			{
				IN("Y", 2*i-1) >> parallel_recoder("y2j-1");
			}
			IN("Y", 2*i) >> parallel_recoder("y2j");
			IN("Y", 2*i + 1) >> parallel_recoder("y2j+1");

			IN("X") >> parallel_recoder("Xi");

			parallel_recoder("Xo") >> multiple_generator("X");
			parallel_recoder("sign") >> multiple_generator("sign");
			parallel_recoder("one") >> multiple_generator("one");
			parallel_recoder("two") >> multiple_generator("two");	

		}
		// attach inputs to first 3:2 adder stage
		for (int i = 0; i <N/2; i++)
		{
			MultipleGenerator& multiple_generator = *MultipleGenerators_[i];

			int idx;
			if (i < 3)
			{
				idx = 0;
			}
			else
			{
				idx = i%3+1;
			}
			SaveAdder& save_adder = *save_adders_[idx];

			// fill the save adder inputs
			for (int j = 0; j < 2*N; j++)
			{
				if (i < 3 && j != 2*(i-1))
				{
					multiple_generator("pp", j) >> save_adder(save_adder_port_names_[i], j);
				}
				else if(i >= 3 && j != 2*(i-1))
				{
					multiple_generator("pp", j) >> save_adder(save_adder_port_names_[2], j);
				}
				else{
					std::cout << i<<" : " << j << std::endl;
				}
			}
		}
		// attach carries
		SaveAdder& save_adder_first_stage_a = *save_adders_[0];
		SaveAdder& save_adder_first_stage_b = *save_adders_[1];
		(*ParallelRecoders_[0])("c") >> save_adder_first_stage_a("Y", 0);
		(*ParallelRecoders_[1])("c") >> save_adder_first_stage_a("Ci", 2);
		(*ParallelRecoders_[2])("c") >> save_adder_first_stage_b("Ci", 4);
		(*ParallelRecoders_[3])("c") >> (*save_adders_[2])("Ci", 6);
		(*ParallelRecoders_[4])("c") >> (*save_adders_[3])("Ci", 8);

		// attach next level 3:2 adder stages
		for (int j = 0; j < 2*N; j++)
		{
			(*save_adders_[0])("S",j) >> (*save_adders_[1])("X",j);
			(*save_adders_[0])("Co",j) >> (*save_adders_[1])("Y",j+1);
		}
		(*save_adders_[1])("X",2*N) <= Bit(0);
		(*save_adders_[1])("Y",0) <= Bit(0);
		(*save_adders_[1])("Y",2*N) <= Bit(0);

		for (int j = 0; j < 2*N+1; j++)
		{
			(*save_adders_[1])("S",j) >> (*save_adders_[2])("X",j);
			(*save_adders_[1])("Co",j) >> (*save_adders_[2])("Y",j+1);
		}
		(*save_adders_[2])("X",2*N+1) <= Bit(0);
		(*save_adders_[2])("Y",0) <= Bit(0);
		(*save_adders_[2])("Y",2*N) <= Bit(0);
		(*save_adders_[2])("Y",2*N+1) <= Bit(0);

		for (int j = 0; j < 2*N+2; j++)
		{
			(*save_adders_[2])("S",j) >> (*save_adders_[3])("X",j);
			(*save_adders_[2])("Co",j) >> (*save_adders_[3])("Y",j+1);
		}
		(*save_adders_[3])("X",2*N+2) <= Bit(0);
		(*save_adders_[3])("Y",0) <= Bit(0);
		(*save_adders_[3])("Y",2*N) <= Bit(0);
		(*save_adders_[3])("Y",2*N+1) <= Bit(0);
		(*save_adders_[3])("Y",2*N+2) <= Bit(0);

		SelectAdder& sel_adder = *sel_adder_;
		for (int j = 0; j < 2*N+2; j++)
		{
			(*save_adders_[3])("S",j) >> sel_adder("X",j);
			(*save_adders_[3])("Co",j) >> sel_adder("Y",j+1); 
		}
		sel_adder("X", 2*N+2) <= Bit(0);
		sel_adder("Y", 0) <= Bit(0);

		for (int j = 0; j < 2*N; j++)
		{
			OUT("P",j) << sel_adder("S",j);
		}
		sel_adder("Ci") <= Bit(0);
		OUT("Co") << sel_adder("S", 24);
	}

public:
	std::vector<SaveAdder*> save_adders_;
	SelectAdder* sel_adder_;
	std::vector<MultipleGenerator*> MultipleGenerators_;
	std::vector<ParallelRecoder*> ParallelRecoders_;
	std::string save_adder_port_names_[3] = {"X", "Y", "Ci"};

	Rad4Multiplier12b(int N)
	{
		// int N = 12;
		addInput("X", N);
		addInput("Y", N);
		addInput("Ci");

		addOutput("P", 2*N+3);
		addOutput("Co");

		std::stringstream ss;
		ss << "Rad4Multiplier12b<" << N << ">";
		classname_ = ss.str();
		
		initSubmodules(N);
		
		attachSubmodules(N);
		
	}

	~Rad4Multiplier12b() { delete sel_adder_;}

	int width() const { return numOutputs() / 2; }
};

#endif
