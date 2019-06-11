#ifndef _MANAGER_H_
#define _MANAGER_H_

#include <systemc.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <limits>
#include "constants.h"

using namespace std;

SC_MODULE(Manager){
	// Manager inputs
	sc_in<bool> clk;
	sc_in<bool> routing_algorithm;
	sc_in<int> route_requestsX[N][M];
	sc_in<int> route_requestsY[N][M];
	sc_in<bool> ended_communications[N][M];
	sc_in<bool> finished_threads[N][M];

	// Manager outputs
	sc_out<bool> available_channels[N][M];
	sc_out< sc_uint<ROUTERS_SWITCHERS> > swtBitsteam[N][M];
	sc_out< sc_uint<ROUTERS_ENABLES> > enBitstream[N][M];

	// Behaviors
	void routeRequest();
	void generateRoutes();
	void checkEndedCommunications();
	void checkEndSimulation();

	// Graphs
	int network[N*M][N*M];
	vector< vector< tuple<int, int> > > paths;
	int switchers[N][M];
	int enables[N][M];

	// Requests queue
	vector< tuple<int, int> > requests_queue;

	bool dijkstra(int srcX, int srcY, int destX, int destY);
	bool xy(int srcX, int srcY, int destX, int destY);
	void enableRoutes(vector< tuple<int, int> > path);
	void printNetwork();

	SC_CTOR(Manager):
		clk("noc_clock"),
		routing_algorithm("routing_algorithm")
	{
		for(int i = 0; i < N; i++){
			for(int j = 0; j < M; j++){
				switchers[i][j] = 0;
				enables[i][j] = 0;
			}
		}

		for(int i = 0; i < N*M; i++){
			for(int j = 0; j < N*M; j++){
				network[i][j] = 0;

				if(i == j){
					network[i][j] = -1;
				}
			}
		}

		for(int i = 0; i < N*M; i++){
			for(int j = i + 1; j < N*M; j++){
				if(i < (N*M - N)){
					if((i+1) % N != 0){
						network[i][j] = 1;
						network[j][i] = 1;

						j += N - 1;

						network[i][j] = 1;
						network[j][i] = 1;

						break;
					}else{
						j += N - 1;

						network[i][j] = 1;
						network[j][i] = 1;
						break;
					}
				}else{
					if(i < (N*M - 1) && (i+1) % N != 0){
						network[i][j] = 1;
						network[j][i] = 1;
						break;
					}
				}
			}
		}

		SC_CTHREAD(routeRequest, clk.pos());
		SC_CTHREAD(generateRoutes, clk.pos());
		SC_CTHREAD(checkEndedCommunications, clk.pos());
		SC_CTHREAD(checkEndSimulation, clk.pos());
	}
};

#endif