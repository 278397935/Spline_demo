#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxworksheet.h"

#include "spline.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void InsertValues(const double *adX0, const double *adY0, const int& iNum0,
                      const double *adX , double *adY, const int& iNum);

    bool IsNumber(QString qstrSrc);

    /* Read last Dir log file, get last Dir(Previous directory) */
    QString LastDirRead();

    /* Save the current project Dir as the most recently opened directory */
    void LastDirWrite(QString oStrFileName);

private slots:
    void on_pushButtonOpenExist_clicked();

    void on_pushButtonOpenTarget_clicked();

    void on_pushButtonSpline_clicked();

private:
    Ui::MainWindow *ui;

    QString oStrTargetFileName;

    double *adExitsX,*adExitsY;
    int uiExitsCnt;

    double *adTargetX,*adTargetY;
    int uiTargetCnt;
};

#endif // MAINWINDOW_H
