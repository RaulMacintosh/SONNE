#include "manager.h"

void Manager::routeRequest(){
	while(true){
		wait();
		for(int i = 0; i < N; i++){
			for(int j = 0; j < M; j++){
				if(route_requestsX[i][j].read() != -1 && route_requestsY[i][j].read() != -1){
					bool isOnStack = false;

					for(int k = 0; k < requests_queue.size(); k++){
						tuple<int, int> currentRequest(i, j);
						if(requests_queue[k] == currentRequest){
							isOnStack = true;
							break;
						}
					}

					if(!isOnStack){
						requests_queue.push_back(make_tuple(i, j));
					}
				}
			}
		}
	}
}

void Manager::generateRoutes(){
	while(true){
		wait();
		int request = -1;
		if(requests_queue.size() > 0){
			if(routing_algorithm.read()){
				int srcX;
				int srcY;
				int destX;
				int destY;
				do{
					request += 1;
					if(request >= requests_queue.size()){
						request = 0;
					}
					srcX = get<0>(requests_queue[request]);
					srcY = get<1>(requests_queue[request]);
					destX = route_requestsX[srcX][srcY].read();
					destY = route_requestsY[srcX][srcY].read();
					wait();
				}while(!xy(srcX, srcY, destX, destY));
			}else{
				int srcX;
				int srcY;
				int destX;
				int destY;
				do{
					request += 1;
					if(request >= requests_queue.size()){
						request = 0;
					}
					srcX = get<0>(requests_queue[request]);
					srcY = get<1>(requests_queue[request]);
					destX = route_requestsX[srcX][srcY].read();
					destY = route_requestsY[srcX][srcY].read();
					wait();
				}while(!dijkstra(srcX, srcY, destX, destY));
			}
		}

		if(requests_queue.size() > 0){
			for(int i = 0; i < paths.size(); i++){
				if(paths[i][0] == requests_queue[request]){
					if(available_channels[get<0>(paths[i][0])][get<1>(paths[i][0])] == 1){
						// wait();
						requests_queue.erase(requests_queue.begin()+request);
					}
				}
			}
		}
	}
}

bool Manager::xy(int srcX, int srcY, int destX, int destY){
	for(int i = 0; i < paths.size(); i++){
		tuple<int, int> destCore(destX, destY);
		vector< tuple<int, int> > path = paths[i];
		if(path[(path.size() - 1)] == destCore){
			return false;
		}
	}

	vector< tuple<int, int> > path;
	path.push_back(make_tuple(srcX, srcY));

	while(srcX < destX){
		if(network[(srcX + srcY * N)][(srcX + 1 + srcY * N)] == 0){
			return false;
		}

		path.push_back(make_tuple((srcX + 1), srcY));
		srcX++;
	}

	while(srcX > destX){
		if(network[(srcX + srcY * N)][(srcX - 1 + srcY * N)] == 0){
			return false;
		}

		path.push_back(make_tuple((srcX - 1), srcY));
		srcX--;
	}

	while(srcY < destY){
		if(network[(srcX + srcY * N)][(srcX + (srcY + 1) * N)] == 0){
			return false;
		}

		path.push_back(make_tuple(srcX, (srcY + 1)));
		srcY++;
	}

	while(srcY > destY){
		if(network[(srcX + srcY * N)][(srcX + (srcY - 1) * N)] == 0){
			return false;
		}

		path.push_back(make_tuple(srcX, (srcY - 1)));
		srcY--;
	}

	for(int i = 0; i < path.size() - 1; i++){
		int xValue = get<0>(path[i]) + get<1>(path[i]) * N;
		int yValue = get<0>(path[i+1]) + get<1>(path[i+1]) * N;
		network[xValue][yValue] = 0;
	}

	paths.push_back(path);
	enableRoutes(path);
	return true;
}

