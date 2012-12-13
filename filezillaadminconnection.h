#ifndef FILEZILLAADMINCONNECTION_H
#define FILEZILLAADMINCONNECTION_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QByteArray>
#include "filezillaserverdesc.h"

struct FilezillaReply
{
    int type;
    int id;
    int length;
    QByteArray data;
};

class FilezillaAdminConnection : public QObject
{
    Q_OBJECT
public:
    enum ConnectionState { INIT, AUTH, NORMAL };

    explicit FilezillaAdminConnection(QObject *parent = 0);
    ~FilezillaAdminConnection();

    void connectToHost(const QString& host, int port, const QString &password);

    FilezillaServerDesc serverDescription;

    bool SendCommand(int type, char *data, int32_t length);
    bool SendCommand(int type);

private:
    QTcpSocket *mSocket;
    QByteArray mDataRead;
    ConnectionState mConnState;

    QString mPassword;

    bool parseData();
    bool parseInitData();
    bool parseAuthData();
    bool parseNormalData();

    void removeParsedData(int numBytes);
signals:
    void authSuccess();
    void connFail(const QString& message);
    void connMessage(const QString &message);
    void replyReceived(FilezillaReply reply);

private slots:
    void bytesToRead();
    void socketError(QAbstractSocket::SocketError error);
    
public slots:

    
};

#endif // FILEZILLAADMINCONNECTION_H
