#ifndef FILEZILLAACCOUNTS_H
#define FILEZILLAACCOUNTS_H

#include <QByteArray>
#include <QString>
#include <vector>

class FilezillaAccountPacket
{
public:
    QByteArray data;
public:
    FilezillaAccountPacket();
    FilezillaAccountPacket(const QByteArray &d);

    int getNextInt8();
    int getNextInt16();
    unsigned int getNextInt32();
    int getNextInt24();

    QString getNextString();

    void addInt8(int i);
    void addInt16(int i);
    void addInt24(int i);
    void addInt32(int i);
    void addString(const QString &str);
};

class FilezillaSpeedLimit
{
public:
    bool Parse(FilezillaAccountPacket &packet);
    void FillPacket(FilezillaAccountPacket &packet);

    bool dateCheck;
    bool fromCheck;
    bool toCheck;

    int year, month, day;
    int fromHour, fromMinute, fromSecond;
    int toHour, toMinute, toSecond;

    int speed;

    int weekday;

};

class FilezillaDirectory
{
public:
    FilezillaDirectory();
    void FillPacket(FilezillaAccountPacket &packet);
    QString dir;
    std::vector<QString> aliases;
    bool fileRead, fileWrite, fileDelete,fileAppend;
    bool dirCreate, dirDelete, dirList, dirSubdirs, isHome;
    bool autoCreate;
};


class FilezillaGroup
{
public:
    virtual void FillPacket(FilezillaAccountPacket &packet);
    virtual bool Parse(FilezillaAccountPacket &packet);
    virtual ~FilezillaGroup();

    QString name;
    unsigned int ipLimit;
    unsigned int userLimit;
    unsigned int bypassUserLimit;
    unsigned int enabled;

    std::vector<QString> disallowedIps;
    std::vector<QString> allowedIps;
    std::vector<FilezillaDirectory> directories;

    QString comment;

    int speedLimitTypes[2];
    int bypassServerSpeedLimit[2];
    int speedLimit[2];  // Dafuck??

    std::vector<FilezillaSpeedLimit> speedLimits[2];

    bool forceSsl;
};

class FilezillaUser: public FilezillaGroup
{
public:
    virtual void FillPacket(FilezillaAccountPacket &packet);
    virtual bool Parse(FilezillaAccountPacket &packet);
    virtual ~FilezillaUser();

    QString username;
    QString password;
};

#endif // FILEZILLAACCOUNTS_H
