#ifndef Rad4Multiplier12b_H_
#define Rad4Multiplier12b_H_

#include "FA.h"
#include "SystemModule.h"
#include "SaveAdder.h"
#include "RippleAdder.h"
#include "SelectAdder.h"
#include "MultipleGenerator.h"
#include "LRParallelRecoder.h"
//Static critical path: X[0] -> P[23] (T=1468)
// Area: 1288
// multiplies two N bit numbers together, using scheme from ”High-Performance Low-Power Left-to-Right Array
// Multiplier Design”, Z. Huang and M. D. Ercegovac
// multiplication done using radix 4 recoding
class Rad4Multiplier12b : public SystemModule
{
private:
	void initSubmodules(int N)
	{
		// create submodules
		ripple_adder_ = new RippleAdder(N*2-1);
		submodule(ripple_adder_);
		
		big_save_adder_ = new SaveAdder(N-3);
		submodule(big_save_adder_);
		small_save_adder_ = new SaveAdder(2);
		submodule(small_save_adder_);

		bottom_row_4_2_adders_stage_1_.push_back(new FA());
		bottom_row_4_2_adders_stage_1_.push_back(new FA());
		bottom_row_4_2_adders_stage_1_.push_back(new FA());
		bottom_row_4_2_adders_stage_1_.push_back(new FA());

		bottom_row_4_2_adders_stage_2_.push_back(new FA());
		bottom_row_4_2_adders_stage_2_.push_back(new FA());
		bottom_row_4_2_adders_stage_2_.push_back(new FA());
		bottom_row_4_2_adders_stage_2_.push_back(new FA());
		
		bottom_row_3_2_adders_.push_back(new FA());
		bottom_row_3_2_adders_.push_back(new FA());
		bottom_row_3_2_adders_.push_back(new FA());

		for(int i = 0; i < 4; i++)
		{
			submodule(bottom_row_4_2_adders_stage_1_[i]);
			submodule(bottom_row_4_2_adders_stage_2_[i]);

			if (i < 3)
			{
				submodule(bottom_row_3_2_adders_[i]);
			}
		}

		// adders for top three partial products
		row0_FAs_.push_back(new FA());
		row0_FAs_.push_back(new FA());
		row0_FAs_.push_back(new FA());
		row0_FAs_.push_back(new FA());
		row0_FAs_.push_back(new FA());

		for(int i = 0; i < row0_FAs_.size(); i++)
		{
			submodule(row0_FAs_[i]);
		}

		row0_HAs_.push_back(new HA());
		row0_HAs_.push_back(new HA());
		row0_HAs_.push_back(new HA());
		row0_HAs_.push_back(new HA());
		row0_HAs_.push_back(new HA());

		for(int i = 0; i < row0_HAs_.size(); i++)
		{
			submodule(row0_HAs_[i]);
		}
		//////////////////////////////////////////////////////////
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());
		row1_FAs_.push_back(new FA());

		for(int i = 0; i < row1_FAs_.size(); i++)
		{
			submodule(row1_FAs_[i]);
		}

		row1_HAs_.push_back(new HA());
		row1_HAs_.push_back(new HA());
		row1_HAs_.push_back(new HA());
		row1_HAs_.push_back(new HA());
		row1_HAs_.push_back(new HA());

		for(int i = 0; i < row1_HAs_.size(); i++)
		{
			submodule(row1_HAs_[i]);
		}
		///////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());
		row2_FAs_.push_back(new FA());

		for(int i = 0; i < row2_FAs_.size(); i++)
		{
			submodule(row2_FAs_[i]);
		}

		row2_HAs_.push_back(new HA());

		for(int i = 0; i < row2_HAs_.size(); i++)
		{
			submodule(row2_HAs_[i]);
		}
		///////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////
		row3_FAs_.push_back(new FA());
		row3_FAs_.push_back(new FA());
		row3_FAs_.push_back(new FA());
		row3_FAs_.push_back(new FA());
		row3_FAs_.push_back(new FA());
		row3_FAs_.push_back(new FA());

