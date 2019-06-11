#ifndef _CORE_H_
#define _CORE_H_

#include <systemc.h>
#include <vector>
#include <tuple>
#include "constants.h"

using namespace std;

SC_MODULE(Core){
	// Core inputs
	sc_in<bool> clk;
	sc_in<bool> start;
	sc_in<bool> channel_available;
	sc_in< sc_uint<CHANNEL_WIDITH> > data_in;

	// Core outputs
	sc_out<int> requested_coreX;
	sc_out<int> requested_coreY;
	sc_out< sc_uint<CHANNEL_WIDITH> > data_out;
	sc_out<bool> finish;
	sc_out<bool> last_pckgReceived;

	// Messages
	int cyclesElapsed;
	bool lastPackageIncoming;
	vector<int> idleCycles; // Waiting time (clock cycles) to request a new communication
	vector< tuple<int, int> > destinyCores; // List of destiny cores to send the packages
	vector<int> numPckgs; // Number of packages to be send in each communication

	// Behaviors
	void requestRoute();
	void sendPackages();
	void receivePackages();

	SC_CTOR(Core):
		clk("noc_clock"),
		start("start"),
		channel_available("channel_available"),
		data_in("data_in"),
		requested_coreX("requested_coreX"),
		requested_coreY("requested_coreY"),
		data_out("data_out"),
		finish("finish")
	{
		cyclesElapsed = 0;
		lastPackageIncoming = false;

		SC_CTHREAD(requestRoute, clk.pos());
		SC_METHOD(receivePackages);
		sensitive << data_in;
	}
};

#endif