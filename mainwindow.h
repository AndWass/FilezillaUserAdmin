#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include "filezillaadminconnection.h"
#include "filezillaaccounts.h"
#include "filezillausermodel.h"
#include <vector>
#include <QSharedPointer>
#include "filezillaaccountcontroller.h"

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
    void connectionClosed();
    void serverReply(FilezillaReply reply);

    void on_btnAddUser_clicked();

    void on_btnGeneratePass_clicked();

    void on_btnDelete_clicked();

    void on_tvUsers_clicked(const QModelIndex &index);

    void on_tvUsers_activated(const QModelIndex &index);

    void on_tvUsers_customContextMenuRequested(const QPoint &pos);

    void on_actionDelete_user_triggered();

    void on_actionEdit_user_directories_triggered();

    void action_group_changed(bool checked);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QSettings *getSettings();
    void userSelectionChanged(const QModelIndex &index);
    void deleteSelectedUser();
    QSharedPointer<QMenu> usersContextMenu;
    QSharedPointer<QMenu> usersGroupsMenu;

    QString getSelectedUser();

    void handleAccountSettingReply(FilezillaReply &reply);

    void buildGroupsMenu();
    void checkSelectedGroup(const QString &groupName);

    void loadSettings();

    QString getNoneGroupMenuText();

    Ui::MainWindow *ui;
    FilezillaAdminConnection conn;

    FilezillaUserModel *userModel;

    FilezillaAccountController accountController;
};

#endif // MAINWINDOW_H
