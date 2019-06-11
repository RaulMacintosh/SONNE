#include "core.h"

void Core::requestRoute(){
	finish.write(0);
	requested_coreX.write(-1);
	requested_coreY.write(-1);
	while(true){
		wait();
		if(start.read() == 1){
			if(!idleCycles.empty()){
				if(cyclesElapsed == 0){
					if(idleCycles.front() <= (sc_time_stamp()).value()/1000){
						cout << name() << " Requesting route at: " << sc_time_stamp() << endl;
						requested_coreX.write(get<0>(destinyCores.front()));
						requested_coreY.write(get<1>(destinyCores.front()));

						while(channel_available.read() != 1){
							wait();
						}

						cout << name() << " Request Accepted at:" << sc_time_stamp() << endl;
						requested_coreX.write(-1);
						requested_coreY.write(-1);

						sendPackages();

						idleCycles.erase(idleCycles.begin());

						while(channel_available.read() == 1){
							cyclesElapsed = sc_time_stamp().value()/1000;
							if(idleCycles.size() == 0){
								break;
							}
							wait();
						}
						cyclesElapsed = 0;
					}
				}
			}else{
				finish.write(1);
			}
		}

		if(last_pckgReceived == 1){
			last_pckgReceived.write(false);
		}
	}
}

void Core::sendPackages(){
	for(int i = 0; i < numPckgs.front(); i++){
		wait();
		data_out.write((numPckgs.front() - i));
	}

	numPckgs.erase(numPckgs.begin());
	destinyCores.erase(destinyCores.begin());
}

void Core::receivePackages(){
	last_pckgReceived.write(false);
	if(data_in.read() != 4294967295 && data_in.read() != 0){
	}

	if(data_in.read() == 2){
		lastPackageIncoming = true;
	}

	if(data_in.read() == 1 && lastPackageIncoming){
		last_pckgReceived.write(true);
		lastPackageIncoming = false;
	}
}
















