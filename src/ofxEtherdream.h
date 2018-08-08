//
//  ofxEtherdream.h
//  ofxILDA
//
//  Created by Daito Manabe + Yusuke Tomoto (Rhizomatiks)
//  Mods by Memo Akten
//
//

#define USE_J4C 1

#include "ofMain.h"

#if !USE_J4C
#include "etherdream.h" 
#else
#include "j4cDAC.h"
#include "dac.h"
#define etherdream dac_t
#endif

#include "ofxIldaFrame.h"

class ofxEtherdream : public ofThread {
public:
    ofxEtherdream():state(ETHERDREAM_NOTFOUND), bAutoConnect(false) {}
    
    ~ofxEtherdream() { kill(); }
    
    bool stateIsFound();
    
    void kill() 
	{
        clear();
        stop();
        if(stateIsFound()) {
#if USE_J4C
			EtherDreamStop(&idEtherdreamConnection);
			EtherDreamCloseDevice(&idEtherdreamConnection);
#else
            etherdream_stop(device);
            etherdream_disconnect(device);
#endif
        }
    }
    
    void setup(bool bStartThread = true, int idEtherdream = 0);
    virtual void threadedFunction();
    
    
    // check if the device has shutdown (weird bug in etherdream driver) and reconnect if nessecary
    bool checkConnection(bool bForceReconnect = true);
    
    void clear();
    void start();
    void stop();

    void addPoints(const vector<ofxIlda::Point>& _points);
    void addPoints(const ofxIlda::Frame &ildaFrame);
    
    void setPoints(const vector<ofxIlda::Point>& _points);
    void setPoints(const ofxIlda::Frame &ildaFrame);
    
    void send();
    
    void setPPS(int i);
    int getPPS() const;
    
    void setWaitBeforeSend(bool b);
    bool getWaitBeforeSend() const;
    
private:
    bool init();
    
private:
    enum {
        ETHERDREAM_NOTFOUND = 0,
        ETHERDREAM_FOUND
    } state;
    
    int pps;
    bool bWaitBeforeSend;
    bool bAutoConnect;
    
#if !USE_J4C
    struct etherdream *device;
#endif

    vector<ofxIlda::Point> points;
    
    int idEtherdreamConnection;
};