		for(int i = 0; i < row3_FAs_.size(); i++)
		{
			submodule(row3_FAs_[i]);
		}

		row3_HAs_.push_back(new HA());
		row3_HAs_.push_back(new HA());
		row3_HAs_.push_back(new HA());
		row3_HAs_.push_back(new HA());

		for(int i = 0; i < row3_HAs_.size(); i++)
		{
			submodule(row3_HAs_[i]);
		}
		///////////////////////////////////////////////////////////////////

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

		(*ripple_adder_)("Ci") <= Bit(0);

		std::vector<MultipleGenerator*> multiple_generator = MultipleGenerators_;
		std::vector<ParallelRecoder*> parallel_recoder = ParallelRecoders_;

		// attach the least shifted partial products first
		OUT("P", 0) << (*parallel_recoder[0])("ppi_new");

		for (int i = 1; i <= 23; i++)
		{
			OUT("P", i) << (*ripple_adder_)("S", i-1);
		}
		OUT("Co") << (*ripple_adder_)("Co");

		(*MultipleGenerators_[0])("pp", 0) >> (*ripple_adder_)("X", 0);
		(*multiple_generator[0])("pp", 1) >> (*ripple_adder_)("X", 1);

		int row3_FA_idx = 0;
		int row3_HA_idx = 0;
		
		(*multiple_generator[0])("pp", 2) >> (*row3_HAs_[row3_HA_idx])("X");
		row3_HA_idx++;

		(*multiple_generator[0])("pp", 3) >> (*row3_HAs_[row3_HA_idx])("X");
		row3_HA_idx++;

		(*multiple_generator[0])("pp", 4) >> (*row3_HAs_[row3_HA_idx])("X");
		row3_HA_idx++;

		(*multiple_generator[0])("pp", 5) >> (*row3_HAs_[row3_HA_idx])("X");
		row3_HA_idx++;

		(*multiple_generator[0])("pp", 6) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;

		(*multiple_generator[0])("pp", 7) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;

		(*multiple_generator[0])("pp", 8) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;

		(*multiple_generator[0])("pp", 9) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;

		(*multiple_generator[0])("pp", 10) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;

		(*multiple_generator[0])("pp", 11) >> (*row3_FAs_[row3_FA_idx])("X");
		row3_FA_idx++;
		// attach 2nd least shifted partial products
		(*parallel_recoder[0])("c") >> (*ripple_adder_)("Y", 0);
		(*parallel_recoder[1])("ppi_new") >> (*ripple_adder_)("Y", 1);
		(*multiple_generator[1])("pp",2) >> (*ripple_adder_)("Y", 2);
		(*multiple_generator[1])("pp",3) >> (*big_save_adder_)("Y",0);
		(*multiple_generator[1])("pp",4) >> (*big_save_adder_)("Y",1);

		int row2_HA_idx = 0;
		int row2_FA_idx = 0;
		(*multiple_generator[1])("pp",5) >> (*row2_HAs_[row2_HA_idx])("X");

