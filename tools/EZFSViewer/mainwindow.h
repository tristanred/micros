#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

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

private:
    Ui::MainWindow *ui;

    QString GetLastPath();
    void SavePath(QString path);

};

#endif // MAINWINDOW_H
