#include "filezillaserverdesc.h"

FilezillaServerDesc::FilezillaServerDesc():
    isValid(false), serverVersion(0), protocolVersion(0)
{
}

/**
 * @brief Parses a server description from filezilla.
 * @param The bytes first received from the filezilla server when connecting.
 * @return -1 if parsing failed, otherwise the number of bytes that was parsed, always more than 0.
 */
int FilezillaServerDesc::Parse(const QByteArray &bytes)
{
    isValid = false;    // Assume incorrect until proven otherwise.
    if(bytes.length() < 15)
    {
        return -1;
    }
    if(bytes[0] == 'F' && bytes[1] == 'Z' && bytes[2] == 'S')
    {
        unsigned int uiTemp = bytes[3]*256 + bytes[4];
        if(uiTemp != 4)
        {
            return -1;
        }
        uiTemp = *reinterpret_cast<const unsigned int*>(&bytes.data()[5]);
        serverVersion = uiTemp;
        uiTemp = bytes[9]*256 + bytes[10];
        if(uiTemp != 4)
        {
            return -1;
        }
        uiTemp = *reinterpret_cast<const unsigned int*>(&bytes.data()[11]);
        protocolVersion = uiTemp;
        isValid = true;
    }
    else
    {
        return -1;
    }

    return 15;
}
