#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "ezfs_reader.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnLoadDisk_clicked()
{
    QString diskFile = QFileDialog::getOpenFileName(this, "Select an ezfs disk file.");

    if(diskFile != "")
    {
        ui->txtDiskFilePath->setText(diskFile);

        EZFS_Reader x;
        x.Open(diskFile.toLocal8Bit().data());

        if(x.IsValidFilesystem())
        {
            x.ExtractFilesystemData();

            ui->tabWidget->setEnabled(true);
        }
    }
}
