#include "ofxEtherdream.h"

#include <chrono>
#include <thread>

//--------------------------------------------------------------
void ofxEtherdream::setup(bool bStartThread, int idEtherdream) {

    idEtherdreamConnection = idEtherdream;
    
#if USE_J4C
#else
    etherdream_lib_start(); 
#endif

    setPPS(30000);
    setWaitBeforeSend(true);
    
	/* Sleep for a bit over a second, to ensure that we see broadcasts
	 * from all available DACs. */
	std::this_thread::sleep_for(std::chrono::microseconds(1000000));
	//usleep(1000000);
    
    init();
#if USE_J4C
	if (bStartThread) start();
#else
    if(bStartThread) start();
#endif
}


//--------------------------------------------------------------
bool ofxEtherdream::stateIsFound() {
    return state == ETHERDREAM_FOUND;
}

//--------------------------------------------------------------
bool ofxEtherdream::checkConnection(bool bForceReconnect) {

#if USE_J4C
#else
	if (device->state == ST_SHUTDOWN || device->state == ST_BROKEN || device->state == ST_DISCONNECTED) {

		if (bForceReconnect) {
			kill();
			setup(true, idEtherdreamConnection);
		}

		return false;
	}
#endif
	return true;
}

//--------------------------------------------------------------
bool ofxEtherdream::init() {
#if USE_J4C
	int device_num = EtherDreamGetCardNum();
#else
    int device_num = etherdream_dac_count();
#endif
	if (!device_num || idEtherdreamConnection>device_num) {
		ofLogWarning() << "ofxEtherdream::init - No DACs found";
		return false;
	}
    
#if USE_J4C
#else
	for (int i=0; i<device_num; i++) {
		ofLogNotice() << "ofxEtherdream::init - " << i << " Ether Dream " << etherdream_get_id(etherdream_get(i));
    }
    device = etherdream_get(idEtherdreamConnection);
#endif

    ofLogNotice() << "ofxEtherdream::init - Connecting...";
#if USE_J4C
    if (EtherDreamOpenDevice(&idEtherdreamConnection) == true)
#else
	if (etherdream_connect(device) == 0)
#endif
	{
		ofLogNotice() << "ofxEtherdream::init - done";

		state = ETHERDREAM_FOUND;

		return true;
	}
	else
		return false;
    
}

//--------------------------------------------------------------
void ofxEtherdream::threadedFunction() {
    while (isThreadRunning()) {
		//printf("state: %i\n", state);
        switch (state) {
            case ETHERDREAM_NOTFOUND:
                if(bAutoConnect) init();
                break;
                
            case ETHERDREAM_FOUND:
                if(lock()) {
                    send();
                    unlock();
                }
                break;
        }
    }
}

//--------------------------------------------------------------
void ofxEtherdream::start() {
    startThread(true);//, false);  // TODO: blocking or nonblocking?
}

//--------------------------------------------------------------
void ofxEtherdream::stop() {
    stopThread();
}

//--------------------------------------------------------------
void ofxEtherdream::send() {
#if USE_J4C
	//	printf("start send\n");
	if (!stateIsFound() || points.empty()) return;
	//    printf("wait for ready\n");

	if (EtherDreamGetStatus(&idEtherdreamConnection) != GET_STATUS_READY)
	{
		return;
	}
	int byte = sizeof(EAD_Pnt_s) * points.size();
	//int res = EzAudDacWriteFrameNR(&idEtherdreamConnection, (EAD_Pnt_s*)points.data(), byte, pps, 1);
	int res = EtherDreamWriteFrame(&idEtherdreamConnection, (EAD_Pnt_s*)points.data(), byte, pps, 1);
#else
//	printf("start send\n");
    if(!stateIsFound() || points.empty()) return;
//    printf("wait for ready\n");
    if(bWaitBeforeSend) etherdream_wait_for_ready(device);
    else if(!etherdream_is_ready(device)) return;
//    printf("try to write points: %i\n", points.size());
    // DODGY HACK: casting ofxIlda::Point* to etherdream_point*
    int res = etherdream_write(device, (etherdream_point*)points.data(), points.size(), pps, -1);
#endif
//	printf("%i\n", res);
    if (res != 0) {
        ofLogVerbose() << "ofxEtherdream::write " << res;
    }
    points.clear();
}


//--------------------------------------------------------------
void ofxEtherdream::clear() {
    if(lock()) {
        points.clear();
        unlock();
    }
}

//--------------------------------------------------------------
void ofxEtherdream::addPoints(const vector<ofxIlda::Point>& _points) {
    if(lock()) {
        if(!_points.empty()) {
            points.insert(points.end(), _points.begin(), _points.end());
        }
        unlock();
    }
}


//--------------------------------------------------------------
void ofxEtherdream::addPoints(const ofxIlda::Frame &ildaFrame) {
    addPoints(ildaFrame.getPoints());
}


//--------------------------------------------------------------
void ofxEtherdream::setPoints(const vector<ofxIlda::Point>& _points) {
    if(lock()) {
        points = _points;
        unlock();
    }
}


//--------------------------------------------------------------
void ofxEtherdream::setPoints(const ofxIlda::Frame &ildaFrame) {
    setPoints(ildaFrame.getPoints());
}

//--------------------------------------------------------------
void ofxEtherdream::setWaitBeforeSend(bool b) {
    if(lock()) {
        bWaitBeforeSend = b;
        unlock();
    }
}

//--------------------------------------------------------------
bool ofxEtherdream::getWaitBeforeSend() const {
    return bWaitBeforeSend;
}


//--------------------------------------------------------------
void ofxEtherdream::setPPS(int i) {
    if(lock()) {
        pps = i;
        unlock();
    }
}

//--------------------------------------------------------------
int ofxEtherdream::getPPS() const {
    return pps;
}