bool Manager::dijkstra(int srcX, int srcY, int destX, int destY){
	for(int i = 0; i < paths.size(); i++){
		tuple<int, int> destCore(destX, destY);
		tuple<int, int> srcCore(srcX, srcY);

		vector< tuple<int, int> > path = paths[i];
		if(path[(path.size() - 1)] == destCore || path[0] == srcCore){
			return false;
		}
	}

	int d[N*M]; // Distance from s to d[v]
	tuple<int, int> pi[N*M]; // Origin vertex to reach v
	vector<int> q; // Set of verteces which do not have the shortest distance to s

	for(int i = 0; i < N*M; i++){
		d[i] = INT_MAX;
		pi[i] = make_tuple(-1, -1);
	}
	
	d[(srcX * N + srcY)] = 0;

	for(int i = 0; i < N*M; i++){
		if(network[(srcX * N + srcY)][i] > 0){
			d[i] = network[(srcX * N + srcY)][i];
			pi[i] = make_tuple(srcX, srcY);
		}
	}

	for(int i = 0; i < N*M; i++){
		if(d[i] != 0){
			q.push_back(i);
		}
	}

	while(!q.empty()){
		int u = q.front();
		int vertexIndex = 0;
		for(int i = 0; i < q.size(); i++){
			if(d[q[i]] < d[u]){
				u = q[i];
				vertexIndex = i;
			}
		}

		q.erase(q.begin() + vertexIndex);

		for(int i = 0; i < N*M; i++){
			if(network[u][i] > 0){
				if(d[i] > (d[u] + network[u][i])){
					d[i] = d[u] + network[u][i];
					pi[i] = make_tuple((u/N), (u%M));
				}
			}
		}
	}

	int vertex = (destX * N + destY);
	vector< tuple<int, int> > reversePath;
	vector< tuple<int, int> > path;

	if(d[vertex] == INT_MAX){
		return false;
	}

	tuple<int, int> destiny(destX, destY);
	reversePath.push_back(destiny);
	if(destX < 0 || destY < 0){
		return false;
	}

	int loopDetector = -1;

	do{
		if(get<0>(pi[vertex]) < 0 || get<1>(pi[vertex]) < 0){
			return false;
		}

		tuple<int, int> node(get<0>(pi[vertex]), get<1>(pi[vertex]));
		reversePath.push_back(node);
		int newX = get<0>(pi[vertex]) * N;
		int newY = get<1>(pi[vertex]);
		vertex = newX + newY;
		loopDetector++;
		if(loopDetector > N*M){
			return false;
		}
	}while(vertex != (srcX * N + srcY));

	for(int i = (reversePath.size() - 1); i >= 0; i--){
		int currX = get<0>(reversePath[i]);
		int currY = get<1>(reversePath[i]);

		if(currX < 0 || currY < 0){
			return false;
		}

		path.push_back(reversePath[i]);
	}

	for(int i = 0; i < path.size() - 1; i++){
		int xValue = get<0>(path[i]) * N + get<1>(path[i]);
		int yValue = get<0>(path[i+1]) * N + get<1>(path[i+1]);
		network[xValue][yValue] = 0;
	}

	paths.push_back(path);
	enableRoutes(path);
	return true;
}

