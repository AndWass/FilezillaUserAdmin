#include "filezillaaccounts.h"
#include <cstdlib>

bool FilezillaSpeedLimit::Parse(FilezillaAccountPacket& packet)
{
    if(packet.data.length() < 4+4+6+1)
    {
        return false;
    }

    int temp;
    speed = packet.getNextInt32();

    if(speed > 1048576)
    {
        speed = 1048576;
    }

    temp = packet.getNextInt32();

    if(temp == 0)
    {
        dateCheck = false;
    }
    else
    {
        year = temp >> 16;
        month = (temp >> 8) & 0x000000FF;
        day = temp & 0x000000FF;

        if(year < 1900 || year > 3000 || month < 1 || month > 12 || day < 1 || day > 31)
        {
            return false;
        }
    }

    temp = packet.getNextInt24();

    if(temp == 0)
    {
        fromCheck = false;
    }
    else
    {
        fromCheck = true;
        fromHour = temp >> 16;
        fromMinute = (temp >> 8) & 0x0000FF;
        fromSecond = temp & 0x0000FF;

        if(fromHour > 23 || fromMinute > 59 || fromSecond > 59)
        {
            return false;
        }
    }

    temp = packet.getNextInt24();

    if(temp == 0)
    {
        toCheck = false;
    }
    else
    {
        toCheck = true;
        toHour = temp >> 16;
        toMinute = (temp >> 8) & 0x0000FF;
        toSecond = temp & 0x0000FF;

        if(toHour > 23 || toMinute > 59 || toSecond > 59)
        {
            return false;
        }
    }

    weekday = packet.getNextInt8();

    return true;
}

void FilezillaSpeedLimit::FillPacket(FilezillaAccountPacket &packet)
{
    packet.addInt32(speed);
    if(dateCheck)
    {
        packet.addInt16(year);
        packet.addInt8(month);
        packet.addInt8(day);
    }
    else
    {
        packet.addInt32(0);
    }

    if(fromCheck)
    {
        packet.addInt8(fromHour);
        packet.addInt8(fromMinute);
        packet.addInt8(fromSecond);
    }
    else
    {
        packet.addInt24(0);
    }

    if(toCheck)
    {
        packet.addInt8(toHour);
        packet.addInt8(toMinute);
        packet.addInt8(toSecond);
    }
    else
    {
        packet.addInt24(0);
    }

    packet.addInt8(weekday);

}


void FilezillaGroup::FillPacket(FilezillaAccountPacket &packet)
{
    packet.addString(name);
    int temp = ipLimit;
    temp = ((temp & 0x000000FF) << 24) | ((temp & 0x0000FF00) << 8) | ((temp & 0x00FF0000) >> 8)  | ((temp & 0xFF000000) >> 24);
    packet.addInt32(temp);
    temp = userLimit;
    temp = ((temp & 0x000000FF) << 24) | ((temp & 0x0000FF00) << 8) | ((temp & 0x00FF0000) >> 8)  | ((temp & 0xFF000000) >> 24);
    packet.addInt32(temp);

    int options = bypassUserLimit & 0x03;
    options |= (enabled & 0x03) << 2;

    packet.addInt8(options);

    packet.addInt16(disallowedIps.size());
    for(unsigned int i=0; i<disallowedIps.size(); i++)
    {
        packet.addString(disallowedIps[i]);
    }

    packet.addInt16(allowedIps.size());
    for(unsigned int i=0; i<allowedIps.size(); i++)
    {
        packet.addString(allowedIps[i]);
    }

    packet.addInt16(directories.size());
    for(unsigned int i=0; i<directories.size(); i++)
    {
        directories[i].FillPacket(packet);
    }

    for(int i=0; i<2; i++)
    {
        packet.addInt8((speedLimitTypes[i] & 0x03) + ((bypassServerSpeedLimit[i] & 0x03) << 2));
        packet.addInt16(speedLimit[i]);
        packet.addInt16(speedLimits[i].size());
        for(unsigned int j=0; j<speedLimits[i].size(); j++)
        {
            speedLimits[i][j].FillPacket(packet);
        }
    }

    packet.addString(comment);
    packet.addInt8(forceSsl);
}

