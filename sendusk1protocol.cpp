#include "sendusk1protocol.h"
#include "senduskv1global.h"

#include <QSerialPort>
#include <QTimer>
#include <QDebug>

using namespace SendUSKv1Namespace;

#define waitForRemainingDataTimeout 250
#define waitResponseTimeout 1500
#define sendTimePeriod 60000


SendUsk1Protocol::SendUsk1Protocol(QObject *parent) :
    QObject(parent),
    m_serialPort(nullptr),
    m_timer(new QTimer(this)),
    m_timerForCheckOutgoingPackets(new QTimer(this)),
    m_timerForSendTime(new QTimer(this)),
    m_currentUskState(waitData),
    m_incomingCommandFactory(new Usk1IncomingCommandFactory(this)),
    m_attempts(3)
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    connect(m_timerForCheckOutgoingPackets, SIGNAL(timeout()),
            this, SLOT(checkOutgoingBuffer()));
    connect(m_timerForSendTime, SIGNAL(timeout()),
            this, SLOT(onSendTimeTimeout()));
}

SendUsk1Protocol::~SendUsk1Protocol()
{
    delete m_incomingCommandFactory;
}

void SendUsk1Protocol::setSerialPortName(const QString &portName)
{
    m_portName = portName;
}

void SendUsk1Protocol::setUskName(const QString &uskName)
{
    m_uskName = uskName;
}

void SendUsk1Protocol::setUskNum(const int uskNum)
{
    m_uskNum = uskNum;
}

QString SendUsk1Protocol::getUskName() const
{
    return m_uskName;
}

QString SendUsk1Protocol::getUskPortName() const
{
    return m_portName;
}

int SendUsk1Protocol::getUskStatus() const
{
    int retVal = SendUSKv1Namespace::uskIsClose;
    if (m_serialPort) {
        retVal = m_uskIsPresent ? SendUSKv1Namespace::uskIsPresent : uskIsMissing;
    }
    return retVal;
}

void SendUsk1Protocol::setAttemptsCount(const int attempts)
{
    m_attempts = attempts;
}

bool SendUsk1Protocol::openUsk()
{
    if (m_serialPort) {
        return true;
    }
    m_serialPort = new QSerialPort(m_portName, this);
    m_firstUse = true;
    m_uskIsPresent = false;
    bool res = m_serialPort->open(QIODevice::ReadWrite);
    if (!res) return res;
    if (m_serialPort->setDataBits(QSerialPort::Data8) &&
            m_serialPort->setBaudRate(9600) &&
            m_serialPort->setStopBits(QSerialPort::OneStop) &&
            m_serialPort->setFlowControl(QSerialPort::NoFlowControl) &&
            m_serialPort->setParity(QSerialPort::NoParity)) {
        connect(m_serialPort, SIGNAL(readyRead()),
                this, SLOT(onReadyRead()));
        emit portIsOpen(m_uskName, m_portName);
        m_timerForCheckOutgoingPackets->start(200);
        m_timerForSendTime->start(sendTimePeriod);
        sendTime(QDateTime::currentDateTime());
        res = true;

    }  else {
        emit error(m_uskName, errorOpenPort);
        res = false;
        delete m_serialPort;
        m_serialPort = nullptr;
    }
    m_buffer.clear();
    return res;
}

void SendUsk1Protocol::closeUsk()
{
    m_timer->stop();
    m_timerForCheckOutgoingPackets->stop();
    m_timerForSendTime->stop();
    if (!m_serialPort) {
        return;
    }
    QString portName = m_serialPort->portName();
    m_serialPort->close();
    m_serialPort->deleteLater();
    emit portIsClose(m_uskName, portName);
}

void SendUsk1Protocol::sendTime(const QDateTime &time)
{
    auto cmd = new SendTimeUsk1OutgoingCommand(m_serialPort, m_uskNum, m_attempts, time);
    m_outgoingCommnads.append(Usk1OutgoingCommandSharedPtr(cmd));
}

void SendUsk1Protocol::sendMessage(const QString &message)
{
    auto cmd = new SendMessageUsk1OutgoingCommand(m_serialPort, m_uskNum, m_attempts, message);
    m_outgoingCommnads.append(Usk1OutgoingCommandSharedPtr(cmd));
}

void SendUsk1Protocol::resetUsk()
{
    auto cmd = new ResetUsk1OutgoingCommand(m_serialPort, m_uskNum, m_attempts);
    m_outgoingCommnads.append(Usk1OutgoingCommandSharedPtr(cmd));
}

void SendUsk1Protocol::changeRelayStatus(const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus, const QString &sensorName)
{
    auto cmd = new ChangeRelayUsk1OutgoingCommand(m_serialPort, m_uskNum, m_attempts, rayNum, kpuNum, sensorNum, relayStatus, sensorName);
    m_outgoingCommnads.append(Usk1OutgoingCommandSharedPtr(cmd));
}

void SendUsk1Protocol::changeVoltageStatus(const int numOutput, const bool &on)
{
    auto cmd = new ChangeVoltageUsk1OutgoingCommand(m_serialPort, m_uskNum, m_attempts, numOutput, on);
    m_outgoingCommnads.append(Usk1OutgoingCommandSharedPtr(cmd));
}

void SendUsk1Protocol::onUnknowCommand(const QString &command)
{
    emit unknowCommand(m_uskName, command);
}

void SendUsk1Protocol::onError(int errorCode)
{
    emit error(m_uskName, errorCode);
}

void SendUsk1Protocol::onResetUskCommand()
{
    emit uskReset(m_uskName);
    emit uskInfoPacketReceived(m_uskName, packetUskReset);
}

