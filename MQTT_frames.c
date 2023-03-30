#include "MQTT_frames.h"

f_Connect create_ConnectF(char username[], uint16_t kA){
    f_Connect frame;

    frame.bFrameType = 0x10;
    frame.wMsgLen = sizeof(f_Connect);
    frame.wProtlNameLen = 0x0004;
    strcpy(frame.sProtName,"MQTT");
    frame.bVersion = 0x02;
    frame.bConnectFlags = 0x04;
    frame.bKeepAlive = kA;
    frame.wClientIdLen = strlen(username);
    strcpy(frame.sClientID, username);

    return frame;
}

f_ConnAcknowledge create_ConnAck(uint8_t ret_code){
    f_ConnAcknowledge frame;

    frame.bFrameType = 0x20;
    frame.bRemainLen = 0x02;
    frame.bReservedVal = 0x00;
    frame.bReturnCode = ret_code;
   
    return frame;
}

f_PingRequest create_PReq(){
    f_PingRequest frame;

    frame.bFrameType = 0xC0;
    frame.bkeepAlive = 0x00;

    return frame;
}

f_PingResponse create_PRes(){
    f_PingResponse frame;

    frame.bFrameType = 0xD0;
    frame.bresponse = 0x00;

    return frame;
}