//
//  ofxUmb.cpp
//  boelgen
//
//  Created by Tobias Ebsen on 09/05/18.
//
//

#include "ofxUmb.h"

ofxUmb::ofxUmb() {
    frameOut.setSender(0xF001);
}

vector<ofSerialDeviceInfo> ofxUmb::getSerialDevices() {
	return serial.getDeviceList();
}

vector<string> ofxUmb::getSerialNames() {
	vector<string> names;
	vector<ofSerialDeviceInfo> infos = serial.getDeviceList();
	for (auto & i : infos) {
		names.push_back(i.getDeviceName());
	}
	return names;
}

bool ofxUmb::connect(int device) {
    return serial.setup(device, 19200);
}

bool ofxUmb::connect(string deviceName) {
    return serial.setup(deviceName, 19200);
}

void ofxUmb::close() {
    serial.close();
}

bool ofxUmb::isConnected() {
    return serial.isInitialized();
}

void ofxUmb::flush() {
    serial.flush();
}

void ofxUmb::setReceiver(int to) {
    frameOut.setReceiver(to);
}

bool ofxUmb::getVersion(int &hwVer, int &swVer) {
    frameOut.prepareFrame(0x20);
    if (transact()) {
        hwVer = frameIn.getPayloadByte(1);
        swVer = frameIn.getPayloadByte(2);
        return true;
    }
    return false;
}

string ofxUmb::getDeviceID() {
    frameOut.prepareFrame(0x2D, 1);
    frameOut.setPayload(0, 0x11);
    if (transact()) {
        return frameIn.getPayloadString(2, 40);
    }
    return string();
}

string ofxUmb::getDeviceDesc() {
    frameOut.prepareFrame(0x2D, 1);
    frameOut.setPayload(0, 0x10);
    if (transact()) {
        return frameIn.getPayloadString(2, 40);
    }
    return string();
}

int ofxUmb::getStatus() {
    frameOut.prepareFrame(0x26);
    if (transact()) {
        return frameIn.getPayloadByte(1);
    }
    return -1;
}

int ofxUmb::getNumChannels(uint8_t * blocks) {
    frameOut.prepareFrame(0x2D, 1);
    frameOut.setPayload(0, 0x15);
    if (transact()) {
        if (blocks != NULL)
            *blocks = frameIn.getPayloadByte(4);
        return frameIn.getPayloadShort(2);
    }
    return 0;
}

vector<int> ofxUmb::getChannelNumbers(int block) {
    vector<int> chan;
    frameOut.prepareFrame(0x2D, 2);
    frameOut.setPayload(0, 0x16);
    frameOut.setPayload(1, block);
    if (transact()) {
        uint8_t b = frameIn.getPayloadByte(2);
        uint8_t n = frameIn.getPayloadByte(3);
        for (int i=0; i<n; i++) {
            int c = frameIn.getPayloadShort(4+i*2);
            chan.push_back(c);
        }
    }
    return chan;
}

string ofxUmb::getChannelVariable(int chan) {
    frameOut.prepareFrame(0x2D, 3);
    frameOut.setPayload(0, 0x20);
    frameOut.setPayloadShort(1, chan);
    if (transact()) {
        uint16_t c = frameIn.getPayloadShort(2);
        return frameIn.getPayloadString(4, 20);
    }
    return string();
}

string ofxUmb::getChannelUnit(int chan) {
    frameOut.prepareFrame(0x2D, 3);
    frameOut.setPayload(0, 0x22);
    frameOut.setPayloadShort(1, chan);
    if (transact()) {
        uint16_t c = frameIn.getPayloadShort(2);
        return frameIn.getPayloadString(4, 15);
    }
    return string();
}

UMB::TYPE ofxUmb::getChannelDataType(int chan) {
    frameOut.prepareFrame(0x2D, 3);
    frameOut.setPayload(0, 0x23);
    frameOut.setPayloadShort(1, chan);
    if (transact()) {
        uint16_t c = frameIn.getPayloadShort(2);
        return (UMB::TYPE)frameIn.getPayloadByte(4);
    }
    return (UMB::TYPE)0;
}

UMB::VALUE ofxUmb::getChannelValueType(int chan) {
    frameOut.prepareFrame(0x2D, 3);
    frameOut.setPayload(0, 0x24);
    frameOut.setPayloadShort(1, chan);
    if (transact()) {
        return (UMB::VALUE)frameIn.getPayloadByte(4);
    }
    return (UMB::VALUE)0;
}

bool ofxUmb::getOnlineData(int chan, UMB::TYPE & type, void* & data) {
    frameOut.prepareFrame(0x23, 2);
    frameOut.setPayloadShort(0, chan);
    if (transact(UMB_RESPONSE_LONG)) {
        uint16_t c = frameIn.getPayloadShort(1);
        type = (UMB::TYPE)frameIn.getPayloadByte(3);
        data = frameIn.getPayloadPtr(4);
        return true;
    }
    return false;
}

float ofxUmb::getOnlineDataAsFloat(int chan, float defaultValue) {
    UMB::TYPE t;
    void * d = NULL;
    if (getOnlineData(chan, t, d)) {
        if (t == UMB::FLOAT && d != NULL)
            return *(float*)d;
    }
    return defaultValue;
}

bool ofxUmb::resetSoftware() {
    frameOut.prepareFrame(0x25);
    frameOut.setPayload(0, 0x10);
    return transact();
}

bool ofxUmb::transact(int timeout) {
    
    frameOut.setCheckSum();

    serial.writeBytes(frameOut.getData(), frameOut.getSize());

    uint64_t txTime = ofGetElapsedTimeMillis();
    int t = 0;
    bool gotResponse = false;
    while (!gotResponse && t < timeout) {
        if (serial.available() > 0) {

            frameIn.setSize(255);
            frameIn.clear();
            int n = serial.readBytes(frameIn.getData(), frameIn.getSize());
            if (n == 0) {
                ofLogWarning("ofxUmb") << "transact(). Read partial response.";
            }
            else {
                
                txTime = ofGetElapsedTimeMillis();
                int len = n >= 7 ? frameIn.getLength() : 0;
                while ((len == 0 || n < len) && t < timeout+100) {
                    if (serial.available() > 0) {
                        n += serial.readBytes(frameIn.getData()+n, frameIn.getSize()-n);
                        if (len == 0 && n >=7) {
                            len = frameIn.getLength();
                        }
                    }
                    else {
                        ofSleepMillis(1);
                    }
                    t = ofGetElapsedTimeMillis()-txTime;
                }
                if (len > 0)
                    frameIn.setSize(len);

                if (!frameIn.validFrame()) {
                    ofLogWarning("ofxUmb") << "transact(). Invalid response.";
                }
                else {
                
                    if (frameIn.getSender() != frameOut.getReceiver()) {
                        ofLogWarning("ofxUmb") << "transact(). Response from other device.";
                    }
                    else {
                    
                        if (frameIn.getCommand() != frameOut.getCommand()) {
                            ofLogWarning("ofxUmb") << "transact(). Response command mismatch.";
                        }
                        else {
                            gotResponse = true;
                        }
                    }
                }
            }
        }
        else {
            ofSleepMillis(1);
        }
        t = ofGetElapsedTimeMillis()-txTime;
    }

    if (!gotResponse) {
        ofLogError("ofxUmb") << "transact() failed. Response timeout.";
        return false;
    }
    
    if (frameIn.getResponseStatus() != UMB::OK) {
        ofLogError("ofxUmb") << "transact() failed. Response status code: " << (int)frameIn.getResponseStatus();
        return false;
    }

    return true;
}