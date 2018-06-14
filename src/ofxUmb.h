//
//  ofxUmb.hpp
//  boelgen
//
//  Created by Tobias Ebsen on 09/05/18.
//
//

#ifndef ofxUmb_h
#define ofxUmb_h

#include <ofMain.h>
#include "ofxUmbFrame.h"

#define UMB_RESPONSE_SHORT  50
#define UMB_RESPONSE_LONG   500

typedef enum {
    OK = 0,
    UNBEK_CMD = 16,
    UNGLTG_PARAM,
    UNGLTG_HEADER,
    UNGLTG_VERC,
    UNGLTG_PW,
    LESE_ERR = 32,
    SCHREIB_ERR,
    ZU_LANG,
    UNGLTG_ADRESS,
    UNGLTG_KANAL,
    UNGLTG_CMD,
    UNBEK_CAL_CMD,
    CAL_ERROR,
    BUSY,
    LOW_VOLTAGE,
    HW_ERROR
} umbStatus;

typedef enum {
    UNSIGNED_CHAR = 16,
    SIGNED_CHAR,
    UNSIGNED_SHORT,
    SIGNED_SHORT,
    UNSIGNED_LONG,
    SIGNED_LONG,
    FLOAT,
    DOUBLE
} umbType;

typedef enum {
    MWT_CURRENT = 16,
    MWT_MIN,
    MWT_MAX,
    MWT_AVG,
    MWT_SUM,
    MWT_VCT
} umbValue;

class ofxUmb {
public:
    
    ofxUmb();
    
    vector <ofSerialDeviceInfo> getDeviceList();
    bool connect(int device = 0);
    bool connect(string deviceName);
    void close();
    bool isConnected();
    void flush();
    
    void setReceiver(int to);

    bool getVersion(int & hwVer, int & swVer);

    string getDeviceID();
    string getDeviceDesc();
    
    int getStatus();
    
    int getNumChannels(uint8_t * blocks = NULL);
    vector<int> getChannelNumbers(int block = 0);
    string getChannelVariable(int chan);
    string getChannelUnit(int chan);
    umbType getChannelDataType(int chan);
    umbValue getChannelValueType(int chan);
    
    bool getOnlineData(int chan, umbType & type, void* & data);
    float getOnlineDataAsFloat(int chan, float defaultValue = -1.f);
    
    bool resetSoftware();

private:
    bool transact(int timeout = UMB_RESPONSE_SHORT);

    ofSerial serial;
    ofxUmbFrame frameIn;
    ofxUmbFrame frameOut;
};



#endif /* ofxUmb_h */