bool FilezillaGroup::Parse(FilezillaAccountPacket &packet)
{
    name = packet.getNextString();
    int temp = packet.getNextInt32();
    ipLimit = (temp >> 24) + ((temp & 0x00FF0000) >> 16) + ((temp & 0x0000FF00) >> 8) + ((temp & 0x000000FF));
    temp = packet.getNextInt32();
    userLimit = (temp >> 24) + ((temp & 0x00FF0000) >> 16) + ((temp & 0x0000FF00) >> 8) + ((temp & 0x000000FF));

    int options = packet.getNextInt8();
    bypassUserLimit = options & 0x03;
    enabled = (options >> 2) & 0x03;
    int numDisallowedIps = packet.getNextInt16();
    while(numDisallowedIps--)
    {
        QString ip = packet.getNextString();
        disallowedIps.push_back(ip);
    }

    int numAllowedIps = packet.getNextInt16();
    while(numAllowedIps--)
    {
        QString ip = packet.getNextString();
        allowedIps.push_back(ip);
    }

    int dirCount = packet.getNextInt16();
    bool gotHome = false;
    for(int i=0; i<dirCount; i++)
    {
        FilezillaDirectory dir;
        QString str = packet.getNextString();

        dir.dir = str;
        int aliasCnt = packet.getNextInt16();
        for(int j=0; j<aliasCnt; j++)
        {
            QString alias = packet.getNextString();
            dir.aliases.push_back(alias);
        }

        int rights = packet.getNextInt16();

        dir.dirCreate = rights & 0x0001 ? 1:0;
        dir.dirDelete = rights & 0x0002 ? 1:0;
        dir.dirList = rights & 0x0004 ? 1:0;
        dir.dirSubdirs = rights & 0x0008 ? 1:0;
        dir.fileAppend = rights & 0x0010 ? 1:0;
        dir.fileDelete = rights & 0x0020 ? 1:0;
        dir.fileRead = rights & 0x0040 ? 1:0;
        dir.fileWrite = rights & 0x0080 ? 1:0;
        dir.isHome = rights & 0x0100 ? 1:0;
        dir.autoCreate = rights & 0x0200 ? 1:0;

        if(dir.isHome)
        {
            if(!gotHome)
            {
                gotHome = true;
            }
            else
            {
                dir.isHome = false;
            }
        }

        directories.push_back(dir);

    }

    for(int i=0; i<2; i++)
    {
        int temp = packet.getNextInt8();
        speedLimitTypes[i] = temp & 0x03;
        bypassServerSpeedLimit[i] = (temp >> 2) & 0x03;

        temp = packet.getNextInt16();
        speedLimit[i] = temp;
        if(!speedLimit[i])
        {
            speedLimit[i] = 10;
        }

        temp = packet.getNextInt16();
        while(temp--)
        {
            FilezillaSpeedLimit sl;
            sl.Parse(packet);
            speedLimits[i].push_back(sl);
        }
    }

    comment = packet.getNextString();

    forceSsl = packet.getNextInt8();

    return true;

}

FilezillaGroup::~FilezillaGroup()
{
}



































FilezillaAccountPacket::FilezillaAccountPacket()
{
}

FilezillaAccountPacket::FilezillaAccountPacket(const QByteArray &d):
    data(d)
{
}

int FilezillaAccountPacket::getNextInt8()
{
    int retVal = data[0];
    data = data.right(data.length()-1);
    return retVal;
}

int FilezillaAccountPacket::getNextInt16()
{
    int retVal = data[0]*256 + data[1];
    data = data.right(data.length()-2);
    return retVal;
}

unsigned int FilezillaAccountPacket::getNextInt32()
{
    unsigned int retVal = getNextInt16();
    retVal <<= 16;
    retVal += getNextInt16();

    return retVal;
}

int FilezillaAccountPacket::getNextInt24()
{
    int retVal = getNextInt16();
    retVal <<= 8;
    retVal += getNextInt8();

    return retVal;
}

QString FilezillaAccountPacket::getNextString()
{
    QString retVal;
    int strLen = getNextInt16();
    if(strLen)
    {
        retVal = QString::fromUtf8(data.data(), strLen);
        data = data.right(data.length()-strLen);
    }
    else
    {
        retVal = "";
    }

    return retVal;
}

void FilezillaAccountPacket::addInt8(int i)
{
    data.append(static_cast<char>(i));
}

void FilezillaAccountPacket::addInt16(int i)
{
    data.append(static_cast<char>(i >> 8));
    data.append(static_cast<char>(i & 0x00FF));
}

void FilezillaAccountPacket::addInt24(int i)
{
    addInt16(i >> 8);
    addInt8(i);
}

void FilezillaAccountPacket::addInt32(int i)
{
    addInt16(i >> 16);
    addInt16(i & 0x0000FFFF);
}

void FilezillaAccountPacket::addString(const QString &str)
{
    addInt16(str.length());
    QByteArray utf8 = str.toUtf8();
    data.append(utf8);
}


void FilezillaUser::FillPacket(FilezillaAccountPacket &packet)
{
    FilezillaGroup::FillPacket(packet);
    packet.addString(username);
    packet.addString(password);
}

bool FilezillaUser::Parse(FilezillaAccountPacket &packet)
{
    FilezillaGroup::Parse(packet);
    username = packet.getNextString();
    password = packet.getNextString();

    return true;
}

FilezillaUser::~FilezillaUser()
{
}


FilezillaDirectory::FilezillaDirectory()
{
    dir = "";
    fileRead = fileWrite = fileDelete = fileAppend = false;
    dirCreate = dirDelete = dirList = dirSubdirs = isHome = false;
    autoCreate = false;
}
void FilezillaDirectory::FillPacket(FilezillaAccountPacket &packet)
{
    packet.addString(dir);
    packet.addInt16(aliases.size());
    for(unsigned int i=0; i<aliases.size(); i++)
    {
        packet.addString(aliases[i]);
    }

    int rights = 0;
    rights |= dirCreate ? 0x0001:0;
    rights |= dirDelete ? 0x0002:0;
    rights |= dirList ? 0x0004:0;
    rights |= dirSubdirs ? 0x0008:0;
    rights |= fileAppend ? 0x0010:0;
    rights |= fileDelete ? 0x0020:0;
    rights |= fileRead ? 0x0040:0;
    rights |= fileWrite ? 0x0080:0;
    rights |= isHome ? 0x0100:0;
    rights |= autoCreate ? 0x0200:0;

    packet.addInt16(rights);
}
