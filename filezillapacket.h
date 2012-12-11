#ifndef FILEZILLAPACKET_H
#define FILEZILLAPACKET_H

#include <QByteArray>
#include <QString>
#include <cstdint>

class FilezillaPacket
{
public:
    QByteArray data;
public:
    FilezillaPacket();
    FilezillaPacket(const QByteArray &d);

    uint8_t getNextInt8();
    uint16_t getNextInt16();
    uint32_t getNextInt24();
    uint32_t getNextInt32();
    uint32_t getNextReversedInt32();


    QString getNextString();

    void addInt8(uint8_t i);
    void addInt16(uint16_t i);
    void addInt24(uint32_t i);
    void addInt32(uint32_t i);
    void addReversedInt32(uint32_t i);
    void addString(const QString &str);
};

#endif // FILEZILLAPACKET_H
