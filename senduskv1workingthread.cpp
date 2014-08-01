#include "senduskv1workingthread.h"
#include "senduskv1global.h"

#include "sendusk1protocol.h"

#include <QStringList>

using namespace SendUSKv1Namespace;

SendUSKv1WorkingThread::SendUSKv1WorkingThread(QObject *parent) :
    QObject(parent)
{

}

void SendUSKv1WorkingThread::getInfoAboutUsk(QStringList &uskNameList, QStringList &portNameList, QList<int> &uskStatusList)
{
    QList<QString> uskLst = m_hashOfUsk.keys();
    for (const QString &uskName: uskLst) {
        SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
        if (protocol) {
            uskNameList.append(uskName);
            portNameList.append(protocol->getUskPortName());
            uskStatusList.append(protocol->getUskStatus());
        }
    }
}

void SendUSKv1WorkingThread::addUsk(const QString &uskName, const QString &portName, int uskNum)
{
    bool emitVal = false;
    if (!m_hashOfUsk.contains(uskName)) {
        SendUsk1Protocol *protocol = new SendUsk1Protocol(this);
        protocol->setUskName(uskName);
        protocol->setUskNum(uskNum);
        protocol->setSerialPortName(portName);
        protocol->setAttemptsCount(3);
        m_hashOfUsk[uskName] = protocol;
        connect(protocol, SIGNAL(commandAccepted(QString,QString)),
                this, SIGNAL(commandAccepted(QString,QString)));

        connect(protocol, SIGNAL(detectedDisconnetcedKpu(QString,int,int)),
                this, SIGNAL(detectedDisconnetcedKpu(QString,int,int)));

        connect(protocol, SIGNAL(detectedNewKpu(QString,int,int)),
                this, SIGNAL(detectedNewKpu(QString,int,int)));

        connect(protocol, SIGNAL(error(QString,int)),
                this, SIGNAL(error(QString,int)));

        connect(protocol, SIGNAL(errorOnSendingCommand(QString,QString)),
                this, SIGNAL(errorOnSendingCommand(QString,QString)));

        connect(protocol, SIGNAL(portIsClose(QString,QString)),
                this, SIGNAL(portIsClose(QString,QString)));

        connect(protocol, SIGNAL(portIsOpen(QString,QString)),
                this, SIGNAL(portIsOpen(QString,QString)));

        connect(protocol, SIGNAL(receivedTextMessage(QString,QString)),
                this, SIGNAL(receivedTextMessage(QString,QString)));

        connect(protocol, SIGNAL(sensorChanged(QString,int,int,int,int)),
                this, SIGNAL(sensorChanged(QString,int,int,int,int)));

        connect(protocol, SIGNAL(startSendingCommand(QString,QString)),
                this, SIGNAL(startSendingCommand(QString,QString)));

        connect(protocol, SIGNAL(unknowCommand(QString,QString)),
                this, SIGNAL(unknowCommand(QString,QString)));

        connect(protocol, SIGNAL(uskInfoPacketReceived(QString,int)),
                this, SIGNAL(uskInfoPacketReceived(QString,int)));

        connect(protocol, SIGNAL(uskIsPresent(QString,bool,bool)),
                this, SIGNAL(uskIsPresent(QString,bool,bool)));

        connect(protocol, SIGNAL(uskReset(QString)),
                this, SIGNAL(uskReset(QString)));

        connect(protocol, SIGNAL(voltageStatusChanged(QString,int,bool)),
                this, SIGNAL(voltageStatusChanged(QString,int,bool)));

    }
    emit uskIsAdded(uskName, emitVal);
}

void SendUSKv1WorkingThread::removeUsk(const QString &uskName)
{
    bool emitVal = false;
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        delete protocol;
        m_hashOfUsk.remove(uskName);
        emitVal = true;
    }
    emit uskIsDeleted(uskName, emitVal);
}

void SendUSKv1WorkingThread::openUsk(const QString &uskName)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->openUsk();
    }
}

void SendUSKv1WorkingThread::closeUsk(const QString &uskName)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->closeUsk();
    }
}

void SendUSKv1WorkingThread::sendTime(const QString &uskName, const QDateTime &time)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->sendTime(time);
    }
}

void SendUSKv1WorkingThread::sendMessage(const QString &uskName, const QString &message)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->sendMessage(message);
    }
}

void SendUSKv1WorkingThread::resetUsk(const QString &uskName)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->resetUsk();
    }
}

void SendUSKv1WorkingThread::changeRelayStatus(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus, const QString &sensorName)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->changeRelayStatus(rayNum, kpuNum, sensorNum, relayStatus, sensorName);
    }
}

void SendUSKv1WorkingThread::changeVoltageStatus(const QString &uskName, const int numOutput, const bool &on)
{
    SendUsk1Protocol *protocol = m_hashOfUsk.value(uskName, nullptr);
    if (protocol) {
        protocol->changeVoltageStatus(numOutput, on);
    }
}

void SendUSKv1WorkingThread::removeAllUsk()
{
    QList<QString> uskNameList = m_hashOfUsk.keys();
    for (const QString &uskName: uskNameList) {
        removeUsk(uskName);
    }
}
