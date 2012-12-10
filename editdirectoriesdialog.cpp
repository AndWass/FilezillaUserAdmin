#include <QFileDialog>
#include "editdirectoriesdialog.h"
#include "ui_editdirectoriesdialog.h"

EditDirectoriesDialog::EditDirectoriesDialog(const std::vector<FilezillaDirectory> &dirs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDirectoriesDialog),
    directories(dirs)
{
    ui->setupUi(this);
    dirModel = new QStandardItemModel();
    for(int i=0; i<directories.size(); i++)
    {
        dirModel->appendRow(new QStandardItem(directories[i].dir));
    }

    ui->lvDirectories->setModel(dirModel);

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
        int i = prevIndex.row();
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
        int i = index.row();
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
        }
    }
}

void EditDirectoriesDialog::on_btnDelete_clicked()
{
    QModelIndexList indices = ui->lvDirectories->selectionModel()->selectedIndexes();
    if(indices.size() > 0)
    {
        dirModel->removeRow(indices[0].row());
        directories.erase(directories.begin() + indices[0].row());
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
