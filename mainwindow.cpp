#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filezillaserverdesc.h"
#include "filezillaaccounts.h"
#include "editdirectoriesdialog.h"
#include "md5.h"
#include <QtNetwork/QTcpSocket>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <QDir>
#include <QMessageBox>
#include <QCommonStyle>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    srand(time(0));
    ui->setupUi(this);
    userModel = new FilezillaUserModel(&users);
    ui->tvUsers->setModel(userModel);
    connect(&conn, SIGNAL(authSuccess()), this, SLOT(connectionAuthenticated()));
    connect(&conn, SIGNAL(connFail(QString)), this, SLOT(connectionFailed(QString)));
    connect(&conn, SIGNAL(connMessage(QString)), this, SLOT(connectionMessage(QString)));
    connect(&conn, SIGNAL(replyReceived(FilezillaReply)), this, SLOT(serverReply(FilezillaReply)));
    usersContextMenu = new QMenu();
    usersContextMenu->addAction(ui->actionEdit_user_directories);
    usersContextMenu->addAction(ui->actionDelete_user);
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
    updateAccountSettings();
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
    if(reply.id == 6)
    {
        handleAccountReply(reply);
    }
    else if(reply.id == 0)
    {
        updateAccountSettings();
    }
}

FilezillaUser MainWindow::getUser(const QString company, const QString password)
{
    QString lower = company.toLower();
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

    QDir realDir("M:/");
    realDir.mkpath(startUpper + "/" + "# Customer uploads");

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

    QByteArray utf8 = password.toUtf8();
    MD5 md5;
    md5.update(reinterpret_cast<const unsigned char*>(utf8.data()), password.length());
    md5.finalize();
    QString pass = QString::fromAscii(md5.hex_digest());
    ret.password = pass;

    return ret;
}

FilezillaUser *MainWindow::getSelectedUser()
{
    QModelIndexList selected = ui->tvUsers->selectionModel()->selectedIndexes();
    if(selected.size() == 0)
    {
        return NULL;
    }

    return &users[selected[0].row()];
}

void MainWindow::handleAccountReply(FilezillaReply &reply)
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
    if(reply.length < 2)
    {
        ui->textArea->appendPlainText("Unexpected data length");
    }
    else
    {
        FilezillaAccountPacket packet(reply.data);
        unsigned int num = packet.getNextInt16();
        for(unsigned int i=0; i<num; i++)
        {
            FilezillaGroup grp;
            grp.Parse(packet);
        }

        num = packet.getNextInt16();

        ui->btnDelete->setEnabled(false);
        userModel->clear();
        while(num--)
        {
            FilezillaUser user;
            user.Parse(packet);
            userModel->pushBack(user);
        }
    }
}

void MainWindow::updateAccountSettings()
{
    conn.SendCommand(6, 0, 0);
}

void MainWindow::sendAccountSettings()
{
    FilezillaAccountPacket packet;
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

    conn.SendCommand(6, packet.data.data(), packet.data.length());
}

void MainWindow::on_btnAddUser_clicked()
{
    FilezillaUser user = getUser(ui->leUsername->text(), ui->lePassword->text());
    bool found = false;
    unsigned int i;
    for(i=0; i<users.size(); i++)
    {
        // If user exists update with new info rather than
        // creating a new user completely.
        if(users[i].username == user.username)
        {
            found = true;
            users[i] = user;
            break;
        }
    }

    if(!found)
    {
        users.push_back(user);
    }
    sendAccountSettings();
    updateAccountSettings();
}

void MainWindow::on_btnGeneratePass_clicked()
{
    // Valid ranges: 48 - 57, 65-90, 97-122

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
    if(ui->tvUsers->model()->flags(index) & Qt::ItemIsSelectable)
    {
        QVariant username = userModel->data(index, Qt::DisplayRole);
        if(username.isValid())
        {
            ui->leUsername->setText(username.toString());
        }
        ui->btnDelete->setEnabled(true);
    }
    else
    {
        ui->btnDelete->setEnabled(false);
    }
}

void MainWindow::on_tvUsers_activated(const QModelIndex &index)
{
    if(ui->tvUsers->model()->flags(index) & Qt::ItemIsSelectable)
    {
        QVariant username = userModel->data(index, Qt::DisplayRole);
        if(username.isValid())
        {
            ui->leUsername->setText(username.toString());
        }
        ui->btnDelete->setEnabled(true);
    }
    else
    {
        ui->btnDelete->setEnabled(false);
    }
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
        }
    }
}

void MainWindow::on_actionDelete_user_triggered()
{
    deleteSelectedUser();
}

void MainWindow::deleteSelectedUser()
{
    QModelIndexList selected = ui->tvUsers->selectionModel()->selectedIndexes();
    if(selected.size() > 0)
    {
        QMessageBox mb;
        mb.setText("Are you sure you want to delete user " + users[selected[0].row()].username + "? This can not be undone.");
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        mb.setIcon(QMessageBox::Warning);
        if(mb.exec() == QMessageBox::Yes)
        {
            userModel->removeAt(selected[0].row());

            sendAccountSettings();

            updateAccountSettings();
        }
    }
}

void MainWindow::on_actionEdit_user_directories_triggered()
{
    FilezillaUser *selected = getSelectedUser();
    if(selected == NULL)
    {
        return;
    }
    EditDirectoriesDialog diag(selected->directories);
    int res = diag.exec();
    if(res == QDialog::Accepted)
    {
        selected->directories = diag.getDirectories();
        sendAccountSettings();
        updateAccountSettings();
    }
}
