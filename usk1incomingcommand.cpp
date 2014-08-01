#include "usk1incomingcommand.h"
#include "sendusk1protocol.h"
#include "senduskv1global.h"
#include <QTextCodec>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

Usk1IncomingCommand::Usk1IncomingCommand(SendUsk1Protocol *protocol) :
    m_protocol(protocol)
{
}

bool Usk1IncomingCommand::parsePacket(const QByteArray &packet)
{
    m_isCorrectPacket = false;
    if (packet.length() != 26) {
        return false;
    }
    char crc = 0;
    for (int i = 0; i < 25; ++i)
        crc += packet.at(i);
    if (crc != packet.at(25)) {
        return false;
    }
    m_uskNumber = (ushort)packet.at(0) + (ushort)packet.at(1) * 0x100;
    m_u1 = packet.at(23);
    m_u2 = packet.at(24);
    m_flags = (quint32)packet.at(2) * 0x01 +
            (quint32)packet.at(3) * 0x0100 +
            (quint32)packet.at(4) * 0x010000 +
            (quint32)packet.at(5) * 0x01000000;
    m_packetData = packet.mid(7, 16);
    m_isCorrectPacket = true;
    return m_isCorrectPacket;
}

bool Usk1IncomingCommand::isCorrectPacket() const
{
    return m_isCorrectPacket;
}

QTextCodec *Usk1IncomingCommand::getWin1251TextCodec()
{
    static QTextCodec *codecWin1251 = QTextCodec::codecForName("Windows-1251");
    return codecWin1251;
}


Usk1IncomingCommandFactory::Usk1IncomingCommandFactory(SendUsk1Protocol *protocol) :
    m_protocol(protocol)
{
    registerClass<BadUsk1IncomingCommand>(0);
    registerClass<UnknowUsk1IncomingCommand>(65535);
    registerClass<ResetUsk1IncomingCommand>(10);
    registerClass<TextMessageUsk1IncomingCommand>(10);
    registerClass<NewKpuUsk1IncomingCommand>(10);
    registerClass<DisconnectedKpuUsk1IncomingCommand>(10);
    registerClass<VoltageStatusChangedUsk1IncomingCommand>(10);
    registerClass<SensorChangeUsk1IncomingCommand>(10);
    registerClass<InfoUsk1IncomingCommand>(10);
}

Usk1IncomingCommandSharedPtr Usk1IncomingCommandFactory::getCommandByPacket(const QByteArray &packet)
{
    qDebug() << Q_FUNC_INFO;
    QMapIterator<int, QList<producedFunction> > i(m_functions);
    QList<int> priorities = m_functions.keys();
    for (const int &priority: priorities) {
        QList<producedFunction> listOfFunctions = m_functions.value(priority);
        for (producedFunction fn: listOfFunctions) {
            Usk1IncomingCommandSharedPtr command = fn();
            if (command->isMyPacket(packet)) {
                return command;
            }
        }
    }
    return Usk1IncomingCommandSharedPtr(nullptr);

}


UnknowUsk1IncomingCommand::UnknowUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol)
{

}

bool UnknowUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    return parsePacket(packet);
}

QString UnknowUsk1IncomingCommand::description() const
{
    QString message = getWin1251TextCodec()->toUnicode(m_packetData);
    return QObject::trUtf8("неизвестная входящая комманда: %0").arg(message);
}

void UnknowUsk1IncomingCommand::informAboutCommand()
{
    QString message = getWin1251TextCodec()->toUnicode(m_packetData);
    if (m_protocol) {
        m_protocol->onUnknowCommand(message);
    }
}


BadUsk1IncomingCommand::BadUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol)
{

}

bool BadUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    return !parsePacket(packet);
}

QString BadUsk1IncomingCommand::description() const
{
    return QObject::trUtf8("неверный входящий пакет (не сошлась контрольная сумма)");
}

void BadUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        m_protocol->onError(SendUSKv1Namespace::errorUskWrongPacket);
    }
}


