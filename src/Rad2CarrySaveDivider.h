#ifndef Rad2CarrySaveDivider_H_
#define Rad2CarrySaveDivider_H_

#include "SystemModule.h"
#include "CLK.h"
#include "QSel.h"
#include "QConv.h"
#include "MUX.h"
#include "MetaGate.h"
#include "REG.h"
#include "SaveAdder.h"
#include "InputCtrl.h"

// Implements the Radix-2 divider with carry-save residual as a system module
class Rad2CarrySaveDivider : public SystemModule
{
	private:
		int n_;
		delay_t T_;
		void initSubmodules()
		{
			clk_ = new CLK(n_+2, T_/2, T_/2);
			submodule(clk_);

			reg_d_ = new REG(n_);
			submodule(reg_d_);

			reg_wc_ = new REG(n_);
			submodule(reg_wc_);

			reg_ws_ = new REG(n_);
			submodule(reg_ws_);

			mux_divisor_ = new MUX(n_);
			submodule(mux_divisor_);

			mux_wc_ = new MUX(n_);
			submodule(mux_wc_);

			mux_ws_ = new MUX(n_);
			submodule(mux_ws_);

			zeroAnd_ = new MetaGate<AND>(2, n_);
			submodule(zeroAnd_);

			qsel_ = new QSel();
			submodule(qsel_);

			saveAdder_ = new SaveAdder(n_);
			submodule(saveAdder_);

			qconv_ = new QConv(n_);
			submodule(qconv_);

			input_ctrl_ = new InputCtrl(T_);
			submodule(input_ctrl_);
		}

		void attachSubmodules(int d)
		{
			// attach inputs to first stage
			IN("d") >> (*reg_d_)("D");
			IN("WC") >> (*mux_wc_)("A");
			IN("WS") >> (*mux_ws_)("A");

			// attach first stage outputs
			(*reg_d_)("Q") >> (*mux_divisor_)("A");
			(*mux_divisor_)("B") <= -1*d;
			(*mux_wc_)("Z") >> (*reg_wc_)("D");
			(*mux_ws_)("Z") >> (*reg_ws_)("D");

			// attach second qsel inputs
			(*reg_wc_)("Q", 3, 0) >> (*qsel_)("WC");
			(*reg_ws_)("Q", 3, 0) >> (*qsel_)("WS");

			// attach 3:2 adder inputs
			(*reg_wc_)("Q") >> (*saveAdder_)("X");
			(*reg_ws_)("Q") >> (*saveAdder_)("Y");
			(*mux_divisor_)("Z") >> (*zeroAnd_)("0");
			(*zeroAnd_)("Z") >> (*saveAdder_)("Ci");

			// attach selection function outputs
			(*qsel_)("qs") >> (*mux_divisor_)("SEL");

			for (int i = 0; i < n_; i++)
			{
				(*qsel_)("qm") >> (*zeroAnd_)("1", i);
			}
			(*qsel_)("qs") >> (*qconv_)("q", 1);
			(*qsel_)("qm") >> (*qconv_)("q", 0);

			// attach save adder outputs
			(*saveAdder_)("S") >> (*qconv_)("WS");
			(*saveAdder_)("Co") >> (*qconv_)("WC");
			(*saveAdder_)("S") >> (*mux_ws_)("B");
			cyclic_connection("S", saveAdder_, "B", mux_ws_);
			(*saveAdder_)("Co") >> (*mux_wc_)("B");
			cyclic_connection("Co", saveAdder_, "B", mux_wc_);

			// attach control to mux_ws and mux_wc
			(*input_ctrl_)("CTRL") >> (*mux_wc_)("SEL");
			(*input_ctrl_)("CTRL") >> (*mux_ws_)("SEL");

			// attach system output
			OUT("Q") << (*qconv_)("QN");

			// attach clock to submodules that need clock signals
			*clk_ >> (*reg_d_)("CLK");
			*clk_ >> (*reg_wc_)("CLK");
			*clk_ >> (*reg_ws_)("CLK");
			*clk_ >> (*qconv_)("CLK");
			*clk_ >> (*input_ctrl_)("CLK");

			// clk_->propagate();

		}

	public:
		CLK* clk_;
		REG* reg_d_;
		REG* reg_wc_;
		REG* reg_ws_;
		QSel* qsel_;
		MUX* mux_divisor_;
		MUX* mux_wc_;
		MUX* mux_ws_;
		MetaGate<AND>* zeroAnd_;
		SaveAdder* saveAdder_;
		QConv* qconv_;
		InputCtrl* input_ctrl_;

		Rad2CarrySaveDivider(int N, int d, delay_t clk_period)
		{
			n_ = N;
			T_ = clk_period;
			addInput("d", N);
			addInput("WC", N);
			addInput("WS", N);
			addOutput("Q", N);

			std::stringstream ss;
			ss << "Rad2CarrySaveDivider<" << N << ">";
			classname_ = ss.str();
			
			initSubmodules();
			
			attachSubmodules(d);
			
		}

		~Rad2CarrySaveDivider() { delete saveAdder_; delete clk_; delete reg_d_; delete reg_wc_;}
};

#endif
