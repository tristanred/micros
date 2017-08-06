#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "ezfs_reader.h"

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
    void on_btnLoadDisk_clicked();

    void on_btnPreviousAllocation_clicked();

    void on_btnNextAllocation_clicked();

private:
    int currentAllocId = 0;

    Ui::MainWindow *ui;

    EZFS_Reader* disk;

    QString GetLastPath();
    void SavePath(QString path);

    void ShowFileAllocation(int index);

};

#endif // MAINWINDOW_H