ResetUsk1IncomingCommand::ResetUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol)
{

}

bool ResetUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    if (!parsePacket(packet)) {
        return false;
    }
    QString message = getWin1251TextCodec()->toUnicode(m_packetData).trimmed().toLower();
    return message == QString("полный сброс уск");
}

QString ResetUsk1IncomingCommand::description() const
{
    return QObject::trUtf8("входящая команда: 'сброс УСК'");
}

void ResetUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        m_protocol->onResetUskCommand();
    }
}


TextMessageUsk1IncomingCommand::TextMessageUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol)
{

}

bool TextMessageUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    if (!parsePacket(packet)) {
        return false;
    }
    return m_flags & 0x10000;
}

QString TextMessageUsk1IncomingCommand::description() const
{
    QString message = getWin1251TextCodec()->toUnicode(m_packetData);
    return QObject::trUtf8("входящая команда: текстовое сообщение '%0'")
            .arg(message);
}

void TextMessageUsk1IncomingCommand::informAboutCommand()
{
    QString message = getWin1251TextCodec()->toUnicode(m_packetData);
    if (m_protocol) {
        m_protocol->onReceivedTextMessage(message);
    }
}

NewKpuUsk1IncomingCommand::NewKpuUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol),
    m_kpuNum(0),
    m_rayNum(0)
{

}

bool NewKpuUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    bool retVal = false;
    if (parsePacket(packet)) {
        QString message = getWin1251TextCodec()->toUnicode(m_packetData).toLower();
        retVal = message.left(9) == QString("новый оу:");
        if (retVal) {
            m_rayNum = QString(message.at(15)).toInt();
            m_kpuNum = QString(message.at(9)).toInt();
        }
    }
    return retVal;
}

QString NewKpuUsk1IncomingCommand::description() const
{
    return QObject::trUtf8("входящая команда: 'обнаружено новое КПУ: луч %0, номер КПУ %1 '")
            .arg(m_rayNum)
            .arg(m_kpuNum);
}

void NewKpuUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        m_protocol->onDetectedNewKpu(m_rayNum, m_kpuNum);
    }
}


DisconnectedKpuUsk1IncomingCommand::DisconnectedKpuUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol),
    m_kpuNum(0),
    m_rayNum(0)
{

}

bool DisconnectedKpuUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    bool retVal = false;
    if (parsePacket(packet)) {
        QString message = getWin1251TextCodec()->toUnicode(m_packetData).toLower();
        retVal = message.left(9) == QString("неисп.оу:");
        if (retVal) {
            m_rayNum = QString(message.at(15)).toInt();
            m_kpuNum = QString(message.at(9)).toInt();
        }
    }
    return retVal;
}

QString DisconnectedKpuUsk1IncomingCommand::description() const
{
    return QObject::trUtf8("входящая команда: 'обнаружено отключение КПУ: луч %0, номер КПУ %1'")
            .arg(m_rayNum)
            .arg(m_kpuNum);
}

void DisconnectedKpuUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        m_protocol->onDetectedDisconnetcedKpu(m_rayNum, m_kpuNum);
    }
}


VoltageStatusChangedUsk1IncomingCommand::VoltageStatusChangedUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol),
    m_numOutput(0),
    m_on(false)
{

}

bool VoltageStatusChangedUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    bool retVal = false;
    if (parsePacket(packet)) {
        QString message = getWin1251TextCodec()->toUnicode(m_packetData).toLower().trimmed();
        QRegExp re("\\b(\\w+) 220-([12])\\b");
        if (re.indexIn(message) >= 0)
        {
            retVal = true;
            QString cap1 = re.cap(1);
            QString cap2 = re.cap(2);
            m_numOutput = cap2.toInt();
            if (cap1 == QString("включение"))
                m_on = true;
            else if (cap1 == QString("выключение"))
                m_on = false;
            else
                retVal = false;
        }
    }
    return retVal;
}

