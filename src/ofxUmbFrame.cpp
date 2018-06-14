//
//  ofxUmbFrame.cpp
//  
//
//  Created by Tobias Ebsen on 09/05/18.
//
//

#include "ofxUmbFrame.h"

ofxUmbFrame::ofxUmbFrame() {
    frame.reserve(255);
}

void ofxUmbFrame::setSize(uint8_t size) {
    frame.resize(size);
}

void ofxUmbFrame::prepareFrame(uint8_t cmd, uint8_t npayload) {
    uint8_t len = 2 + MIN(npayload, 210);
    frame.resize(12 + len);

    frame[0] = UMB_SOH;
    frame[1] = UMB_VER;
    frame[2] = receiver & 0xFF;
    frame[3] = receiver >> 8;
    frame[4] = sender & 0xFF;
    frame[5] = sender >> 8;
    frame[6] = len;
    frame[7] = UMB_STX;
    frame[8] = cmd;
    frame[9] = 0x10;
    frame[len + 8] = UMB_ETX;
    frame[len + 11] = UMB_EOT;
}

void ofxUmbFrame::clear() {
    std::fill(frame.begin(), frame.end(), 0);
}

bool ofxUmbFrame::validFrame() {

    if (frame.size() < 14)
        return false;
    
    if (frame[0] != UMB_SOH)
        return false;
    if (frame[1] != UMB_VER)
        return false;
    
    if (frame[6] != frame.size()-12)
        return false;
    if (frame[7] != UMB_STX)
        return false;
    
    uint8_t len = frame[6];
    if (frame[len + 8] != UMB_ETX)
        return false;
    if (frame[len + 11] != UMB_EOT)
        return false;

    return true;
}

void ofxUmbFrame::setReceiver(uint16_t to) {
    receiver = to;
    if (frame.size() >= 4) {
        frame[2] = to & 0xFF;
        frame[3] = to >> 8;
    }
}

void ofxUmbFrame::setSender(uint16_t from) {
    sender = from;
    if (frame.size() >= 6) {
        frame[4] = from & 0xFF;
        frame[5] = from >> 8;
    }
}

void ofxUmbFrame::setCommand(uint8_t cmd) {
    frame[8] = cmd;
}

void ofxUmbFrame::setPayload(uint8_t nbyte, uint8_t value) {
    frame[10+nbyte] = value;
}

void ofxUmbFrame::setPayloadShort(uint8_t offset, uint16_t value) {
    frame[10+offset] = value & 0xFF;
    frame[11+offset] = value >> 8;
}

void ofxUmbFrame::setCheckSum(uint16_t cs) {
    uint8_t len = frame.size() - 12;
    frame[len + 9] = cs & 0xFF;
    frame[len + 10] = cs >> 8;
}

void ofxUmbFrame::setCheckSum() {
    setCheckSum(calcCheckSum());
}

uint16_t ofxUmbFrame::calcCheckSum() {
    
    unsigned short crc = 0xFFFF;
    
    for (int n=0; n<frame.size()-3; n++) {
        unsigned char input = frame[n];
        unsigned char i;
        unsigned short x16;
        for (i=0; i<8; i++) {
            if( (crc & 0x0001) ^ (input & 0x01) )
                x16 = 0x8408;
            else
                x16 = 0x0000;
            crc = crc >> 1;
            crc ^= x16;
            input = input >> 1;
        }
    }
    return crc;
}

bool ofxUmbFrame::validCheckSum() {
    return getCheckSum() == calcCheckSum();
}

uint16_t ofxUmbFrame::getReceiver() {
    if (frame.size() < 4)
        return 0;
    return frame[2] | (frame[3] << 8);
}

uint16_t ofxUmbFrame::getSender() {
    if (frame.size() < 6)
        return 0;
    return frame[4] | (frame[5] << 8);
}

uint8_t ofxUmbFrame::getCommand() {
    if (frame.size() < 9)
        return 0;
    return frame[8];
}

uint8_t ofxUmbFrame::getLength() {
    if (frame.size() < 7)
        return 0;
    return frame[6] + 12;
}

uint8_t ofxUmbFrame::getPayloadLength() {
    return frame[6] - 2;
}

uint8_t ofxUmbFrame::getPayloadByte(uint8_t offset) {
    return frame[10+offset];
}

uint16_t ofxUmbFrame::getPayloadShort(uint8_t offset) {
    return frame[10+offset] | (frame[11+offset] << 8);
}

string ofxUmbFrame::getPayloadString(uint8_t offset) {
    return string((char*)frame.data()+10+offset);
}

string ofxUmbFrame::getPayloadString(uint8_t offset, uint8_t len) {
    string s = string((char*)frame.data()+10+offset, len);
    s.append("\0");
    return ofTrimBack(s);
}

void* ofxUmbFrame::getPayloadPtr(uint8_t offset) {
    return frame.data()+10+offset;
}

uint8_t ofxUmbFrame::getResponseStatus() {
    return frame[10];
}

uint16_t ofxUmbFrame::getCheckSum() {
    uint8_t len = frame.size() - 12;
    return frame[len+9] | (frame[len+10] << 8);
}

uint8_t* ofxUmbFrame::getData() {
    return frame.data();
}

uint8_t ofxUmbFrame::getSize() {
    return frame.size();
}