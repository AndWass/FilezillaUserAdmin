#include "filezillapacket.h"

FilezillaPacket::FilezillaPacket()
{
}

FilezillaPacket::FilezillaPacket(const QByteArray &d):
    data(d)
{
}

uint8_t FilezillaPacket::getNextInt8()
{
    uint8_t retVal = data[0];
    data = data.right(data.length()-1);
    return retVal;
}

uint16_t FilezillaPacket::getNextInt16()
{
    uint16_t retVal = data[0]*256 + data[1];
    data = data.right(data.length()-2);
    return retVal;
}

uint32_t FilezillaPacket::getNextInt32()
{
    uint32_t retVal = getNextInt16();
    retVal <<= 16;
    retVal += getNextInt16();

    return retVal;
}

uint32_t FilezillaPacket::getNextReversedInt32()
{
    uint32_t retVal, temp = getNextInt32();
    retVal = ((temp >> 24) & 0x000000FF) | ((temp >> 8) & 0x0000FF00) | ((temp << 8) & 0x00FF0000) | ((temp << 24) & 0xFF000000);
    return retVal;
}

uint32_t FilezillaPacket::getNextInt24()
{
    uint32_t retVal = getNextInt16();
    retVal <<= 8;
    retVal += getNextInt8();

    return retVal;
}

QString FilezillaPacket::getNextString()
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

void FilezillaPacket::addInt8(uint8_t i)
{
    data.append(static_cast<char>(i));
}

void FilezillaPacket::addInt16(uint16_t i)
{
    data.append(static_cast<char>(i >> 8));
    data.append(static_cast<char>(i & 0x00FF));
}

void FilezillaPacket::addInt24(uint32_t i)
{
    addInt16(i >> 8);
    addInt8(i);
}

void FilezillaPacket::addInt32(uint32_t i)
{
    addInt16(i >> 16);
    addInt16(i & 0x0000FFFF);
}

void FilezillaPacket::addReversedInt32(uint32_t i)
{
    uint32_t iReversed = ((i & 0x000000FF) << 24) | ((i & 0x0000FF00) << 8) | ((i & 0x00FF0000) >> 8)  | (((i & 0xFF000000) >> 24) & 0x000000FF);
    addInt32(iReversed);
}

void FilezillaPacket::addString(const QString &str)
{
    addInt16(str.length());
    QByteArray utf8 = str.toUtf8();
    data.append(utf8);
}
