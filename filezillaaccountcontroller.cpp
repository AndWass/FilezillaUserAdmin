#include "filezillapacket.h"
#include "filezillaaccountcontroller.h"
#include "md5.h"
#include <QDir>



void FilezillaAccountController::handleAccountSettingsReply(FilezillaReply reply)
{
    if(reply.length < 2)
    {
        return;
    }
    else
    {
        FilezillaPacket packet(reply.data);
        unsigned int num = packet.getNextInt16();
        groups.clear();
        for(unsigned int i=0; i<num; i++)
        {
            FilezillaGroup grp;
            grp.Parse(packet);
            groups.push_back(grp);
        }

        num = packet.getNextInt16();

        userModel->clear();
        while(num--)
        {
            FilezillaUser user;
            user.Parse(packet);
            userModel->pushBack(user);
        }
    }
}

void FilezillaAccountController::updateExistingUser(FilezillaUser *p, const QString &password)
{
    QString pass = getMD5Password(password);
    p->password = pass;
}

void FilezillaAccountController::addNewUser(const QString &username, const QString &password, bool createServerDir)
{
    QString lower = username.toLower();
    QString startUpper = lower;
    startUpper[0] = lower[0].toUpper();
    FilezillaUser ret;
    ret.name = "";
    ret.ipLimit = 0;
    ret.userLimit = 0;
    ret.bypassUserLimit = 0;
    ret.enabled = 1;

    FilezillaDirectory dir;
    dir.isHome = true;
    dir.fileRead = true;
    dir.dirList = true;
    dir.dirSubdirs = true;
    dir.dir = "F:\\Organisation\\FTP\\" + startUpper;

    if(createServerDir)
    {
        QDir realDir("M:/");
        realDir.mkpath(startUpper + "/" + "# Customer uploads");
    }

    ret.directories.push_back(dir);

    dir.dir = "F:\\Organisation\\FTP\\" + startUpper + "\\# Customer uploads";
    dir.fileAppend = true;
    dir.fileDelete = true;
    dir.fileWrite = true;
    dir.dirCreate = true;
    dir.dirDelete = true;

    ret.directories.push_back(dir);

    ret.speedLimitTypes[0] = ret.speedLimitTypes[1] = 0;
    ret.bypassServerSpeedLimit[0] = ret.bypassServerSpeedLimit[1] = 0;
    ret.speedLimit[0] = ret.speedLimit[1] = 10;

    ret.comment = "";
    ret.forceSsl = 0;

    ret.username = lower;

    QString pass = getMD5Password(password);
    ret.password = pass;

    userModel->pushBack(ret);
}

QString FilezillaAccountController::getMD5Password(const QString &plaintext)
{
    QByteArray utf8 = plaintext.toUtf8();
    MD5 md5;
    md5.update(reinterpret_cast<const unsigned char*>(utf8.data()), plaintext.length());
    md5.finalize();
    QString pass = QString::fromAscii(md5.hex_digest());
    return pass;
}

FilezillaAccountController::FilezillaAccountController(FilezillaAdminConnection *connection):
    conn(connection)
{
    userModel = new FilezillaUserModel(&users);
    connect(conn, SIGNAL(replyReceived(FilezillaReply)), this, SLOT(replyReceived(FilezillaReply)));
}

FilezillaUserModel *FilezillaAccountController::getUserModel()
{
    return userModel;
}

void FilezillaAccountController::userSetGroup(const QString &username, const QString &groupName)
{
    FilezillaUser *p = getUser(username);
    if(p)
    {
        p->name = groupName;
    }

    saveAccountSettings();
    updateAccountSettings();
}

QString FilezillaAccountController::userGetGroup(const QString &username)
{
    FilezillaUser *p = getUser(username);
    if(p)
    {
        return p->name;
    }

    return "";
}

void FilezillaAccountController::createOrUpdateUser(const QString &username, const QString &password, bool createServerDir)
{
    FilezillaUser *p = getUser(username);
    if(p)
    {
        updateExistingUser(p, password);
    }
    else
    {
        addNewUser(username, password, createServerDir);
    }

    saveAccountSettings();
    updateAccountSettings();
}

void FilezillaAccountController::removeUser(const QString &username)
{
    for(unsigned int i=0; i<users.size(); i++)
    {
        if(users[i].username == username)
        {
            userModel->removeAt(i);
            break;
        }
    }

    saveAccountSettings();
    updateAccountSettings();
}

void FilezillaAccountController::setUserDirectories(const QString &username, const vector<FilezillaDirectory> directories)
{
    FilezillaUser *p = getUser(username);
    if(p)
    {
        p->directories = directories;
        saveAccountSettings();
        updateAccountSettings();
    }
}

vector<FilezillaDirectory> FilezillaAccountController::getUserDirectories(const QString &username)
{
    FilezillaUser *p = getUser(username);
    if(p)
    {
        return p->directories;
    }

    return std::vector<FilezillaDirectory>();
}


void FilezillaAccountController::replyReceived(FilezillaReply reply)
{
    if(reply.id == 6)
    {
        handleAccountSettingsReply(reply);
    }
}


void FilezillaAccountController::saveAccountSettings()
{
    FilezillaPacket packet;
    packet.addInt16(groups.size());
    for(unsigned int i=0; i<groups.size(); i++)
    {
        groups[i].FillPacket(packet);
    }

    packet.addInt16(users.size());
    for(unsigned int i=0; i<users.size(); i++)
    {
        users[i].FillPacket(packet);
    }

    conn->SendCommand(6, packet.data.data(), packet.data.length());
}

void FilezillaAccountController::updateAccountSettings()
{
    conn->SendCommand(6, 0, 0);
}


std::vector<QString> FilezillaAccountController::getGroupNames()
{
    std::vector<QString> retVal;
    for(unsigned int i=0; i<groups.size(); i++)
    {
        retVal.push_back(groups[i].name);
    }

    return retVal;
}


FilezillaUser *FilezillaAccountController::getUser(const QString &username)
{
    FilezillaUser *retVal = NULL;
    for(unsigned int i=0; i<users.size(); i++)
    {
        if(users[i].username == username)
        {
            retVal = &users[i];
            break;
        }
    }

    return retVal;
}
