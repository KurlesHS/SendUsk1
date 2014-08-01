#ifndef USK1OUTGOINGCOMMAND_H
#define USK1OUTGOINGCOMMAND_H

#include <QSharedPointer>
#include <QDateTime>
#include <QList>

class QSerialPort;

class Usk1OutgoingCommand
{
public:
    Usk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,  const int attempts);
    virtual ~Usk1OutgoingCommand();
    virtual QString description() const = 0;
    virtual QByteArray outgoingBinaryPacket() const = 0;
    virtual bool needToWaitCommand() const;
    virtual bool needToInformAboutStartSending() const;
    bool isAnotherAttemptPresent() const;
    bool isFirstAttempt() const;
    void sendCommand();
    int uskNumber() const;

protected:
    static QTextCodec *getWin1251TextCodec();
    QByteArray getFirstPartOfPacket(const quint64 &flags,
                                    const quint8 &priority = 0x00) const;
    void appendCrcToPacket(QByteArray &packet) const;

private:
    QSerialPort *m_serialPort;
    int m_attempts;
    bool m_isFirstAttempt;
    int m_uskNumber;
};

typedef QSharedPointer<Usk1OutgoingCommand> Usk1OutgoingCommandSharedPtr;
typedef QList<Usk1OutgoingCommandSharedPtr> Usk1OutgoingCommandSharedPtrList;

class SendTimeUsk1OutgoingCommand : public Usk1OutgoingCommand
{
public:
    SendTimeUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                                const int attempts,
                                const QDateTime &dateTime);
    virtual QString description() const;
    virtual QByteArray outgoingBinaryPacket() const;
    bool needToInformAboutStartSending() const;

private:
    QDateTime m_dateTime;
};

class SendMessageUsk1OutgoingCommand : public Usk1OutgoingCommand
{
public:
    SendMessageUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                                const int attempts,
                                const QString &message);
    virtual QString description() const;
    virtual QByteArray outgoingBinaryPacket() const;

private:
    QString m_message;
};

class ResetUsk1OutgoingCommand : public Usk1OutgoingCommand
{
public:
    ResetUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                             const int attempts);
    virtual QString description() const;
    virtual QByteArray outgoingBinaryPacket() const;
    virtual bool needToWaitCommand() const;
};

class ChangeRelayUsk1OutgoingCommand : public Usk1OutgoingCommand
{
public:
    ChangeRelayUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                                   const int attempts, const int &rayNum,
                                   const int &kpuNum, const int &sensorNum,
                                   const int &relayStatus, const QString &sensorName);
    virtual QString description() const;
    virtual QByteArray outgoingBinaryPacket() const;

private:
    int m_rayNum;
    int m_kpuNum;
    int m_sensorNum;
    int m_relayStatus;
    QString m_sensorName;
};

class ChangeVoltageUsk1OutgoingCommand : public Usk1OutgoingCommand
{
public:
    ChangeVoltageUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                             const int attempts,
                             const int numOutput, const bool &on);
    virtual QString description() const;
    virtual QByteArray outgoingBinaryPacket() const;

private:
    int m_numOutput;
    bool m_on;
};


#endif // USK1OUTGOINGCOMMAND_H
