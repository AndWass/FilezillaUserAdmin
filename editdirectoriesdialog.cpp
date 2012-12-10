#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include "editdirectoriesdialog.h"
#include "ui_editdirectoriesdialog.h"

EditDirectoriesDialog::EditDirectoriesDialog(const std::vector<FilezillaDirectory> &dirs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDirectoriesDialog),
    directories(dirs)
{
    ui->setupUi(this);
    dirModel = new QStandardItemModel();
    for(unsigned int i=0; i<directories.size(); i++)
    {
        dirModel->appendRow(new QStandardItem(directories[i].dir));
    }

    ui->lvDirectories->setModel(dirModel);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focus_changed(QWidget*,QWidget*)));
}

EditDirectoriesDialog::~EditDirectoriesDialog()
{
    delete ui;
    delete dirModel;
}

std::vector<FilezillaDirectory> EditDirectoriesDialog::getDirectories()
{
    return directories;
}

void EditDirectoriesDialog::savePrevIndex()
{
    if(prevIndex.isValid())
    {
        unsigned int i = prevIndex.row();
        if(i < directories.size())
        {
            FilezillaDirectory *d = &directories[i];
            d->fileRead = ui->cbFileRead->isChecked();
            d->fileWrite = ui->cbFileWrite->isChecked();
            d->fileDelete = ui->cbFileDelete->isChecked();
            d->fileAppend = ui->cbFileAppend->isChecked();

            d->dirCreate = ui->cbDirCreate->isChecked();
            d->dirDelete = ui->cbDirDelete->isChecked();
            d->dirList = ui->cbDirList->isChecked();
            d->dirSubdirs = ui->cbDirSubDirs->isChecked();
            d->isHome = ui->cbIsHome->isChecked();
        }
    }
}

void EditDirectoriesDialog::on_lvDirectories_clicked(const QModelIndex &index)
{
    savePrevIndex();
    prevIndex = index;
    if(index.isValid())
    {
        unsigned int i = index.row();
        if(i < directories.size())
        {
            FilezillaDirectory *d = &directories[i];
            ui->cbFileRead->setChecked(d->fileRead);
            ui->cbFileWrite->setChecked(d->fileWrite);
            ui->cbFileAppend->setChecked(d->fileAppend);
            ui->cbFileDelete->setChecked(d->fileDelete);

            ui->cbDirCreate->setChecked(d->dirCreate);
            ui->cbDirDelete->setChecked(d->dirDelete);
            ui->cbDirList->setChecked(d->dirList);
            ui->cbDirSubDirs->setChecked(d->dirSubdirs);

            ui->cbIsHome->setChecked(d->isHome);
            ui->cbIsHome->setEnabled(!d->isHome);
        }
    }
}

void EditDirectoriesDialog::on_btnDelete_clicked()
{
    QModelIndexList indices = ui->lvDirectories->selectionModel()->selectedIndexes();
    if(indices.size() > 0)
    {
        if(directories[indices[0].row()].isHome)
        {
            QMessageBox mb;
            mb.setText("Can not delete a home directory");
            mb.setStandardButtons(QMessageBox::Ok);
            mb.exec();
        }
        else
        {
            dirModel->removeRow(indices[0].row());
            directories.erase(directories.begin() + indices[0].row());
        }
    }
}

void EditDirectoriesDialog::on_btnBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), "C:\\",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dir.length() > 0)
    {
        while(dir[dir.length()-1] == '\\')
        {
            dir.remove(dir.length()-1, 1);
        }

        ui->leDirectory->setText(dir);
    }
}

void EditDirectoriesDialog::on_btnAdd_clicked()
{
    QString dir = ui->leDirectory->text();
    if(dir.length() > 0)
    {
        FilezillaDirectory d;
        d.dir = dir;
        d.fileWrite = ui->cbFileWrite->isChecked();
        d.fileDelete = ui->cbFileDelete->isChecked();
        d.fileAppend = ui->cbFileAppend->isChecked();
        d.fileRead = ui->cbFileRead->isChecked();

        d.dirCreate = ui->cbDirCreate->isChecked();
        d.dirDelete = ui->cbDirDelete->isChecked();
        d.dirList = ui->cbDirList->isChecked();
        d.dirSubdirs = ui->cbDirSubDirs->isChecked();
        d.isHome = ui->cbIsHome->isChecked();

        dirModel->appendRow(new QStandardItem(d.dir));
        directories.push_back(d);
    }
}

void EditDirectoriesDialog::on_cbIsHome_clicked()
{
    if(ui->cbIsHome->isChecked())
    {
        QModelIndexList indices = ui->lvDirectories->selectionModel()->selectedIndexes();
        if(indices.size() > 0)
        {
            unsigned int selectedRow = indices[0].row();
            for(unsigned int i=0; i<directories.size(); i++)
            {
                if(i == selectedRow)
                {
                    continue;
                }

                directories[i].isHome = false;
            }

            ui->cbIsHome->setEnabled(false);
        }
    }
}

void EditDirectoriesDialog::focus_changed(QWidget *from, QWidget *to)
{
    from; // Get rid of warning.
    if(to == ui->leDirectory)
    {
        ui->lvDirectories->clearSelection();
        ui->cbFileRead->setChecked(true);
        ui->cbFileWrite->setChecked(false);
        ui->cbFileAppend->setChecked(false);
        ui->cbFileDelete->setChecked(false);

        ui->cbDirCreate->setChecked(false);
        ui->cbDirDelete->setChecked(false);
        ui->cbDirList->setChecked(true);
        ui->cbDirSubDirs->setChecked(true);

        ui->cbIsHome->setEnabled(false);

        if(directories.size() == 0)
        {
            ui->cbIsHome->setChecked(true);
        }
        else
        {
            ui->cbIsHome->setChecked(false);
        }

    }
}
