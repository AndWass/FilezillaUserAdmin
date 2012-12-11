#ifndef FILEZILLAACCOUNTS_H
#define FILEZILLAACCOUNTS_H

#include <QByteArray>
#include <QString>
#include <vector>
#include "filezillapacket.h"

class FilezillaSpeedLimit
{
public:
    bool Parse(FilezillaPacket &packet);
    void FillPacket(FilezillaPacket &packet);

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
    void FillPacket(FilezillaPacket &packet);
    QString dir;
    std::vector<QString> aliases;
    bool fileRead, fileWrite, fileDelete,fileAppend;
    bool dirCreate, dirDelete, dirList, dirSubdirs, isHome;
    bool autoCreate;
};


class FilezillaGroup
{
public:
    virtual void FillPacket(FilezillaPacket &packet);
    virtual bool Parse(FilezillaPacket &packet);
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
    virtual void FillPacket(FilezillaPacket &packet);
    virtual bool Parse(FilezillaPacket &packet);
    virtual ~FilezillaUser();

    QString username;
    QString password;
};

#endif // FILEZILLAACCOUNTS_H
