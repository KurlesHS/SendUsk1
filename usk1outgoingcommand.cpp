#include "usk1outgoingcommand.h"
#include <QSerialPort>
#include <QTextCodec>

Usk1OutgoingCommand::Usk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum, const int attempts) :
    m_serialPort(serialPort),
    m_attempts(attempts),
    m_isFirstAttempt(true),
    m_uskNumber(uskNum)
{
}

Usk1OutgoingCommand::~Usk1OutgoingCommand()
{
}

bool Usk1OutgoingCommand::needToWaitCommand() const
{
    return false;
}

bool Usk1OutgoingCommand::needToInformAboutStartSending() const
{
    return true;
}

bool Usk1OutgoingCommand::isAnotherAttemptPresent() const
{
    return m_attempts > 0;
}

bool Usk1OutgoingCommand::isFirstAttempt() const
{
    return m_isFirstAttempt;
}

void Usk1OutgoingCommand::sendCommand()
{
    m_isFirstAttempt = false;
    if (m_serialPort && m_serialPort->isOpen()) {
        --m_attempts;
        m_serialPort->write(outgoingBinaryPacket());
    }
}

int Usk1OutgoingCommand::uskNumber() const
{
    return m_uskNumber;
}

QTextCodec *Usk1OutgoingCommand::getWin1251TextCodec()
{
    static QTextCodec *codecWin1251 = QTextCodec::codecForName("Windows-1251");
    return codecWin1251;
}


SendTimeUsk1OutgoingCommand::SendTimeUsk1OutgoingCommand(QSerialPort *serialPort,
                                                         const int &uskNum,
                                                         const int attempts,
                                                         const QDateTime &dateTime) :
    Usk1OutgoingCommand(serialPort, uskNum, attempts),
    m_dateTime(dateTime)
{

}

QString SendTimeUsk1OutgoingCommand::description() const
{
    return QObject::trUtf8("команда установки времени (%0)")
            .arg(m_dateTime.toString());
}

QByteArray SendTimeUsk1OutgoingCommand::outgoingBinaryPacket() const
{
    QByteArray res;
    res.append(getFirstPartOfPacket(2, 0));
    QString day = QString("%0").arg(m_dateTime.date().day(), 2, 10, QChar('0'));
    QString month = QString("%0").arg(m_dateTime.date().month(), 2, 10, QChar('0'));
    QString year = QString::number(m_dateTime.date().year() % 10);
    QString hour = QString("%0").arg(m_dateTime.time().hour(), 2, 10, QChar('0'));
    QString minute = QString("%0").arg(m_dateTime.time().minute(), 2, 10, QChar('0'));
    QString second = QString("%0").arg(m_dateTime.time().second(), 2, 10, QChar('0'));
    QString stringDate;
    stringDate.append(day).append('/').append(month).append('/').append(year).append(' ')
            .append(hour).append(':').append(minute).append(':').append(second);
    res.append(stringDate);
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    appendCrcToPacket(res);
    return res;
}

bool SendTimeUsk1OutgoingCommand::needToInformAboutStartSending() const
{
    return false;
}

QByteArray Usk1OutgoingCommand::getFirstPartOfPacket(const quint64 &flags, const quint8 &priority) const
{
    QByteArray res;
    const ushort uskNum = uskNumber();
    res.append(static_cast<char>(0x00));
    res.append(uskNum & 0xff);
    res.append((uskNum >> 8) & 0xff);
    res.append(flags & 0xff);
    res.append((flags >> 8) & 0xff);
    res.append((flags >> 16) & 0xff);
    res.append((flags >> 24) & 0xff);
    res.append(priority);
    return res;
}

void Usk1OutgoingCommand::appendCrcToPacket(QByteArray &packet) const
{
    char crc = 0x00;
    for (int i = 0; i < packet.length(); ++i)
        crc += packet.at(i);
    packet.append(crc);
}


SendMessageUsk1OutgoingCommand::SendMessageUsk1OutgoingCommand(QSerialPort *serialPort,
                                                               const int &uskNum,
                                                               const int attempts,
                                                               const QString &message) :
    Usk1OutgoingCommand(serialPort, uskNum, attempts),
    m_message(message)
{

}

QString SendMessageUsk1OutgoingCommand::description() const
{
    return QObject::trUtf8("команда посылки сообщения: '#0'")
            .arg(m_message);
}

