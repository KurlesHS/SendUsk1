#ifndef SENDUSKV1WORKINGTHREAD_H
#define SENDUSKV1WORKINGTHREAD_H

#include <QObject>
#include <QDateTime>
#include <QHash>

class SendUsk1Protocol;
class SendUSKv1WorkingThread : public QObject
{
    Q_OBJECT
public:
    explicit SendUSKv1WorkingThread(QObject *parent = 0);
    void getInfoAboutUsk(QStringList &uskNameList, QStringList &portNameList, QList<int> &uskStatusList);

public slots:
    void addUsk(const QString &uskName, const QString &portName, int uskNum);
    void removeUsk(const QString &uskName);
    void openUsk(const QString &uskName);
    void closeUsk(const QString &uskName);
    void sendTime(const QString &uskName, const QDateTime &time);
    void sendMessage(const QString &uskName, const QString &message);
    void resetUsk(const QString &uskName);
    void changeRelayStatus(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus, const QString &sensorName);
    void changeVoltageStatus(const QString &uskName, const int numOutput, const bool &on);
    void removeAllUsk();

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
    void uskIsAdded(const QString &uskName, const bool &added);
    void uskIsDeleted(const QString &uksName, const bool &deleted);

signals:

private:
    QHash<QString, SendUsk1Protocol*> m_hashOfUsk;

};

#endif // SENDUSKV1WORKINGTHREAD_H
