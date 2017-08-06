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
    QString lastDir = "";

    lastDir = GetLastPath();

    QString diskFile = QFileDialog::getOpenFileName(this, "Select an ezfs disk file.", lastDir);

    if(diskFile != "")
    {
        SavePath(diskFile);

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
