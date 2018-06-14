//
//  ofxUMB.h
//  
//
//  Created by Tobias Ebsen on 09/05/18.
//
//

#ifndef ofxUMB_h
#define ofxUMB_h

#include <ofMain.h>

#define UMB_VER     0x10

#define UMB_SOH     0x01
#define UMB_STX     0x02
#define UMB_ETX     0x03
#define UMB_EOT     0x04


class ofxUmbFrame {
public:
    ofxUmbFrame();
    
    void prepareFrame(uint8_t cmd, uint8_t npayload = 0);
    bool validFrame();
    void clear();
    
    void setReceiver(uint16_t to);
    void setSender(uint16_t from);
    void setCommand(uint8_t cmd);
    void setPayload(uint8_t nbyte, uint8_t value);
    void setPayloadShort(uint8_t offset, uint16_t value);
    
    void setCheckSum(uint16_t cs);
    void setCheckSum();

    void setSize(uint8_t size);
    
    uint16_t getReceiver();
    uint16_t getSender();
    uint8_t getCommand();
    uint8_t getLength();
    uint8_t getPayloadLength();
    uint8_t getPayloadByte(uint8_t offset);
    uint16_t getPayloadShort(uint8_t offset);
    string getPayloadString(uint8_t offset);
    string getPayloadString(uint8_t offset, uint8_t len);
    void* getPayloadPtr(uint8_t offset);
    
    uint8_t getResponseStatus();

    uint16_t getCheckSum();
    uint16_t calcCheckSum();
    bool validCheckSum();
    
    uint8_t * getData();
    uint8_t getSize();

private:
    vector<uint8_t> frame;
    uint16_t receiver;
    uint16_t sender;
};


#endif /* ofxUMB_h */
