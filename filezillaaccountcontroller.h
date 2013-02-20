#ifndef FILEZILLAACCOUNTCONTROLLER_H
#define FILEZILLAACCOUNTCONTROLLER_H

#include "filezillaadminconnection.h"
#include "filezillausermodel.h"
#include "filezillaaccounts.h"
#include <vector>
#include <QSharedPointer>
#include <QString>
#include <QObject>

class FilezillaAccountController: public QObject
{
    Q_OBJECT

    FilezillaAdminConnection *conn;
    FilezillaUserModel* userModel;

    std::vector<FilezillaUser> users;
    std::vector<FilezillaGroup> groups;

    FilezillaUser *getUser(const QString &username);

    void handleAccountSettingsReply(FilezillaReply reply);

    void updateExistingUser(FilezillaUser *p, const QString &password);
    bool addNewUser(const QString &username, const QString &password, bool createServerDir);

    QString getMD5Password(const QString &plaintext);
public:
    FilezillaAccountController(FilezillaAdminConnection *connection);

    FilezillaUserModel *getUserModel();

    void userSetGroup(const QString &username, const QString &groupName);
    QString userGetGroup(const QString &username);
    bool createOrUpdateUser(const QString &username, const QString &password, bool createServerDir);
    void removeUser(const QString &username);

    void setUserDirectories(const QString &username, const std::vector<FilezillaDirectory> directories);
    std::vector<FilezillaDirectory> getUserDirectories(const QString &username);

    std::vector<QString> getGroupNames();

    void saveAccountSettings();
    void updateAccountSettings();

private slots:
    void replyReceived(FilezillaReply reply);
};

#endif // FILEZILLAACCOUNTCONTROLLER_H
