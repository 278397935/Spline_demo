#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <QLineEdit>

#include <QComboBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InsertValues(const double *adX0, const double *adY0, const int &iNum0,
                              const double *adX,  double *adY,        const int &iNum)
{
    qDebug()<<iNum0<<iNum;

    double leftBound = 0, RightBound = 0;	//边界导数

    try
    {
        Spline sp(adX0, adY0, iNum0, GivenSecondOrder, leftBound, RightBound);

        sp.MultiPointInterp(adX, iNum, adY);			//求x的插值结果y

        //        for(int i = 0;i < iNum;i++)
        //        {
        //            qDebug()<<adX[i]<<","<<adY[i];
        //        }
    }
    catch(SplineFailure sf)
    {
        qDebug()<<sf.GetMessage()<<endl;
    }
}

/* open exist file */
void MainWindow::on_pushButtonOpenExist_clicked()
{
    ui->lineEdit->clear();
    QString oStrFileName = QFileDialog::getOpenFileName(this, tr("Open Exist File"),
                                                        QString("%1").arg(this->LastDirRead()),
                                                        tr("Exist file(*.xlsx)"));

    if(!oStrFileName.isEmpty())
    {
        ui->lineEditExist->setText(oStrFileName);
    }

    QXlsx::Document oXlsx(oStrFileName);

    QVector<double> adX,adY;
    adX.clear();
    adY.clear();
    int iRow = 1;
    while(!(oXlsx.read(iRow, 1).isNull()))
    {
        bool bOkX = false, bOkY = false;
        double dX = oXlsx.read(iRow, 1).toDouble(&bOkX); //转换是被时返回0.0,ok=false;
        double dY = oXlsx.read(iRow, 2).toDouble(&bOkY); //转换是被时返回0.0,ok=false;
        if(bOkX&&bOkY)
        {
            adX.append(dX);
            adY.append(dY);
            iRow++;
        }
    }

    uiExitsCnt = adX.length();
    adExitsX = new double[uiExitsCnt];
    adExitsY = new double[uiExitsCnt];

    for(int i = 0; i < uiExitsCnt; i++)
    {
        adExitsX[i] = adX[i];
        adExitsY[i] = adY[i];
    }

    oXlsx.save();

    ui->lineEdit->setText("Read exits file over!");
}

/* 目标文件 */
void MainWindow::on_pushButtonOpenTarget_clicked()
{
    ui->lineEdit->clear();
    oStrTargetFileName.clear();

    oStrTargetFileName = QFileDialog::getOpenFileName(this, tr("Open Target File"),
                                                      QString("%1").arg(this->LastDirRead()),
                                                      tr("Target file(*.xlsx)"));
    if(!oStrTargetFileName.isEmpty())
    {
        ui->lineEditTarget->setText(oStrTargetFileName);
    }

    QXlsx::Document oXlsx(oStrTargetFileName);

    QVector<double> adX;
    adX.clear();
    int iRow = 1;
    while(!(oXlsx.read(iRow, 1).isNull()))
    {
        bool bOkX = false;
        double dX = oXlsx.read(iRow, 1).toDouble(&bOkX);

        if(bOkX)
        {
            adX.append(dX);
            iRow++;
        }
    }

    uiTargetCnt = adX.length();
    adTargetX = new double[uiTargetCnt];
    adTargetY = new double[uiTargetCnt];

    for(int i = 0; i < uiTargetCnt; i++)
    {
        adTargetX[i] = adX[i];
    }
    oXlsx.save();

    ui->lineEdit->setText("Read target file over!");
}

/* 样条插值转换 */
void MainWindow::on_pushButtonSpline_clicked()
{
    ui->lineEdit->clear();

    this->InsertValues(adExitsX, adExitsY ,uiExitsCnt,
                       adTargetX, adTargetY, uiTargetCnt);

    QXlsx::Document xlsx(oStrTargetFileName);

    for(int i = 0; i < uiTargetCnt; i++)
    {
        xlsx.write(i+1, 2, adTargetY[i]);
    }
    xlsx.save();

    ui->lineEdit->setText("Completed!");
}

bool MainWindow::IsNumber(QString qstrSrc)
{
    QByteArray ba = qstrSrc.toLatin1();
    const char *s = ba.data();
    bool bret = true;
    while(*s)
    {
        if(*s>='0' && *s<='9')
        {

        }
        else
        {
            bret = false;
            break;
        }
        s++;
    }
    return bret;
}



/*******************************************************************
 * Read last Dir log file, get last Dir(Previous directory)
 */
QString MainWindow::LastDirRead()
{
    QString oStrLastDir;
    oStrLastDir.clear();

    //qDebugV0()<<"Read last time Dir.";

    QFile oFileLastDir(LASTDIR);

    if( oFileLastDir.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QTextStream oTextStreamIn(&oFileLastDir);
        oStrLastDir = oTextStreamIn.readLine();

        if(oStrLastDir.isNull())
        {
            /* Default Dir */
            oStrLastDir = "D:/";
        }

        //qDebugV0()<<"Last Dir:"<<oStrLastDir;
    }
    else
    {
        qDebugV0()<<"Can't open the file! Return default Dir.";

        /* Default Dir */
        oStrLastDir = "D:/";
    }

    oFileLastDir.close();

    return oStrLastDir;
}


/***********************************************************************
 * Save the current project Dir as the most recently opened directory
 */
void MainWindow::LastDirWrite(QString oStrFileName)
{
    QFileInfo oFileInfoLastDir(oStrFileName);

    //qDebugV0()<<"Current project Dir:"<<oFileInfoLastDir.absoluteDir().absolutePath();

    QDir oDir = oFileInfoLastDir.absoluteDir();

    //qDebugV0()<<oDir;

    if( !oDir.cdUp() )
    {
        //qDebugV0()<<"After switching to the first level Dir:"<<oDir.absolutePath();
        qDebugV0()<<"The previous directory of the current directory does not exist!";
        return;
    }

    QFile oFileLastDir(LASTDIR);

    if( !oFileLastDir.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) )
    {
        qDebugV0()<<"Open last dir file false!";
        return;
    }

    QTextStream oTextStreamOut(&oFileLastDir);

    QDir oDirRslt = oFileInfoLastDir.absoluteDir();

    //qDebugV0()<<oDirRslt.absolutePath();

    oTextStreamOut<<oDirRslt.absolutePath();

    oFileLastDir.close();
}
