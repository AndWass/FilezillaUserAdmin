#ifndef FILEZILLASERVERDESC_H
#define FILEZILLASERVERDESC_H

#include <QByteArray>

class FilezillaServerDesc
{
public:
    FilezillaServerDesc();

    int Parse(const QByteArray &bytes);
    bool isValid;
    unsigned int serverVersion;
    unsigned int protocolVersion;
};

#endif // FILEZILLASERVERDESC_H
