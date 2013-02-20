#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editdirectoriesdialog.h"
#include <string>
#include <cstdlib>
#include <ctime>
#include <QMessageBox>
#include <QCommonStyle>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    usersContextMenu(new QMenu()),
    usersGroupsMenu(new QMenu()),
    ui(new Ui::MainWindow),
    conn(),
    accountController(&conn)
{
    srand(time(0));
    ui->setupUi(this);
    ui->tvUsers->setModel(accountController.getUserModel());
    connect(&conn, SIGNAL(authSuccess()), this, SLOT(connectionAuthenticated()));
    connect(&conn, SIGNAL(connFail(QString)), this, SLOT(connectionFailed(QString)));
    connect(&conn, SIGNAL(connMessage(QString)), this, SLOT(connectionMessage(QString)));
    connect(&conn, SIGNAL(replyReceived(FilezillaReply)), this, SLOT(serverReply(FilezillaReply)));
    usersContextMenu->addAction(ui->actionEdit_user_directories);
    usersContextMenu->addAction(ui->actionDelete_user);
    usersContextMenu->addAction(ui->actionGroup_membership);
    ui->actionGroup_membership->setMenu(usersGroupsMenu.data());

    userModel = accountController.getUserModel();

    buildGroupsMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnConnect_clicked()
{
    conn.connectToHost(ui->leHost->text(), ui->lePort->text().toInt(), ui->leConnPassword->text());
}

void MainWindow::on_btnUpdate_clicked()
{
    accountController.updateAccountSettings();
}

void MainWindow::connectionAuthenticated()
{
    ui->textArea->appendPlainText("Authenticating");
    ui->textArea->update();
}

void MainWindow::connectionFailed(const QString &reason)
{
    ui->textArea->appendPlainText("Connection error: " + reason);
    ui->textArea->update();
}

void MainWindow::connectionMessage(const QString &message)
{
    ui->textArea->appendPlainText(message);
    ui->textArea->update();
}

void MainWindow::serverReply(FilezillaReply reply)
{
    if(reply.id == 0)
    {
        accountController.updateAccountSettings();
    }
    else if(reply.id == 6)
    {
        handleAccountSettingReply(reply);
    }
}

QString MainWindow::getSelectedUser()
{
    QModelIndexList selected = ui->tvUsers->selectionModel()->selectedIndexes();
    if(selected.size() == 0)
    {
        return "";
    }

    return accountController.getUserModel()->data(selected[0], Qt::DisplayRole).toString();
}

void MainWindow::handleAccountSettingReply(FilezillaReply &reply)
{
    if(reply.length == 1)
    {
        if((char)reply.data[0] == 0)
        {
            ui->textArea->appendPlainText("Account settings successfully sent.");
        }
        else if((char)reply.data[0] == 1)
        {
            ui->textArea->appendPlainText("Failed to send account settings.");
        }

        return;
    }

    ui->textArea->appendPlainText("Account settings received");
}

void MainWindow::buildGroupsMenu()
{
    std::vector<QString> groups = accountController.getGroupNames();
    usersGroupsMenu->clear();
    QAction *nonAction = new QAction(usersGroupsMenu.data());
    nonAction->setText(getNoneGroupMenuText());
    nonAction->setCheckable(true);
    nonAction->setChecked(false);
    connect(nonAction, SIGNAL(triggered(bool)), this, SLOT(action_group_changed(bool)));
    usersGroupsMenu->addAction(nonAction);
    for(unsigned int i=0; i<groups.size(); i++)
    {
        QAction* ac = new QAction(usersGroupsMenu.data());
        ac->setText(groups[i]);
        ac->setCheckable(true);
        ac->setChecked(false);
        connect(ac, SIGNAL(triggered(bool)), this, SLOT(action_group_changed(bool)));

        usersGroupsMenu->addAction(ac);
    }

    checkSelectedGroup(accountController.userGetGroup(ui->leUsername->text()));
}

void MainWindow::checkSelectedGroup(const QString &groupName)
{
    if(groupName == "")
    {
        usersGroupsMenu->actions()[0]->setChecked(true);
    }
    else
    {
        bool found = false;
        QList<QAction*> actions = usersGroupsMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            if(actions[i]->text() == groupName)
            {
                found = true;
                actions[i]->setChecked(true);
                break;
            }
        }

        if(!found)
        {
            // If not found make it check none
            checkSelectedGroup("");
        }
    }
}