QString VoltageStatusChangedUsk1IncomingCommand::description() const
{
    return QObject::trUtf8("входящая команда: '%2 НЧ выхода номер %1'")
            .arg(m_numOutput)
            .arg(m_on ? QObject::trUtf8("включение") : QObject::trUtf8("выключение"));
}

void VoltageStatusChangedUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        m_protocol->onVoltageStatusChanged(m_numOutput, m_on);
    }
}


SensorChangeUsk1IncomingCommand::SensorChangeUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol),
    m_rayNum(0),
    m_kpuNum(0),
    m_sensorNum(0),
    m_prevState(0),
    m_curState(0)
{

}

bool SensorChangeUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{

    bool retVal = false;
    if (parsePacket(packet)) {
        QString message = getWin1251TextCodec()->toUnicode(m_packetData).toLower();
        retVal = message.mid(0, 2) == "l=" && message.mid(4, 2) == "k=";
        if (retVal) {
            m_rayNum = QString(message.at(2)).toInt();
            m_kpuNum = QString(message.at(6)).toInt();
            m_prevState = m_u1;
            m_curState = m_u2;
        }
    }
    return retVal;
}

QString SensorChangeUsk1IncomingCommand::description() const
{
    QStringList values;
    for (const QPair<int, int> &state : getChangedRelays()) {
        values.append(QString("%0:%1").arg(state.first).arg(state.second));
    }
    QString value = values.join(", ");
    return QObject::trUtf8("входящая команда: 'изменение датчиков на КПУ №%0 (луч #1): {%2}'")
            .arg(m_kpuNum)
            .arg(m_rayNum)
            .arg(value);
}

void SensorChangeUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol) {
        for (const QPair<int, int> &state : getChangedRelays()) {
            m_protocol->onSensorChanged(m_rayNum, m_kpuNum, state.first ,state.second);
        }
    }
}

QList<QPair<int, int> > SensorChangeUsk1IncomingCommand::getChangedRelays() const
{
    QList<QPair<int, int> > retVal;
    char bit = 0x01;
    // пробегаемся по всем контактам
    for (int i = 8; i != 0; --i)
    {
        // если предыдущее и текущее состояние различаеся - информируем внешний мир об этом
        if ((m_prevState ^ m_curState) & bit) {
            QPair<int, int> state(i, m_curState & bit ? 1 : 0);
            retVal.append(state);
        }
        // переходим к сл. датчику (биту)
        bit <<= 1;
    }
    return retVal;
}


QHash<QString, int> InfoUsk1IncomingCommand::m_uskInfoHash = QHash<QString, int>();

InfoUsk1IncomingCommand::InfoUsk1IncomingCommand(SendUsk1Protocol *protocol) :
    Usk1IncomingCommand(protocol)
{
    if (m_uskInfoHash.isEmpty()) {
        m_uskInfoHash[QString("включение уск")] = SendUSKv1Namespace::packetUskOn;
        m_uskInfoHash[QString("установка часов")] = SendUSKv1Namespace::packetUskSettingTime;
        m_uskInfoHash[QString("ошибка приема rs")] = SendUSKv1Namespace::packetUskErrorReceivingRS;
    }
}

bool InfoUsk1IncomingCommand::isMyPacket(const QByteArray &packet)
{
    bool retVal = false;
    if (parsePacket(packet)) {
        m_message = getWin1251TextCodec()->toUnicode(m_packetData).toLower().trimmed();
        retVal = m_uskInfoHash.contains(m_message);
    }
    return retVal;
}

QString InfoUsk1IncomingCommand::description() const
{
    QString retVal = QObject::trUtf8("");
    if (m_uskInfoHash.contains(m_message)) {
        retVal = QObject::trUtf8("входящая комманда: %0")
                .arg(m_message);
    }
    return retVal;
}

void InfoUsk1IncomingCommand::informAboutCommand()
{
    if (m_protocol && m_uskInfoHash.contains(m_message)) {
        m_protocol->onUskInfoPacketReceived(m_uskInfoHash.value(m_message));
    }
}
