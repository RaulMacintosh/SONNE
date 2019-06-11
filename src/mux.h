#ifndef _MUX_H_
#define _MUX_H_

#include <systemc.h>
#include "constants.h"

SC_MODULE(Mux){
	// Mux inputs
	sc_in<bool> clk;
	sc_in< sc_uint<CHANNEL_WIDITH> > in_0;
	sc_in< sc_uint<CHANNEL_WIDITH> > in_1;
	sc_in< sc_uint<CHANNEL_WIDITH> > in_2;
	sc_in< sc_uint<CHANNEL_WIDITH> > in_3;

	sc_in< sc_uint<2> > selector;
	sc_in<bool> enable;

	// Mux outpus
	sc_out< sc_uint<CHANNEL_WIDITH> > output;

	// Behaviors
	void switchGate();

	SC_CTOR(Mux):
		clk("mux_clock"),
		in_0("in_0"),
		in_1("in_1"),
		in_2("in_2"),
		in_3("in_3"),
		selector("selector"),
		enable("enable"),
		output("output")
	{
		SC_CTHREAD(switchGate, clk.pos());
	}
};

#endif