void Manager::enableRoutes(vector< tuple<int, int> > path){
	for(int j = 0; j < (path.size() - 1); j++){
		int currX = get<0>(path[j]);
		int currY = get<1>(path[j]);

		int nextX = get<0>(path[j+1]);
		int nextY = get<1>(path[j+1]);

		if(j == 0){
			if(nextX > currX){
				enables[currX][currY] = enables[currX][currY] | 4; // 00100
				switchers[currX][currY] = switchers[currX][currY] | 16; // 00 00 01 00 00
				
				swtBitsteam[currX][currY].write(switchers[currX][currY]);
				enBitstream[currX][currY].write(enables[currX][currY]);
			}else if(nextX < currX){
				enables[currX][currY] = enables[currX][currY] | 8; // 01000
				switchers[currX][currY] = switchers[currX][currY] | 0; // 00 00 00 00 00
				
				swtBitsteam[currX][currY].write(switchers[currX][currY]);
				enBitstream[currX][currY].write(enables[currX][currY]);
			}

			if(nextY > currY){
				enables[currX][currY] = enables[currX][currY] | 2; // 00010
				switchers[currX][currY] = switchers[currX][currY] | 8; // 00 00 00 10 00
				
				swtBitsteam[currX][currY].write(switchers[currX][currY]);
				enBitstream[currX][currY].write(enables[currX][currY]);
			}else if(nextY < currY){
				enables[currX][currY] = enables[currX][currY] | 1; // 00001
				switchers[currX][currY] = switchers[currX][currY] | 3; // 00 00 00 00 11
				
				swtBitsteam[currX][currY].write(switchers[currX][currY]);
				enBitstream[currX][currY].write(enables[currX][currY]);
			}
		}else{
			int prevX = get<0>(path[j-1]);
			int prevY = get<1>(path[j-1]);
			if(nextX > currX){
				enables[currX][currY] = enables[currX][currY] | 4; // 00100
				enBitstream[currX][currY].write(enables[currX][currY]);

				if(prevX < currX){
					switchers[currX][currY] = switchers[currX][currY] | 0; // 00 00 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY > currY){
					switchers[currX][currY] = switchers[currX][currY] | 32; // 00 00 10 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY < currY){
					switchers[currX][currY] = switchers[currX][currY] | 48; // 00 00 11 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}
			}else if(nextX < currX){
				enables[currX][currY] = enables[currX][currY] | 8; // 01000
				enBitstream[currX][currY].write(enables[currX][currY]);

				if(prevX > currX){
					switchers[currX][currY] = switchers[currX][currY] | 64; // 00 01 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY > currY){
					switchers[currX][currY] = switchers[currX][currY] | 128; // 00 10 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY < currY){
					switchers[currX][currY] = switchers[currX][currY] | 192; // 00 11 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}
			}

			if(nextY > currY){
				enables[currX][currY] = enables[currX][currY] | 2; // 00010
				enBitstream[currX][currY].write(enables[currX][currY]);

				if(prevX < currX){
					switchers[currX][currY] = switchers[currX][currY] | 0; // 00 00 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevX > currX){
					switchers[currX][currY] = switchers[currX][currY] | 4; // 00 00 00 01 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY < currY){
					switchers[currX][currY] = switchers[currX][currY] | 12; // 00 00 00 11 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}
			}else if(nextY < currY){
				enables[currX][currY] = enables[currX][currY] | 1; // 00001
				enBitstream[currX][currY].write(enables[currX][currY]);

				if(prevX < currX){
					switchers[currX][currY] = switchers[currX][currY] | 0; // 00 00 00 00 00
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevX > currX){
					switchers[currX][currY] = switchers[currX][currY] | 1; // 00 00 00 00 01
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}else if(prevY > currY){
					switchers[currX][currY] = switchers[currX][currY] | 2; // 00 00 00 00 10
					swtBitsteam[currX][currY].write(switchers[currX][currY]);
				}
			}
		}
	}

	int currX = get<0>(path[path.size() - 1]);
	int currY = get<1>(path[path.size() - 1]);
	int prevX = get<0>(path[path.size() - 2]);
	int prevY = get<1>(path[path.size() - 2]);

	enables[currX][currY] = enables[currX][currY] | 16; // 10000
	enBitstream[currX][currY].write(enables[currX][currY]);

	if(prevX < currX){
		switchers[currX][currY] = switchers[currX][currY] | 0; // 00 00 00 00 00
		swtBitsteam[currX][currY].write(switchers[currX][currY]);
	}else if(prevX > currX){
		switchers[currX][currY] = switchers[currX][currY] | 256; // 01 00 00 00 00
		swtBitsteam[currX][currY].write(switchers[currX][currY]);
	}else if(prevY < currY){
		switchers[currX][currY] = switchers[currX][currY] | 768; // 11 00 00 00 00
		swtBitsteam[currX][currY].write(switchers[currX][currY]);
	}else if(prevY > currY){
		switchers[currX][currY] = switchers[currX][currY] | 512; // 10 00 00 00 00
		swtBitsteam[currX][currY].write(switchers[currX][currY]);
	}

	available_channels[get<0>(path[0])][get<1>(path[0])].write(true);
}

void Manager::checkEndedCommunications(){
	while(true){
		wait();

		for(int i = 0; i < N; i++){
			for(int j = 0; j < M; j++){
				if(ended_communications[i][j].read() == 1){
					for(int k = 0; k < paths.size(); k++){
						tuple<int, int> destCore(i, j);
						vector< tuple<int, int> > path = paths[k];
						if(path[path.size() - 1] == destCore){
							for(int l = 0; l < path.size() - 1; l++){
								int xValue;
								int yValue;

								if(routing_algorithm.read()){
									xValue = get<0>(path[l]) + get<1>(path[l]) * N;
									yValue = get<0>(path[l+1]) + get<1>(path[l+1]) * N;
								}else{
									xValue = get<0>(path[l]) * N + get<1>(path[l]);
									yValue = get<0>(path[l+1]) * N + get<1>(path[l+1]);
								}

								network[xValue][yValue] = 1;

								int currX = get<0>(path[l]);
								int currY = get<1>(path[l]);

								int nextX = get<0>(path[l+1]);
								int nextY = get<1>(path[l+1]);

								if(l == 0){
									if(nextX > currX){
										enables[currX][currY] = enables[currX][currY] & 27; // 11011
										switchers[currX][currY] = switchers[currX][currY] & 1007; // 11 11 10 11 11
										
										swtBitsteam[currX][currY].write(switchers[currX][currY]);
										enBitstream[currX][currY].write(enables[currX][currY]);
									}else if(nextX < currX){
										enables[currX][currY] = enables[currX][currY] & 23; // 10111
										switchers[currX][currY] = switchers[currX][currY] & 1023; // 11 11 11 11 11
										
										swtBitsteam[currX][currY].write(switchers[currX][currY]);
										enBitstream[currX][currY].write(enables[currX][currY]);
									}

									if(nextY > currY){
										enables[currX][currY] = enables[currX][currY] & 29; // 11101
										switchers[currX][currY] = switchers[currX][currY] & 1015; // 11 11 11 01 11
										
										swtBitsteam[currX][currY].write(switchers[currX][currY]);
										enBitstream[currX][currY].write(enables[currX][currY]);
									}else if(nextY < currY){
										enables[currX][currY] = enables[currX][currY] & 30; // 11110
										switchers[currX][currY] = switchers[currX][currY] & 1020; // 11 11 11 11 00
										
										swtBitsteam[currX][currY].write(switchers[currX][currY]);
										enBitstream[currX][currY].write(enables[currX][currY]);
									}
								}else{
									int prevX = get<0>(path[l-1]);
									int prevY = get<1>(path[l-1]);
									if(nextX > currX){
										enables[currX][currY] = enables[currX][currY] & 27; // 11011
										enBitstream[currX][currY].write(enables[currX][currY]);

										if(prevX < currX){
											switchers[currX][currY] = switchers[currX][currY] & 1023; // 11 11 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY > currY){
											switchers[currX][currY] = switchers[currX][currY] & 991; // 11 11 01 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY < currY){
											switchers[currX][currY] = switchers[currX][currY] & 975; // 11 11 00 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}
									}else if(nextX < currX){
										enables[currX][currY] = enables[currX][currY] & 23; // 10111
										enBitstream[currX][currY].write(enables[currX][currY]);

										if(prevX > currX){
											switchers[currX][currY] = switchers[currX][currY] & 959; // 11 10 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY > currY){
											switchers[currX][currY] = switchers[currX][currY] & 895; // 11 01 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY < currY){
											switchers[currX][currY] = switchers[currX][currY] & 831; // 11 00 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}
									}

									if(nextY > currY){
										enables[currX][currY] = enables[currX][currY] & 29; // 11101
										enBitstream[currX][currY].write(enables[currX][currY]);

										if(prevX < currX){
											switchers[currX][currY] = switchers[currX][currY] & 1023; // 11 11 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevX > currX){
											switchers[currX][currY] = switchers[currX][currY] & 1019; // 11 11 11 10 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY < currY){
											switchers[currX][currY] = switchers[currX][currY] & 1011; // 11 11 11 00 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}
									}else if(nextY < currY){
										enables[currX][currY] = enables[currX][currY] & 30; // 11110
										enBitstream[currX][currY].write(enables[currX][currY]);

										if(prevX < currX){
											switchers[currX][currY] = switchers[currX][currY] & 1023; // 11 11 11 11 11
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevX > currX){
											switchers[currX][currY] = switchers[currX][currY] & 1022; // 11 11 11 11 10
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}else if(prevY > currY){
											switchers[currX][currY] = switchers[currX][currY] & 1021; // 11 11 11 11 01
											swtBitsteam[currX][currY].write(switchers[currX][currY]);
										}
									}
								}
							}

							int currX = get<0>(path[path.size() - 1]);
							int currY = get<1>(path[path.size() - 1]);
							int prevX = get<0>(path[path.size() - 2]);
							int prevY = get<1>(path[path.size() - 2]);

							enables[currX][currY] = enables[currX][currY] & 15; // 01111
							enBitstream[currX][currY].write(enables[currX][currY]);

							if(prevX < currX){
								switchers[currX][currY] = switchers[currX][currY] & 1023; // 11 11 11 11 11
								swtBitsteam[currX][currY].write(switchers[currX][currY]);
							}else if(prevX > currX){
								switchers[currX][currY] = switchers[currX][currY] & 767; // 10 11 11 11 11
								swtBitsteam[currX][currY].write(switchers[currX][currY]);
							}else if(prevY < currY){
								switchers[currX][currY] = switchers[currX][currY] & 255; // 00 11 11 11 11
								swtBitsteam[currX][currY].write(switchers[currX][currY]);
							}else if(prevY > currY){
								switchers[currX][currY] = switchers[currX][currY] & 511; // 01 11 11 11 11
								swtBitsteam[currX][currY].write(switchers[currX][currY]);
							}

							available_channels[get<0>(path[0])][get<1>(path[0])].write(false);
							paths.erase(paths.begin()+k);
						}
					}
				}
			}
		}
	}
}

void Manager::checkEndSimulation(){
	while(true){
		wait();
		bool allDone = true;
		for(int i = 0; i < N; i++){
			for(int j = 0; j < M; j++){
				if(finished_threads[i][j].read() == 0){
					allDone = false;
				}
			}
		}

		if(allDone){
			cout << "Ending at: " << sc_time_stamp() << endl;
			sc_stop();
		}
	}
}

void Manager::printNetwork(){
	for(int i = 0; i < N*M; i++){
		for(int j = 0; j < N*M; j++){
			cout << network[i][j] << " ";
		}

		cout << endl;
	}
}