QByteArray SendMessageUsk1OutgoingCommand::outgoingBinaryPacket() const
{
    QByteArray res;
    res.append(getFirstPartOfPacket(0x00000004, 0));
    QByteArray cp1251Message;
    cp1251Message.resize(16);
    cp1251Message.fill(' ');
    cp1251Message.replace(0, m_message.length() > 16 ? 16 : m_message.length(), getWin1251TextCodec()->fromUnicode(m_message.left(16)));
    res.append(cp1251Message);
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    appendCrcToPacket(res);
    return res;
}


ResetUsk1OutgoingCommand::ResetUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum, const int attempts) :
    Usk1OutgoingCommand(serialPort, uskNum, attempts)
{

}

QString ResetUsk1OutgoingCommand::description() const
{
    return QObject::trUtf8("команда сброса УСК");
}

QByteArray ResetUsk1OutgoingCommand::outgoingBinaryPacket() const
{
    QByteArray res;
    res.resize(26);
    res.fill(static_cast<char>(0x00));
    res[1] = static_cast<char>(uskNumber() & 0xff);
    res[2] = static_cast<char>((uskNumber() >> 8) & 0xff);
    res[3] = 0x01;
    appendCrcToPacket(res);
    return res;
}

bool ResetUsk1OutgoingCommand::needToWaitCommand() const
{
    return true;
}


ChangeRelayUsk1OutgoingCommand::ChangeRelayUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                                                               const int attempts, const int &rayNum,
                                                               const int &kpuNum, const int &sensorNum,
                                                               const int &relayStatus, const QString &sensorName) :
    Usk1OutgoingCommand(serialPort, uskNum, attempts),
    m_rayNum(rayNum),
    m_kpuNum(kpuNum),
    m_sensorNum(sensorNum),
    m_relayStatus(relayStatus),
    m_sensorName(sensorName)
{

}

QString ChangeRelayUsk1OutgoingCommand::description() const
{
    return QObject::trUtf8("команда изменения состояния исп. устр-ва \"%0\" на лог. \"%1\"")
            .arg(m_sensorName)
            .arg(m_relayStatus == 0 ? QObject::trUtf8("0") : QObject::trUtf8("1"));
}

QByteArray ChangeRelayUsk1OutgoingCommand::outgoingBinaryPacket() const
{
    QByteArray res;
    res.append(getFirstPartOfPacket(0x00001000, 0));
    QString textCommand = QObject::trUtf8("КПУ %0/%1/%2/%3")
            .arg(m_rayNum % 10)
            .arg(m_kpuNum % 10)
            .arg(m_sensorNum % 10)
            .arg(m_relayStatus == 1 ? QObject::trUtf8("Вкл 0 ") : QObject::trUtf8("Выкл 0"));
    QByteArray encodedCommand = getWin1251TextCodec()->fromUnicode(textCommand);
    res.append(encodedCommand);
    res.append(static_cast<char>(m_rayNum));
    res.append(static_cast<char>(m_kpuNum * 0x10 + ((m_sensorNum - 1) << 1) + m_relayStatus));
    appendCrcToPacket(res);
    return res;
}


ChangeVoltageUsk1OutgoingCommand::ChangeVoltageUsk1OutgoingCommand(QSerialPort *serialPort, const int &uskNum,
                                                                   const int attempts, const int numOutput, const bool &on) :
    Usk1OutgoingCommand(serialPort, uskNum, attempts),
    m_numOutput(numOutput),
    m_on(on)
{

}

QString ChangeVoltageUsk1OutgoingCommand::description() const
{
    return   QObject::trUtf8("команда %2 НЧ выхода номер %1")
            .arg(m_numOutput)
            .arg(m_on ? QObject::trUtf8("включения") : QObject::trUtf8("выключения"));
}

QByteArray ChangeVoltageUsk1OutgoingCommand::outgoingBinaryPacket() const
{
    quint64 flag;
    switch (m_numOutput) {
    case 2:
        flag = m_on ? 0x00002000 : 0x00004000;
        break;
    default:
        flag = m_on ? 0x00000020 : 0x00000100;
        break;
    }
    QByteArray res = getFirstPartOfPacket(flag, 0);
    res.append("                ");
    res.append((char)0x00);
    res.append((char)0x00);
    appendCrcToPacket(res);
    return res;
}
