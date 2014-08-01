#ifndef USK1INCOMINGCOMMAND_H
#define USK1INCOMINGCOMMAND_H

#include <QObject>
#include <QSharedPointer>
#include <functional>
#include <QList>
#include <QMap>
#include <QHash>
#include <QPair>

class SendUsk1Protocol;
class QTextCodec;

class Usk1IncomingCommand
{
public:
    Usk1IncomingCommand(SendUsk1Protocol *protocol);
    virtual ~Usk1IncomingCommand() {}

    bool parsePacket(const QByteArray &packet);
    virtual bool isMyPacket(const QByteArray &packet) = 0;
    virtual QString description() const  = 0;
    virtual void informAboutCommand() = 0;
    bool isCorrectPacket() const;
protected:
    static QTextCodec *getWin1251TextCodec();

protected:
    SendUsk1Protocol *m_protocol;
    quint32 m_flags;
    ushort m_uskNumber;
    bool m_isCorrectPacket;
    QByteArray m_packetData;
    char m_u1;
    char m_u2;
};

typedef QSharedPointer<Usk1IncomingCommand> Usk1IncomingCommandSharedPtr;
typedef std::function<Usk1IncomingCommandSharedPtr()> producedFunction;

class Usk1IncomingCommandFactory {
public:
    Usk1IncomingCommandFactory(SendUsk1Protocol *protocol);
    template<typename A>
    void registerClass(const int priority) {
        m_functions[priority].append([this]() {return Usk1IncomingCommandSharedPtr(new A(m_protocol));});
    }
    Usk1IncomingCommandSharedPtr getCommandByPacket(const QByteArray &packet);

private:
    SendUsk1Protocol *m_protocol;
    QMap<int, QList<producedFunction> > m_functions;
};

class UnknowUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    UnknowUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();
};

class BadUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    BadUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();
};

class ResetUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    ResetUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();
};

class TextMessageUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    TextMessageUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();
};

class NewKpuUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    NewKpuUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();

private:
    int m_kpuNum;
    int m_rayNum;
};

class DisconnectedKpuUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    DisconnectedKpuUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();

private:
    int m_kpuNum;
    int m_rayNum;
};

class VoltageStatusChangedUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    VoltageStatusChangedUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();

private:
    int m_numOutput;
    bool m_on;
};

class SensorChangeUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    SensorChangeUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();

private:
    QList<QPair<int, int> > getChangedRelays() const;

private:
    int m_rayNum;
    int m_kpuNum;
    int m_sensorNum;
    char m_prevState;
    char m_curState;
};

class InfoUsk1IncomingCommand : public Usk1IncomingCommand {
public:
    InfoUsk1IncomingCommand(SendUsk1Protocol *protocol);
    bool isMyPacket(const QByteArray &packet);
    QString description() const;
    void informAboutCommand();

private:
    static QHash<QString, int> m_uskInfoHash;
    QString m_message;
};

#endif // USK1INCOMINGCOMMAND_H
