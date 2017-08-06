#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "ezfs_types.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setEnabled(false);

    disk = new EZFS_Reader();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete(disk);
}

void MainWindow::on_btnLoadDisk_clicked()
{
    QString lastDir = "";

    lastDir = GetLastPath();

    QString diskFile = QFileDialog::getOpenFileName(this, "Select an ezfs disk file.", lastDir);

    if(diskFile != "")
    {
        SavePath(diskFile);

        ui->txtDiskFilePath->setText(diskFile);

        disk->Open(diskFile.toLocal8Bit().data());

        if(disk->IsValidFilesystem())
        {
            disk->ExtractFilesystemData();

            ui->tabWidget->setEnabled(true);

            ui->txtMagic->setText(disk->metablock->magic);
            ui->txtFileCount->setText(QString::number(disk->metablock->files_amount));
            ui->txtMajorVersion->setText(QString::number(disk->metablock->version_fs_major));
            ui->txtMinorVersion->setText(QString::number(disk->metablock->version_fs_minor));
            ui->txtAllocStart->setText(QString::number(disk->metablock->allocation_area_start));
            ui->txtAllocLength->setText(QString::number(disk->metablock->allocation_area_length));
            ui->txtDataStart->setText(QString::number(disk->metablock->data_area_start));
            ui->txtDataLength->setText(QString::number(disk->metablock->data_area_length));

            ShowFileAllocation(currentAllocId);
        }
    }
}

QString MainWindow::GetLastPath()
{
    QFile f("_savedpath");

    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray arr = f.readAll();

        return QString(arr);
    }

    return "";
}

void MainWindow::SavePath(QString path)
{
    QFile f("_savedpath");

    if(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        f.write(path.toLocal8Bit());
    }
}

void MainWindow::ShowFileAllocation(int index)
{
    auto file = disk->Files->at(index);

    ui->txtAllocIndex->setText(QString::number(index));

    ui->txtFileId->setText(QString::number(file->id));
    ui->txtFileNumber->setText(QString::number(file->fileNumber));
    ui->txtFileName->setText(file->name);
    ui->txtFileType->setText(QString::number(file->type));
    ui->txtDiskAddress->setText(QString::number(file->dataBlockDiskAddress));
    ui->txtDataSize->setText(QString::number(file->dataSize));
    ui->txtDiskSize->setText(QString::number(file->diskSize));

    ui->chkAllocated->setChecked(file->allocated == 1);

    QString dataString = QString(disk->GetFileData(index));

    ui->txtData->setPlainText(dataString);
}

void MainWindow::on_btnPreviousAllocation_clicked()
{
    if(currentAllocId > 0)
    {
        currentAllocId--;
        ShowFileAllocation(currentAllocId);
    }
}

void MainWindow::on_btnNextAllocation_clicked()
{
    if(currentAllocId + 1< MAX_FILES_NUM)
    {
        currentAllocId++;
        ShowFileAllocation(currentAllocId);
    }
}
