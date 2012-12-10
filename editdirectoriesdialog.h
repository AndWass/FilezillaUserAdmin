#ifndef EDITDIRECTORIESDIALOG_H
#define EDITDIRECTORIESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "filezillaaccounts.h"

namespace Ui {
class EditDirectoriesDialog;
}

class EditDirectoriesDialog : public QDialog
{
    Q_OBJECT

    std::vector<FilezillaDirectory> directories;

    QStandardItemModel *dirModel;

    QModelIndex prevIndex;

    void savePrevIndex();
    
public:
    explicit EditDirectoriesDialog(const std::vector<FilezillaDirectory> &dirs, QWidget *parent = 0);
    ~EditDirectoriesDialog();

    std::vector<FilezillaDirectory> getDirectories();
    
private slots:

    void on_lvDirectories_clicked(const QModelIndex &index);

    void on_btnDelete_clicked();

    void on_btnBrowse_clicked();

    void on_btnAdd_clicked();

private:
    Ui::EditDirectoriesDialog *ui;
};

#endif // EDITDIRECTORIESDIALOG_H
