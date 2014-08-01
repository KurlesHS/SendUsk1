#include "senduskv1.h"
#include "senduskv1workingthread.h"
#include <QThread>
#include <QDebug>


SendUSKv1::SendUSKv1(QObject *parent) :
    QObject(parent)
{
    m_uskWorkingThread = new SendUSKv1WorkingThread();
    m_thread = new QThread();
    m_uskWorkingThread->moveToThread(m_thread);
    connect(m_uskWorkingThread, SIGNAL(commandAccepted(QString,QString)),
            this, SIGNAL(commandAccepted(QString,QString)));
    connect(m_uskWorkingThread, SIGNAL(detectedDisconnetcedKpu(QString,int,int)),
            this, SIGNAL(detectedDisconnetcedKpu(QString,int,int)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(detectedNewKpu(QString,int,int)),
            this, SIGNAL(detectedNewKpu(QString,int,int)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(error(QString,int)),
            this, SIGNAL(error(QString,int)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(errorOnSendingCommand(QString,QString)),
            this, SIGNAL(errorOnSendingCommand(QString,QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(portIsClose(QString,QString)),
            this, SIGNAL(portIsClose(QString,QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(portIsOpen(QString,QString)),
            this, SIGNAL(portIsOpen(QString,QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(sensorChanged(QString,int,int,int,int)),
            this, SIGNAL(sensorChanged(QString,int,int,int,int)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(startSendingCommand(QString,QString)),
            this, SIGNAL(startSendingCommand(QString,QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(unknowCommand(QString,QString)),
            this, SIGNAL(unknowCommand(QString,QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(uskInfoPacketReceived(QString,int)),
            this, SIGNAL(uskInfoPacketReceived(QString,int)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(uskIsPresent(QString,bool, bool)),
            this, SIGNAL(uskIsPresent(QString,bool,bool)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(uskReset(QString)),
            this, SIGNAL(uskReset(QString)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(uskIsAdded(QString,bool)),
            this, SIGNAL(uskIsAdded(QString,bool)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(uskIsDeleted(QString,bool)),
            this, SIGNAL(uskIsDeleted(QString,bool)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(voltageStatusChanged(QString,int,bool)),
            this, SIGNAL(voltageStatusChanged(QString,int,bool)), Qt::QueuedConnection);
    connect(m_uskWorkingThread, SIGNAL(receivedTextMessage(QString,QString)),
            this, SIGNAL(receivedTextMessage(QString,QString)));
    m_thread->start();
}

SendUSKv1::~SendUSKv1()
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "removeAllUsk", Qt::BlockingQueuedConnection);
    QMetaObject::invokeMethod(m_uskWorkingThread, "deleteLater", Qt::BlockingQueuedConnection);
    m_thread->quit();
    if (!m_thread->wait(20000))
        m_thread->terminate();
    delete m_thread;
}

void SendUSKv1::getInfoAboutUsk(QStringList &uskNameList, QStringList &portNameList, QList<int> &uskStatusList)
{
    // не очень потокобезопасно
    m_uskWorkingThread->getInfoAboutUsk(uskNameList, portNameList, uskStatusList);
}

void SendUSKv1::addUsk(const QString &uskName, const QString &portName, int uskNum)
{
    qDebug() << "add usk" << uskName;
    QMetaObject::invokeMethod(m_uskWorkingThread, "addUsk", Qt::QueuedConnection,
                              Q_ARG(QString, uskName), Q_ARG(QString, portName), Q_ARG(int, uskNum));
}

void SendUSKv1::removeUsk(const QString &uskName)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "removeUsk", Qt::QueuedConnection,
                              Q_ARG(QString, uskName));
}

void SendUSKv1::removeAllUsk()
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "removeAllUsk", Qt::QueuedConnection);
}

void SendUSKv1::openUsk(const QString &uskName)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "openUsk", Qt::QueuedConnection,
                              Q_ARG(QString, uskName));
}

void SendUSKv1::closeUsk(const QString &uskName)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "closeUsk", Qt::QueuedConnection,
                              Q_ARG(QString, uskName));
}

void SendUSKv1::sendTime(const QString &uskName, const QDateTime &time)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "sendTime", Qt::QueuedConnection,
                              Q_ARG(QString, uskName), Q_ARG(QDateTime, time));
}

void SendUSKv1::sendMessage(const QString &uskName, const QString &message)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "sendMessage", Qt::QueuedConnection,
                              Q_ARG(QString, uskName), Q_ARG(QString, message));
}

void SendUSKv1::resetUsk(const QString &uskName)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "resetUsk", Qt::QueuedConnection,
                              Q_ARG(QString, uskName));
}

void SendUSKv1::changeRelayStatus(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus, const QString &sensorName)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "changeRelayStatus", Qt::QueuedConnection,
                              Q_ARG(QString, uskName), Q_ARG(int, rayNum), Q_ARG(int, kpuNum),
                              Q_ARG(int, sensorNum), Q_ARG(int, relayStatus), Q_ARG(QString, sensorName));
}

void SendUSKv1::changeVoltageStatus(const QString &uskName, const int numOutput, const bool &on)
{
    QMetaObject::invokeMethod(m_uskWorkingThread, "changeVoltageStatus", Qt::QueuedConnection,
                              Q_ARG(QString, uskName), Q_ARG(int, numOutput), Q_ARG(bool, on));
}