QString MainWindow::getNoneGroupMenuText()
{
    return "-- None --";
}

void MainWindow::on_btnAddUser_clicked()
{
    bool res = accountController.createOrUpdateUser(ui->leUsername->text(), ui->lePassword->text(), ui->cbCreateServerDir->isChecked());
    if(!res)
    {
        QMessageBox mbox;
        mbox.setText("Failed to add or update user. Make sure username and password are not empty.");
        mbox.setWindowTitle("Error");
        mbox.exec();
    }
}

void MainWindow::on_btnGeneratePass_clicked()
{
    // Valid ranges: 48 - 57, 65-90, 97-122
    // corresponding to 0-9, A-Z, a-z in ASCII.

    QString passwd = "";
    char temp;
    while(true)
    {
        temp =  (rand() % 75) + 48; // Generate a number between 48 and 122
        if((temp >= 48 && temp <= 57) ||
                (temp >= 65 && temp <= 90) ||
                (temp >= 97 && temp <= 122))
        {
            passwd += QChar::fromAscii(temp);
        }

        // Break when the password is 8 chars.
        if(passwd.length() == 8)
        {
            break;
        }
    }

    ui->lePassword->setText(passwd);
}

void MainWindow::on_btnDelete_clicked()
{
    deleteSelectedUser();
}

void MainWindow::on_tvUsers_clicked(const QModelIndex &index)
{
    userSelectionChanged(index);
}

void MainWindow::on_tvUsers_activated(const QModelIndex &index)
{
    userSelectionChanged(index);
}

void MainWindow::on_tvUsers_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tvUsers->indexAt(pos);
    if(index.isValid())
    {
        if(userModel->flags(index) & Qt::ItemIsSelectable)
        {
            QPoint global = ui->tvUsers->mapToGlobal(pos);
            usersContextMenu->popup(global);
            buildGroupsMenu();
        }
    }
}

void MainWindow::on_actionDelete_user_triggered()
{
    deleteSelectedUser();
}

void MainWindow::deleteSelectedUser()
{
    QString username = getSelectedUser();
    if(username.size() > 0)
    {
        QMessageBox mb;
        mb.setText("Are you sure you want to delete user " + username + "? This can not be undone.");
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        mb.setIcon(QMessageBox::Warning);
        if(mb.exec() == QMessageBox::Yes)
        {
            accountController.removeUser(username);
        }
    }
}

void MainWindow::on_actionEdit_user_directories_triggered()
{
    QString selected = getSelectedUser();
    if(selected == "")
    {
        return;
    }
    EditDirectoriesDialog diag(accountController.getUserDirectories(selected));
    int res = diag.exec();
    if(res == QDialog::Accepted)
    {
        accountController.setUserDirectories(selected, diag.getDirectories());
    }
}

void MainWindow::action_group_changed(bool checked)
{
    QString user = getSelectedUser();
    QString newGroup = "";
    if(checked)
    {
        QString oldGroup = accountController.userGetGroup(user);
        if(oldGroup == "")
        {
            oldGroup = getNoneGroupMenuText();
        }
        QList<QAction *> actions = usersGroupsMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            if(actions[i]->text() == oldGroup)
            {
                continue;
            }

            if(actions[i]->isChecked())
            {
                newGroup = actions[i]->text();
                if(newGroup == getNoneGroupMenuText())
                {
                    newGroup = "";
                }
                break;
            }
        }
    }

    accountController.userSetGroup(user, newGroup);
}

void MainWindow::userSelectionChanged(const QModelIndex &index)
{
    ui->btnDelete->setEnabled(false);
    if(ui->tvUsers->model()->flags(index) & Qt::ItemIsSelectable)
    {
        QVariant username = userModel->data(index, Qt::DisplayRole);
        if(username.isValid())
        {
            ui->leUsername->setText(username.toString());
            ui->btnDelete->setEnabled(true);
        }
    }
}
