#include "router.h"

void Router::route(){
	lEnable_mux = enable_bitstream.read()[4];
	nEnable_mux = enable_bitstream.read()[3];
	sEnable_mux = enable_bitstream.read()[2];
	eEnable_mux = enable_bitstream.read()[1];
	wEnable_mux = enable_bitstream.read()[0];
	
	lSelector_mux = switch_bitstream.read().range(9, 8);
	nSelector_mux = switch_bitstream.read().range(7, 6);
	sSelector_mux = switch_bitstream.read().range(5, 4);
	eSelector_mux = switch_bitstream.read().range(3, 2);
	wSelector_mux = switch_bitstream.read().range(1, 0);
}