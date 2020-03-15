#ifndef Rad2CarrySaveDivider_H_
#define Rad2CarrySaveDivider_H_

#include "SystemModule.h"
#include "CLK.h"
#include "QSel.h"
#include "QConv.h"
#include "MUX4to1.h"
#include "REG.h"
#include "SaveAdder.h"

// Implements the Radix-2 divider with carry-save residual as a system module
class Rad2CarrySaveDivider : public SystemModule
{
	private:
		int n_;
		void initSubmodules()
		{

		}

		void attachSubmodules()
		{

		}

	public:
		CLK* clk_;
		std::vector<REG*> registers_;
		QSel* qsel_;
		MUX4to1* mux4to1_;
		SaveAdder* saveAdder_;
		QConv* qconv_;

}
