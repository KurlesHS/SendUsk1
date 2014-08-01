#ifndef SENDUSKV1GLOBAL_H
#define SENDUSKV1GLOBAL_H

namespace SendUSKv1Namespace {

enum errorCodes
{
    errorOpenPort,
    errorTimeoutWhileWaitData,
    errorTimeoutWhileWaitResponse,
    errorUskIsntResponse,
    errorUskInstPresent,
    errorUskWrongPacket
};

enum uskInfoPackets
{
    packetUskReset,
    packetUskSettingTime,
    packetUskOn,
    packetUskErrorReceivingRS
};

enum uskStates {
    uskIsPresent,
    uskIsMissing,
    uskIsClose
};
}

#endif // SENDUSKV1GLOBAL_H
