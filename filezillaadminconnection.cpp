#include "filezillaadminconnection.h"
#include "md5.h"
#include <memory>

FilezillaAdminConnection::FilezillaAdminConnection(QObject *parent) :
    QObject(parent)
{
    mSocket = new QTcpSocket();
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(bytesToRead()));
    connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
}

FilezillaAdminConnection::~FilezillaAdminConnection()
{
    mSocket->abort();
    delete mSocket;
}

void FilezillaAdminConnection::connectToHost(const QString &host, int port, const QString &password)
{
    mSocket->abort();
    mDataRead.clear();
    mConnState = INIT;
    mPassword = password;
    serverDescription.isValid = false;
    mSocket->connectToHost(host, port);
}

bool FilezillaAdminConnection::SendCommand(int type, char *data, int length)
{
    QByteArray arr;
    arr.resize(length+5);
    arr[0] = type << 2;
    char *p = reinterpret_cast<char*>(&length);
    arr[1] = p[0];
    arr[2] = p[1];
    arr[3] = p[2];
    arr[4] = p[3];

    if(data)
    {
        for(int i=0; i<length; i++)
        {
            arr[i+5] = data[i];
        }
    }

    mSocket->write(arr);
    if(!mSocket->waitForBytesWritten())
    {
        emit connFail("Failed to send bytes");
        mSocket->abort();
        return false;
    }

    return true;
}

bool FilezillaAdminConnection::SendCommand(int type)
{
    QByteArray arr;
    arr.resize(5);
    arr[0] = type << 2;
    arr[1] = 0;
    arr[2] = 0;
    arr[3] = 0;
    arr[4] = 0;

    mSocket->write(arr);
    if(!mSocket->waitForBytesWritten())
    {
        emit connFail("Failed to send bytes");
        mSocket->abort();
        return false;
    }

    return false;
}

bool FilezillaAdminConnection::parseData()
{
    // Init and auth data are handled by this class
    // normal data is parsed and packed and a signal is emitted
    // that data is available to be handled.
    if(mConnState == INIT)
    {
        return parseInitData();
    }
    else if(mConnState == AUTH)
    {
        return parseAuthData();
    }

    return parseNormalData();
}

bool FilezillaAdminConnection::parseInitData()
{
    int res = serverDescription.Parse(mDataRead);
    if(res > 0)
    {
        removeParsedData(res);
        mConnState = AUTH;
        return true;
    }

    return false;
}

bool FilezillaAdminConnection::parseAuthData()
{
    if(mDataRead.length() < 5)
    {
        emit connFail("Data length below 5");
        return false;
    }
    if((mDataRead[0] & 0x03) > 2)
    {
        emit connFail("Unknown command type");
        mSocket->abort();
        return false;
    }

    int len = *reinterpret_cast<const unsigned int*>(&mDataRead.data()[1]);
    if(len + 5 <= mDataRead.length())
    {
        int iCmdId = (mDataRead[0] & 0x7C) >> 2;
        int iType = mDataRead[0] & 0x03;
        if(iCmdId == 0 && iType == 0)
        {
            if(len < 4)
            {
                emit connFail("Invalid auth data");
                mSocket->abort();
                return false;
            }

            int nonceLen1 = mDataRead[5]*256+mDataRead[6];
            if((nonceLen1+2) > (len-2))
            {
                emit connFail("Invalid auth data 2");
                mSocket->abort();
                return false;
            }

            int nonceLen2 = mDataRead[5+2+nonceLen1]*256 + mDataRead[5+2+1+nonceLen1];
            if((nonceLen1+nonceLen2+4) > len)
            {
                emit connFail("Invalid auth data 3");
                mSocket->abort();
                return false;
            }

            MD5 md5;
            if(nonceLen1)
            {
                md5.update(reinterpret_cast<const unsigned char*>(&mDataRead.data()[7]), nonceLen1);
            }

            md5.update(reinterpret_cast<const unsigned char*>(mPassword.toUtf8().data()), mPassword.length());

            if(nonceLen2)
            {
                md5.update(reinterpret_cast<const unsigned char*>(&mDataRead.data()[5+4+nonceLen1]), nonceLen2);
            }

            md5.finalize();

            //removeParsedData(len-5);

            mDataRead.clear();


            char *digest = reinterpret_cast<char*>(md5.raw_digest());
            SendCommand(0, digest, 16);
            mConnState = NORMAL;
        }
        else if(iType == 1 && iCmdId == 0)
        {
            mDataRead.clear();
            SendCommand(2);
            mConnState = NORMAL;
        }
        else
        {
            emit connFail("Protocol error, unknown command ID " + QString().setNum(iCmdId));
            mSocket->abort();
            return false;
        }

        if(mConnState == NORMAL)
        {
            emit authSuccess();
        }

        return true;
    }

    emit connFail("Invalid len: " + QString().setNum(len));
    return false;
}

bool FilezillaAdminConnection::parseNormalData()
{
    if(mDataRead.length() < 5)
    {
        return false;
    }

    int type = mDataRead[0] & 0x03;
    int id = (mDataRead[0] & 0x7C) >> 2;
    if(type > 2 || type < 1)
    {
        emit connFail("Protocol error: unknown command type");
        mSocket->abort();
    }
    else
    {
        int len = *reinterpret_cast<const int*>(&mDataRead.data()[1]);
        if(len > 0xFFFFFF)
        {
            emit connFail("Invalid data length");
            mSocket->abort();
        }
        else if(len+5 > mDataRead.length())
        {
            emit connMessage("Wrong data length");
            return false;
        }
        else
        {
            if(type == 1)
            {
                FilezillaReply reply;
                reply.type = type;
                reply.id = id;
                reply.length = len;
                for(int i=0; i<len; i++)
                {
                    reply.data.append(mDataRead[i+5]);
                }
                emit replyReceived(reply);;
            }
            else if(type == 2)
            {
                QString text = QString::fromUtf8(mDataRead.data()+6, len-1);
                if(id == 1)
                {
                    mSocket->abort();
                    emit connMessage(text);
                }
            }
            else
            {
                emit connMessage("Command type not implemented");
            }
            removeParsedData(len+5);

            return true;
        }
    }

    return false;
}


void FilezillaAdminConnection::bytesToRead()
{
    QByteArray bytes = mSocket->readAll();
    mDataRead.append(bytes);
    while(parseData())
    {
    }


}

// TODO: implement
void FilezillaAdminConnection::socketError(QAbstractSocket::SocketError error)
{
    switch(error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        emit connFail("Connection refused");
        break;

    case QAbstractSocket::NetworkError:
        emit connFail("Network error");
        break;

    case QAbstractSocket::SocketTimeoutError:
        emit connFail("Connection timeout");
        break;

    default:
        break;
    }
}


void FilezillaAdminConnection::removeParsedData(int numBytes)
{
    mDataRead = mDataRead.right(mDataRead.length()-numBytes);
}
