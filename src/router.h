#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <systemc.h>
#include "constants.h"
#include "mux.h"

SC_MODULE(Router){
	/*  00     00     00     00    00
	 * Local  North  South  East  West
	 */
	sc_in< sc_uint<ROUTERS_SWITCHERS> > switch_bitstream;

	/*   0      0      0      0     0
	 * Local  North  South  East  West
	 */
	sc_in< sc_uint<ROUTERS_ENABLES> > enable_bitstream;

	sc_in<bool> clk;

	// Router outputs
	sc_out< sc_uint<CHANNEL_WIDITH> > local_out;
	sc_out< sc_uint<CHANNEL_WIDITH> > north_out;
	sc_out< sc_uint<CHANNEL_WIDITH> > south_out;
	sc_out< sc_uint<CHANNEL_WIDITH> > east_out;
	sc_out< sc_uint<CHANNEL_WIDITH> > west_out;

	// Router modules
	sc_buffer< sc_uint<CHANNEL_WIDITH> > local_in;
	sc_buffer< sc_uint<CHANNEL_WIDITH> > north_in;
	sc_buffer< sc_uint<CHANNEL_WIDITH> > south_in;
	sc_buffer< sc_uint<CHANNEL_WIDITH> > east_in;
	sc_buffer< sc_uint<CHANNEL_WIDITH> > west_in;

	Mux *local_mux;
	Mux *north_mux;
	Mux *south_mux;
	Mux *east_mux;
	Mux *west_mux;

	// Multiplexers signals
	sc_signal< sc_uint<2> > lSelector_mux;
	sc_signal< sc_uint<2> > nSelector_mux;
	sc_signal< sc_uint<2> > sSelector_mux;
	sc_signal< sc_uint<2> > eSelector_mux;
	sc_signal< sc_uint<2> > wSelector_mux;

	sc_signal<bool> lEnable_mux;
	sc_signal<bool> nEnable_mux;
	sc_signal<bool> sEnable_mux;
	sc_signal<bool> eEnable_mux;
	sc_signal<bool> wEnable_mux;

	// Behaviors
	void route();

	SC_CTOR(Router):
		local_in("local_in"),
		north_in("north_in"),
		south_in("south_in"),
		east_in("east_in"),
		west_in("west_in"),
		switch_bitstream("switch_bitstream"),
		enable_bitstream("enable_bitstream"),
		clk("router_clock"),
		local_out("local_out"),
		north_out("north_out"),
		south_out("south_out"),
		east_out("east_out"),
		west_out("west_out")
	{
		local_mux = new Mux("local_mux");
		north_mux = new Mux("north_mux");
		south_mux = new Mux("south_mux");
		east_mux = new Mux("east_mux");
		west_mux = new Mux("west_mux");

		// Binds for the core
		local_mux->in_0(north_in);
		local_mux->in_1(south_in);
		local_mux->in_2(east_in);
		local_mux->in_3(west_in);
		local_mux->output(local_out);
		local_mux->selector(lSelector_mux);
		local_mux->enable(lEnable_mux);
		local_mux->clk(clk);

		// Binds for the north router
		north_mux->in_0(local_in);
		north_mux->in_1(south_in);
		north_mux->in_2(east_in);
		north_mux->in_3(west_in);
		north_mux->output(north_out);
		north_mux->selector(nSelector_mux);
		north_mux->enable(nEnable_mux);
		north_mux->clk(clk);

		// Binds for the south router
		south_mux->in_0(north_in);
		south_mux->in_1(local_in);
		south_mux->in_2(east_in);
		south_mux->in_3(west_in);
		south_mux->output(south_out);
		south_mux->selector(sSelector_mux);
		south_mux->enable(sEnable_mux);
		south_mux->clk(clk);

		// Binds for the east router
		east_mux->in_0(north_in);
		east_mux->in_1(south_in);
		east_mux->in_2(local_in);
		east_mux->in_3(west_in);
		east_mux->output(east_out);
		east_mux->selector(eSelector_mux);
		east_mux->enable(eEnable_mux);
		east_mux->clk(clk);

		// Binds for the west router
		west_mux->in_0(north_in);
		west_mux->in_1(south_in);
		west_mux->in_2(east_in);
		west_mux->in_3(local_in);
		west_mux->output(west_out);
		west_mux->selector(wSelector_mux);
		west_mux->enable(wEnable_mux);
		west_mux->clk(clk);

		SC_METHOD(route);
		sensitive << switch_bitstream;
		sensitive << enable_bitstream;
	}
};

#endif