#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

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

        ui->tabWidget->setEnabled(true);
    }
}