		(*multiple_generator[1])("pp",6) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",7) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",8) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",9) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",10) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",11) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		(*multiple_generator[1])("pp",12) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;
		
		(*multiple_generator[1])("pp",13) >> (*row2_FAs_[row2_FA_idx])("X");
		row2_FA_idx++;

		//  attach 3rd least shifted partial products
		(*parallel_recoder[1])("c") >> (*row3_HAs_[0])("Y");
		(*parallel_recoder[2])("ppi_new") >> (*row3_HAs_[1])("Y");

		int row1_HA_idx = 0;
		int row1_FA_idx = 0;
		(*multiple_generator[2])("pp",4) >> (*row1_HAs_[row1_HA_idx])("X");
		row1_HA_idx++;

		(*multiple_generator[2])("pp",5) >> (*row1_HAs_[row1_HA_idx])("X");
		row1_HA_idx++;

		(*multiple_generator[2])("pp",6) >> (*row1_HAs_[row1_HA_idx])("X");
		row1_HA_idx++;

		(*multiple_generator[2])("pp",7) >> (*row1_HAs_[row1_HA_idx])("X");
		row1_HA_idx++;

		(*multiple_generator[2])("pp",8) >> (*row1_HAs_[row1_HA_idx])("X");
		row1_HA_idx++;

		(*multiple_generator[2])("pp",9) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",10) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",11) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",12) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",13) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",14) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		(*multiple_generator[2])("pp",15) >> (*row1_FAs_[row1_FA_idx])("X");
		row1_FA_idx++;

		// attach 4th least shifted pp
		row1_HA_idx = 0;
		row1_FA_idx = 0;
		(*parallel_recoder[2])("c") >> (*row1_HAs_[row1_HA_idx])("Y");
		row1_HA_idx++;

		(*parallel_recoder[3])("ppi_new") >> (*row1_HAs_[row1_HA_idx])("Y");
		row1_HA_idx++;

		(*multiple_generator[3])("pp",6) >> (*row1_HAs_[row1_HA_idx])("Y");
		row1_HA_idx++;

		(*multiple_generator[3])("pp",7) >> (*row1_HAs_[row1_HA_idx])("Y");
		row1_HA_idx++;
		std::cout << "hello" << std::endl;

		(*multiple_generator[3])("pp",8) >> (*row1_HAs_[row1_HA_idx])("Y");
		row1_HA_idx++;

		(*multiple_generator[3])("pp",9) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",10) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",11) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",12) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;
		std::cout << "hello333" << std::endl;

		(*multiple_generator[3])("pp",13) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",14) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",15) >> (*row1_FAs_[row1_FA_idx])("Y");
		row1_FA_idx++;

		(*multiple_generator[3])("pp",16) >> (*bottom_row_4_2_adders_stage_1_[2])("Ci");
		(*multiple_generator[3])("pp",17) >> (*bottom_row_3_2_adders_[2])("Ci");

		(*parallel_recoder[3])("c") >> (*row2_FAs_[0])("Y");
		(*parallel_recoder[4])("ppi_new") >> (*row2_FAs_[1])("Y");
		std::cout << "hello" << std::endl;

		int row0_FA_idx = 0;
		int row0_HA_idx = 0;
		(*multiple_generator[4])("pp",8) >> (*row0_HAs_[row0_HA_idx])("X");
		row0_HA_idx++;
		(*multiple_generator[4])("pp",9) >> (*row0_HAs_[row0_HA_idx])("X");
		row0_HA_idx++;

		(*multiple_generator[4])("pp",10) >> (*row0_FAs_[row0_FA_idx])("X");
		row0_FA_idx++;

		(*multiple_generator[4])("pp",11) >> (*row0_FAs_[row0_FA_idx])("X");
		row0_FA_idx++;

		(*multiple_generator[4])("pp",12) >> (*row0_FAs_[row0_FA_idx])("X");
		row0_FA_idx++;

		(*multiple_generator[4])("pp",13) >> (*row0_HAs_[row0_HA_idx])("X");
		row0_HA_idx++;

		(*multiple_generator[4])("pp",14) >> (*row0_FAs_[row0_FA_idx])("X");
		row0_FA_idx++;

		(*multiple_generator[4])("pp",15) >> (*row0_HAs_[row0_HA_idx])("X");
		row0_HA_idx++;

		(*multiple_generator[4])("pp",16) >> (*row0_FAs_[row0_FA_idx])("X");
		row0_FA_idx++;

		(*multiple_generator[4])("pp",17) >> (*row0_HAs_[row0_HA_idx])("X");
		row0_HA_idx++;
		std::cout << "asdf" << std::endl;

		(*multiple_generator[4])("pp",18) >> (*bottom_row_4_2_adders_stage_1_[3])("X");
		(*multiple_generator[4])("pp",19) >> (*small_save_adder_)("X",0);
				std::cout << "hefffllo" << std::endl;

		(*small_save_adder_)("X",1) <= Bit(0);
		(*small_save_adder_)("Ci",0) <= Bit(0);

		std::cout << "hello" << std::endl;

		row0_FA_idx = 0;
		row0_HA_idx = 0;
		(*parallel_recoder[4])("c") >> (*row0_HAs_[row0_HA_idx])("Y");
		row0_HA_idx++;

		(*parallel_recoder[5])("ppi_new") >> (*row0_HAs_[row0_HA_idx])("Y");
		row0_HA_idx++;

		(*multiple_generator[5])("pp",10) >> (*row0_FAs_[row0_FA_idx])("Y");
		row0_FA_idx++;

		(*multiple_generator[5])("pp",11) >> (*row0_FAs_[row0_FA_idx])("Y");
		row0_FA_idx++;

		(*multiple_generator[5])("pp",12) >> (*row0_FAs_[row0_FA_idx])("Y");
		row0_FA_idx++;

		(*multiple_generator[5])("pp",13) >> (*row0_HAs_[row0_HA_idx])("Y");
		row0_HA_idx++;

		(*multiple_generator[5])("pp",14) >> (*row0_FAs_[row0_FA_idx])("Y");
		row0_FA_idx++;

		(*multiple_generator[5])("pp",15) >> (*row0_HAs_[row0_HA_idx])("Y");
		row0_HA_idx++;

		(*multiple_generator[5])("pp",16) >> (*row0_FAs_[row0_FA_idx])("Y");
		row0_FA_idx++;

		(*multiple_generator[5])("pp",17) >> (*row0_HAs_[row0_HA_idx])("Y");
		row0_HA_idx++;

		(*multiple_generator[5])("pp",18) >> (*bottom_row_4_2_adders_stage_1_[3])("Y");
		(*multiple_generator[5])("pp",19) >> (*small_save_adder_)("Y",0);
		(*multiple_generator[5])("pp",20) >> (*small_save_adder_)("Y",1);
		(*multiple_generator[5])("pp",21) >> (*ripple_adder_)("X",21);

		(*parallel_recoder[5])("c") >> (*row0_FAs_[0])("Ci");
		(*row0_FAs_[1])("Ci") <= Bit(1);
		(*row0_FAs_[2])("Ci") <= Bit(1);
		(*row0_FAs_[3])("Ci") <= Bit(1);
		(*row0_FAs_[4])("Ci") <= Bit(1);

		(*bottom_row_4_2_adders_stage_1_[3])("Ci") <= Bit(1);
		(*small_save_adder_)("Ci",0) <= Bit(0);
		(*small_save_adder_)("Ci",1) <= Bit(1);

		(*ripple_adder_)("X",22) <= Bit(1);	
		(*ripple_adder_)("Y",22) <= Bit(0);	

		////////////////////////////////////////////////////////////////////////////////////

		// connect row 0 adder outputs
		row0_HA_idx = 0;
		row0_FA_idx = 0;

		(*row0_HAs_[row0_HA_idx])("C") >> (*row1_FAs_[0])("Ci");
		(*row0_HAs_[row0_HA_idx])("S") >> (*row2_FAs_[2])("Y");
		row0_HA_idx++;

		(*row0_HAs_[row0_HA_idx])("C") >> (*row1_FAs_[1])("Ci");
		(*row0_HAs_[row0_HA_idx])("S") >> (*row2_FAs_[3])("Y");
		row0_HA_idx++;

		(*row0_FAs_[row0_FA_idx])("Co") >> (*row1_FAs_[2])("Ci");
		(*row0_FAs_[row0_FA_idx])("S") >> (*row2_FAs_[4])("Y");
		row0_FA_idx++;

		(*row0_FAs_[row0_FA_idx])("Co") >> (*row1_FAs_[3])("Ci");
		(*row0_FAs_[row0_FA_idx])("S") >> (*row2_FAs_[5])("Y");
		row0_FA_idx++;

		(*row0_FAs_[row0_FA_idx])("Co") >> (*row1_FAs_[4])("Ci");
		(*row0_FAs_[row0_FA_idx])("S") >> (*row2_FAs_[6])("Y");
		row0_FA_idx++;

		(*row0_HAs_[row0_HA_idx])("C") >> (*row1_FAs_[5])("Ci");
		(*row0_HAs_[row0_HA_idx])("S") >> (*row2_FAs_[7])("Y");
		row0_HA_idx++;

		(*row0_FAs_[row0_FA_idx])("Co") >> (*row1_FAs_[6])("Ci");
		(*row0_FAs_[row0_FA_idx])("S") >> (*bottom_row_4_2_adders_stage_1_[1])("X");
		row0_FA_idx++;

		(*row0_HAs_[row0_HA_idx])("C") >> (*bottom_row_4_2_adders_stage_1_[2])("Y");
		(*row0_HAs_[row0_HA_idx])("S") >> (*bottom_row_3_2_adders_[1])("X");
		row0_HA_idx++;

		(*row0_FAs_[row0_FA_idx])("Co") >> (*bottom_row_3_2_adders_[2])("Y");
		(*row0_FAs_[row0_FA_idx])("S") >> (*bottom_row_4_2_adders_stage_1_[2])("X");
		row0_FA_idx++;

		(*row0_HAs_[row0_HA_idx])("C") >> (*bottom_row_4_2_adders_stage_2_[3])("Ci");
		(*row0_HAs_[row0_HA_idx])("S") >> (*bottom_row_3_2_adders_[2])("X");
		row0_HA_idx++;

		//connect row 1 adder outputs
		row1_HA_idx = 0;
		row1_FA_idx = 0;
		(*row1_HAs_[row1_HA_idx])("S") >> (*row3_HAs_[2])("Y");
		(*row1_HAs_[row1_HA_idx])("C") >> (*row2_HAs_[0])("Y");
		row1_HA_idx++;

		(*row1_HAs_[row1_HA_idx])("S") >> (*row3_HAs_[3])("Y");
		(*row1_HAs_[row1_HA_idx])("C") >> (*row2_FAs_[0])("Ci");
		row1_HA_idx++;		

		(*row1_HAs_[row1_HA_idx])("S") >> (*row3_FAs_[0])("Y");
		(*row1_HAs_[row1_HA_idx])("C") >> (*row2_FAs_[1])("Ci");
		row1_HA_idx++;

		(*row1_HAs_[row1_HA_idx])("S") >> (*row3_FAs_[1])("Y");
		(*row1_HAs_[row1_HA_idx])("C") >> (*row2_FAs_[2])("Ci");
		row1_HA_idx++;		

		(*row1_HAs_[row1_HA_idx])("S") >> (*row3_FAs_[2])("Y");
		(*row1_HAs_[row1_HA_idx])("C") >> (*row2_FAs_[3])("Ci");
		row1_HA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*row3_FAs_[3])("Y");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*row2_FAs_[4])("Ci");
		row1_FA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*row3_FAs_[4])("Y");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*row2_FAs_[5])("Ci");
		row1_FA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*row3_FAs_[5])("Y");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*row2_FAs_[6])("Ci");
		row1_FA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*bottom_row_4_2_adders_stage_1_[0])("X");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*row2_FAs_[7])("Ci");
		row1_FA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*bottom_row_3_2_adders_[0])("X");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*bottom_row_4_2_adders_stage_1_[1])("Ci");
		row1_FA_idx++;
		
		(*row1_FAs_[row1_FA_idx])("S") >> (*bottom_row_4_2_adders_stage_1_[1])("Y");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*bottom_row_3_2_adders_[1])("Ci");
		row1_FA_idx++;

		(*row1_FAs_[row1_FA_idx])("S") >> (*bottom_row_3_2_adders_[1])("Y");
		(*row1_FAs_[row1_FA_idx])("Co") >> (*bottom_row_4_2_adders_stage_2_[2])("Ci");
		row1_FA_idx++;

		// connect row 2 adder outputs
		row2_HA_idx = 0;
		row2_FA_idx = 0;
		(*row2_HAs_[row2_HA_idx])("S") >> (*big_save_adder_)("Y", 2);
		(*row2_HAs_[row2_HA_idx])("C") >> (*row3_FAs_[0])("Ci");
		row2_HA_idx++;

		
		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 3);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*row3_FAs_[1])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 4);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*row3_FAs_[2])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 5);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*row3_FAs_[3])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 6);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*row3_FAs_[4])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 7);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*row3_FAs_[5])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*big_save_adder_)("Y", 8);
		(*row2_FAs_[row2_FA_idx])("Co") >> (*bottom_row_4_2_adders_stage_1_[0])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*bottom_row_4_2_adders_stage_1_[0])("Y");
		(*row2_FAs_[row2_FA_idx])("Co") >> (*bottom_row_3_2_adders_[0])("Ci");
		row2_FA_idx++;

		(*row2_FAs_[row2_FA_idx])("S") >> (*bottom_row_3_2_adders_[0])("Y");
		(*row2_FAs_[row2_FA_idx])("Co") >> (*bottom_row_4_2_adders_stage_2_[1])("Ci");
		row2_FA_idx++;

		// connect row 3 adder outputs
		row3_HA_idx = 0;
		row3_FA_idx = 0;
		(*row3_HAs_[row3_HA_idx])("S") >> (*ripple_adder_)("X", 2);
		(*row3_HAs_[row3_HA_idx])("C") >> (*big_save_adder_)("Ci",0);
		row3_HA_idx++;

		(*row3_HAs_[row3_HA_idx])("S") >> (*big_save_adder_)("X", 0);
		(*row3_HAs_[row3_HA_idx])("C") >> (*big_save_adder_)("Ci",1);
		row3_HA_idx++;

		(*row3_HAs_[row3_HA_idx])("S") >> (*big_save_adder_)("X", 1);
		(*row3_HAs_[row3_HA_idx])("C") >> (*big_save_adder_)("Ci",2);
		row3_HA_idx++;

		(*row3_HAs_[row3_HA_idx])("S") >> (*big_save_adder_)("X", 2);
		(*row3_HAs_[row3_HA_idx])("C") >> (*big_save_adder_)("Ci",3);
		row3_HA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 3);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*big_save_adder_)("Ci",4);
		row3_FA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 4);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*big_save_adder_)("Ci",5);
		row3_FA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 5);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*big_save_adder_)("Ci",6);
		row3_FA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 6);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*big_save_adder_)("Ci",7);
		row3_FA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 7);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*big_save_adder_)("Ci",8);
		row3_FA_idx++;

		(*row3_FAs_[row3_FA_idx])("S") >> (*big_save_adder_)("X", 8);
		(*row3_FAs_[row3_FA_idx])("Co") >> (*bottom_row_4_2_adders_stage_2_[0])("Ci");
		row3_FA_idx++;

		// connect stages of the bottom row 4 2 adders
		(*bottom_row_4_2_adders_stage_1_[0])("S") >> (*bottom_row_4_2_adders_stage_2_[0])("X");
		(*bottom_row_4_2_adders_stage_1_[0])("Co") >> (*bottom_row_4_2_adders_stage_2_[0])("Y");

		(*bottom_row_4_2_adders_stage_1_[1])("S") >> (*bottom_row_4_2_adders_stage_2_[1])("X");
		(*bottom_row_4_2_adders_stage_1_[1])("Co") >> (*bottom_row_4_2_adders_stage_2_[1])("Y");

		(*bottom_row_4_2_adders_stage_1_[2])("S") >> (*bottom_row_4_2_adders_stage_2_[2])("X");
		(*bottom_row_4_2_adders_stage_1_[2])("Co") >> (*bottom_row_4_2_adders_stage_2_[2])("Y");

		(*bottom_row_4_2_adders_stage_1_[3])("S") >> (*bottom_row_4_2_adders_stage_2_[3])("X");
		(*bottom_row_4_2_adders_stage_1_[3])("Co") >> (*bottom_row_4_2_adders_stage_2_[3])("Y");

		// connect remaining inputs to ripple adder
		(*big_save_adder_)("S", 0) >> (*ripple_adder_)("X",3);
		(*ripple_adder_)("Y", 3) <= Bit(0);

		(*big_save_adder_)("Co", 0) >> (*ripple_adder_)("Y",4);
		(*big_save_adder_)("S", 1) >> (*ripple_adder_)("X",4);

		(*big_save_adder_)("Co", 1) >> (*ripple_adder_)("Y",5);
		(*big_save_adder_)("S", 2) >> (*ripple_adder_)("X",5);

		(*big_save_adder_)("Co", 2) >> (*ripple_adder_)("Y",6);
		(*big_save_adder_)("S", 3) >> (*ripple_adder_)("X",6);

		(*big_save_adder_)("Co", 3) >> (*ripple_adder_)("Y",7);
		(*big_save_adder_)("S", 4) >> (*ripple_adder_)("X",7);

		(*big_save_adder_)("Co", 4) >> (*ripple_adder_)("Y",8);
		(*big_save_adder_)("S", 5) >> (*ripple_adder_)("X",8);

		(*big_save_adder_)("Co", 5) >> (*ripple_adder_)("Y",9);
		(*big_save_adder_)("S", 6) >> (*ripple_adder_)("X",9);

		(*big_save_adder_)("Co", 6) >> (*ripple_adder_)("Y",10);
		(*big_save_adder_)("S", 7) >> (*ripple_adder_)("X",10);

		(*big_save_adder_)("Co", 7) >> (*ripple_adder_)("Y",11);
		(*big_save_adder_)("S", 8) >> (*ripple_adder_)("X",11);

		(*big_save_adder_)("Co", 8) >> (*ripple_adder_)("Y",12);

		(*bottom_row_4_2_adders_stage_2_[0])("S") >> (*ripple_adder_)("X",12);
		(*bottom_row_4_2_adders_stage_2_[0])("Co") >> (*ripple_adder_)("Y",13);

		(*bottom_row_3_2_adders_[0])("S") >> (*ripple_adder_)("X", 13);
		(*bottom_row_3_2_adders_[0])("Co") >> (*ripple_adder_)("Y", 14);

		(*bottom_row_4_2_adders_stage_2_[1])("S") >> (*ripple_adder_)("X",14);
		(*bottom_row_4_2_adders_stage_2_[1])("Co") >> (*ripple_adder_)("Y",15);

		(*bottom_row_3_2_adders_[1])("S") >> (*ripple_adder_)("X", 15);
		(*bottom_row_3_2_adders_[1])("Co") >> (*ripple_adder_)("Y", 16);

		(*bottom_row_4_2_adders_stage_2_[2])("S") >> (*ripple_adder_)("X",16);
		(*bottom_row_4_2_adders_stage_2_[2])("Co") >> (*ripple_adder_)("Y",17);

		(*bottom_row_3_2_adders_[2])("S") >> (*ripple_adder_)("X", 17);
		(*bottom_row_3_2_adders_[2])("Co") >> (*ripple_adder_)("Y", 18);

		(*bottom_row_4_2_adders_stage_2_[3])("S") >> (*ripple_adder_)("X",18);
		(*bottom_row_4_2_adders_stage_2_[3])("Co") >> (*ripple_adder_)("Y",19);

		(*small_save_adder_)("S", 0) >> (*ripple_adder_)("X", 19);
		(*small_save_adder_)("Co", 0) >> (*ripple_adder_)("Y", 20);
		(*small_save_adder_)("S", 1) >> (*ripple_adder_)("X", 20);
		(*small_save_adder_)("Co", 1) >> (*ripple_adder_)("Y", 21);

	}

public:
	SaveAdder* big_save_adder_;
	SaveAdder* small_save_adder_;
	std::vector<FA*> bottom_row_4_2_adders_stage_1_;
	std::vector<FA*> bottom_row_4_2_adders_stage_2_;
	std::vector<FA*> bottom_row_3_2_adders_;

	std::vector<FA*> row0_FAs_;
	std::vector<FA*> row1_FAs_;
	std::vector<FA*> row2_FAs_;
	std::vector<FA*> row3_FAs_;

	std::vector<HA*> row0_HAs_;
	std::vector<HA*> row1_HAs_;
	std::vector<HA*> row2_HAs_;
	std::vector<HA*> row3_HAs_;

	RippleAdder* ripple_adder_;
	
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

	~Rad4Multiplier12b() { delete ripple_adder_;}

	int width() const { return numOutputs() / 2; }
};

#endif