void SendUsk1Protocol::onReceivedTextMessage(const QString &textMessage)
{
    emit receivedTextMessage(m_uskName, textMessage);
}

void SendUsk1Protocol::onDetectedNewKpu(const int &rayNum, const int &kpuNum)
{
    emit detectedNewKpu(m_uskName, rayNum, kpuNum);
}

void SendUsk1Protocol::onDetectedDisconnetcedKpu(const int &rayNum, const int &kpuNum)
{
    emit detectedDisconnetcedKpu(m_uskName, rayNum, kpuNum);
}

void SendUsk1Protocol::onVoltageStatusChanged(const int &outputNumber, const bool &status)
{
    emit voltageStatusChanged(m_uskName, outputNumber, status);
}

void SendUsk1Protocol::onSensorChanged(const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state)
{
    emit sensorChanged(m_uskName, rayNum, kpuNum, sensorNum, state);
}

void SendUsk1Protocol::onUskInfoPacketReceived(const int &infoPacket)
{
    emit uskInfoPacketReceived(m_uskName, infoPacket);
}

void SendUsk1Protocol::parseIncomingData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length();
    switch (m_currentUskState) {
    case waitData:
    case waitIncomingCommand:
    case waitTime:
    {
        m_currentUskState = waitData;
        // входящая команда
        while (m_buffer.length() >= 26) {
            QByteArray packet = m_buffer.left(26);
            m_buffer.remove(0, 26);
            Usk1IncomingCommandSharedPtr cmd = m_incomingCommandFactory->getCommandByPacket(packet);
            if (cmd) {
                cmd->informAboutCommand();
                if (!m_uskIsPresent && cmd->isCorrectPacket()) {
                    emitUskIsPresent(true);
                }
            }
        }
        if (m_buffer.length() > 0) {
            // надо что-то допринять
            m_timer->start(waitForRemainingDataTimeout);
        }
    }

        break;
    case waitResponse: {
        // отклик (5 байт)
        if (m_buffer.length() < 5) {
            m_timer->start(waitForRemainingDataTimeout);
            return;
        }
        m_currentUskState = waitData;
        QByteArray packet = m_buffer.left(5);
        m_buffer.remove(0, 5);
        char crc = 0;
        for (int i = 0; i < 4; ++i) {
            crc += packet.at(i);
        }
        if (crc != packet.at(4)) {
            error(m_uskName, errorUskWrongPacket);
        }
        if (m_currentCommand && m_currentCommand->needToInformAboutStartSending()){
            emit commandAccepted(m_uskName, m_currentCommand->description());
        }
        if (!m_uskIsPresent){
            emitUskIsPresent(true);
            m_uskIsPresent = true;
        }
        m_currentCommand = Usk1OutgoingCommandSharedPtr(nullptr);

        if (m_buffer.length() > 0) {
            parseIncomingData();
            return;
        }
    }
        break;
    default:
        break;
    }
}

void SendUsk1Protocol::emitUskIsPresent(const bool isPresent)
{
    emit uskIsPresent(m_uskName, isPresent, m_firstUse);
    if (m_firstUse) m_firstUse = false;
}

void SendUsk1Protocol::onReadyRead()
{
    QSerialPort *serialPort = qobject_cast<QSerialPort*>(sender());
    if (!serialPort) {
        return;
    }
    m_timer->stop();
    m_buffer.append(serialPort->readAll());
    parseIncomingData();
}

void SendUsk1Protocol::onTimerTimeout()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if (!timer) return;
    timer->stop();
    switch (m_currentUskState) {
    case waitData:
    {
        qDebug() << Q_FUNC_INFO << m_buffer.length();
        emit error(m_uskName, errorTimeoutWhileWaitData);
        m_buffer.clear();
    }
        break;
    case waitIncomingCommand:
    case waitTime:
    {
        m_currentUskState = waitData;
    }
        break;
    case waitResponse:
    {

        if (m_buffer.length() == 0) {
            emit error(m_uskName, errorUskIsntResponse);
        } else {
            emit error(m_uskName, errorTimeoutWhileWaitResponse);
        }
        m_uskIsPresent = false;
        emitUskIsPresent(false);
        m_buffer.clear();
        if (m_currentCommand && m_currentCommand->isAnotherAttemptPresent()) {
            m_currentCommand->sendCommand();
            m_timer->start(waitResponseTimeout);
        } else {
            if (m_currentCommand && m_currentCommand->needToInformAboutStartSending()) {
                emit errorOnSendingCommand(m_uskName, m_currentCommand->description());
            }
            m_currentCommand = Usk1OutgoingCommandSharedPtr(nullptr);
            m_currentUskState = waitData;
        }
    }
    default:
        break;
    }
}

void SendUsk1Protocol::checkOutgoingBuffer()
{
    if (m_currentUskState == waitData && m_buffer.length() == 0 && m_currentCommand == nullptr && m_outgoingCommnads.count() > 0) {
        m_currentCommand = m_outgoingCommnads.takeFirst();
        if (m_currentCommand) {
            if (m_currentCommand->needToInformAboutStartSending() && m_currentCommand->isFirstAttempt()) {
                emit startSendingCommand(m_uskName, m_currentCommand->description());
            }
            m_currentCommand->sendCommand();
            m_currentUskState = waitResponse;
            m_timer->start(waitResponseTimeout);
        } else {
            checkOutgoingBuffer();
        }
    }
}

void SendUsk1Protocol::onSendTimeTimeout()
{
    sendTime(QDateTime::currentDateTime());
}
