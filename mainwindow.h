#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include "filezillaadminconnection.h"
#include "filezillaaccounts.h"
#include "filezillausermodel.h"
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_btnConnect_clicked();

    void on_btnUpdate_clicked();

    void connectionAuthenticated();
    void connectionFailed(const QString &reason);
    void connectionMessage(const QString &message);
    void serverReply(FilezillaReply reply);

    void on_btnAddUser_clicked();

    void on_btnGeneratePass_clicked();

    void on_btnDelete_clicked();

    void on_tvUsers_clicked(const QModelIndex &index);

    void on_tvUsers_activated(const QModelIndex &index);

    void on_tvUsers_customContextMenuRequested(const QPoint &pos);

    void on_actionDelete_user_triggered();

    void on_actionEdit_user_directories_triggered();

private:
    void deleteSelectedUser();
    QMenu *usersContextMenu;
    std::vector<FilezillaUser> users;
    std::vector<FilezillaGroup> groups;
    FilezillaUser getUser(const QString company, const QString password);

    FilezillaUser *getSelectedUser();

    void handleAccountSettingReply(FilezillaReply &reply);
    void updateAccountSettings();
    void sendAccountSettings();

    Ui::MainWindow *ui;
    FilezillaAdminConnection conn;

    FilezillaUserModel *userModel;
};

#endif // MAINWINDOW_H
