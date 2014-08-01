#ifndef SENDUSK1PROTOCOL_H
#define SENDUSK1PROTOCOL_H

#include <QObject>
#include <QDateTime>

#include "usk1outgoingcommand.h"
#include "usk1incomingcommand.h"

class QSerialPort;
class QTimer;

class SendUsk1Protocol : public QObject
{
    Q_OBJECT
    enum States {
        waitData = 0x01,
        waitResponse,
        waitIncomingCommand,
        waitTime
    };

public:
    explicit SendUsk1Protocol(QObject *parent = 0);
    ~SendUsk1Protocol();
    void setSerialPortName(const QString &portName);
    void setUskName(const QString &uskName);
    void setUskNum(const int uskNum);
    QString getUskName() const;
    QString getUskPortName() const;
    int getUskStatus() const;

    void setAttemptsCount(const int attempts);
    bool openUsk();
    void closeUsk();
    void sendTime(const QDateTime &time);
    void sendMessage(const QString &message);
    void resetUsk();
    void changeRelayStatus(const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus, const QString &sensorName);
    void changeVoltageStatus(const int numOutput, const bool &on);

    void onUnknowCommand(const QString &command);
    void onError(int errorCode);
    void onResetUskCommand();
    void onReceivedTextMessage(const QString &textMessage);
    void onDetectedNewKpu(const int &rayNum, const int &kpuNum);
    void onDetectedDisconnetcedKpu(const int &rayNum, const int &kpuNum);
    void onVoltageStatusChanged(const int &outputNumber, const bool &status);
    void onSensorChanged(const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state);
    void onUskInfoPacketReceived(const int &infoPacket);

signals:
    void error(const QString &uskName, int errorCode);
    void unknowCommand(const QString &uskName, const QString &command);
    void errorOnSendingCommand(const QString &uskName, const QString &commandDescription);
    void commandAccepted(const QString &uskName, const QString &commandDescription);
    void uskInfoPacketReceived(const QString &uskName, const int &infoPacket);
    void portIsOpen(const QString &uskName, const QString &portName);
    void portIsClose(const QString &uskName, const QString &portName);
    void uskReset(const QString &uskName);
    void receivedTextMessage(const QString &uskName, const QString &textMessage);
    void uskIsPresent(const QString &uskName, const bool &present, const bool &firstUse);
    void startSendingCommand(const QString &uskName, const QString &commandDescription);
    void detectedNewKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void detectedDisconnetcedKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void voltageStatusChanged(const QString &uskName, const int &outputNumber, const bool &status);
    void sensorChanged(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state);

private:
    void parseIncomingData();
    void emitUskIsPresent(const bool isPresent);


private slots:
    void onReadyRead();
    void onTimerTimeout();
    void checkOutgoingBuffer();
    void onSendTimeTimeout();

private:
    QSerialPort *m_serialPort;
    bool m_firstUse;
    bool m_uskIsPresent;
    QString m_portName;
    QString m_uskName;
    QByteArray m_buffer;
    QTimer *m_timer;
    QTimer *m_timerForCheckOutgoingPackets;
    QTimer *m_timerForSendTime;
    Usk1OutgoingCommandSharedPtrList m_outgoingCommnads;
    Usk1OutgoingCommandSharedPtr m_currentCommand;
    States m_currentUskState;
    Usk1IncomingCommandFactory *m_incomingCommandFactory;
    int m_attempts;
    int m_uskNum;
};

#endif // SENDUSK1PROTOCOL_H
