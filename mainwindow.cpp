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

    QToolBar *naviBar = new QToolBar("导航",this);
    this->addToolBar(Qt::LeftToolBarArea,naviBar);

    poActionFiles = new QAction("导入校正文件",this);
    naviBar->addAction(poActionFiles);

    connect(poActionFiles,&QAction::triggered,this,&MainWindow::CAS90B);

    naviBar->setMovable(false);

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

    //    oXlsx.selectSheet("Sheet2");
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

            qDebugV0()<<dX<<dY;
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

/* 读取中科院 电子所 CAS-90B标定文件 */
void MainWindow::CAS90B()
{
    /* 目的频率表 */
    QList<double> adListF;

    adListF = {8192,
               6144,
               4096,
               3072,
               2560,
               2048,
               1536,
               1280,
               1024,
               768,
               640,
               512,
               384,
               320,
               256,
               192,
               160,
               128,
               96,
               80,
               64,
               48,
               40,
               32,
               24,
               20,
               16,
               12,
               10,
               8,
               6,
               5,
               4,
               3,
               2.5,
               2,
               1.5,
               1.25,
               1,
               0.75,
               0.5,
               0.375,
               0.25,
               0.1875,
               0.125,
               0.09375,
               0.0625,
               0.046875,
               0.03125,
               0.0234375,
               0.015625,
               0.01171875};

    QStringList aoStrFileName = QFileDialog::getOpenFileNames(this,
                                                              "选CAS-90B磁棒标定文件",
                                                              "D://",
                                                              "Excel (*.xlsx)");

    foreach(QString oStrFileName, aoStrFileName)
    {
        if(oStrFileName.isEmpty())
        {
            QMessageBox::warning(0,"错误",QString("%1有误！").arg(oStrFileName));
            return;
        }

        QXlsx::Document oXlsx(oStrFileName);

        /* 读 斩波开 sheet 的数据 */
        oXlsx.selectSheet("斩波开");
        QVector<double> adF,adP,adA;
        adF.clear();
        adP.clear();
        adA.clear();

        int iRow = 1;
        while(!(oXlsx.read(iRow, 1).isNull()))
        {
            bool bOkF = false, bOkP = false, bOkA = false;

            double dF = oXlsx.read(iRow, 1).toDouble(&bOkF); //转换是被时返回0.0,ok=false;
            double dP = oXlsx.read(iRow, 2).toDouble(&bOkP); //转换是被时返回0.0,ok=false;
            double dA = oXlsx.read(iRow, 3).toDouble(&bOkA); //转换是被时返回0.0,ok=false;

            if(bOkF&&bOkP&&bOkA)
            {
                adF.append(dF);
                adP.append(dP);
                adA.append(dA);
            }
            iRow++;
        }

        int iCnt = adF.length();
        double *arrdOnF = new double[iCnt];
        double *arrdOnP = new double[iCnt];
        double *arrdOnA = new double[iCnt];

        for(int i = 0; i < iCnt; i++)
        {
            arrdOnF[i] = adF[i];
            arrdOnP[i] = adP[i];
            arrdOnA[i] = adA[i];
        }

        int iCustomCnt = adListF.count();

        /* 斩波开 目的文件数组 定义  初始化 */
        double *arrdCustomOnF = new double[iCustomCnt];
        double *arrdCustomOnP = new double[iCustomCnt];
        double *arrdCustomOnA = new double[iCustomCnt];

        for(int i = 0; i < iCustomCnt; i++)
        {
            arrdCustomOnF[i] = adListF.at(i);
            arrdCustomOnP[i] = 0;//相位默认校正系数 0
            arrdCustomOnA[i] = 1;//振幅默认校正系数 1
        }

        /* 斩波开 抽样插值 */
        this->InsertValues(arrdOnF,  arrdOnP,  iCnt, arrdCustomOnF, arrdCustomOnP, iCustomCnt);
        this->InsertValues(arrdOnF,  arrdOnA,  iCnt, arrdCustomOnF, arrdCustomOnA, iCustomCnt);


        /* 读 斩波关 sheet 的数据 */
        oXlsx.selectSheet("斩波关");
        adF.clear();
        adP.clear();
        adA.clear();

        iRow = 1;
        while(!(oXlsx.read(iRow, 1).isNull()))
        {
            bool bOkF = false, bOkP = false, bOkA = false;

            double dF = oXlsx.read(iRow, 1).toDouble(&bOkF); //转换是被时返回0.0,ok=false;
            double dP = oXlsx.read(iRow, 2).toDouble(&bOkP); //转换是被时返回0.0,ok=false;
            double dA = oXlsx.read(iRow, 3).toDouble(&bOkA); //转换是被时返回0.0,ok=false;

            if(bOkF&&bOkP&&bOkA)
            {
                adF.append(dF);
                adP.append(dP);
                adA.append(dA);
            }
            iRow++;
        }

        iCnt = adF.length();
        double *arrdOffF = new double[iCnt];
        double *arrdOffP = new double[iCnt];
        double *arrdOffA = new double[iCnt];

        for(int i = 0; i < iCnt; i++)
        {
            arrdOffF[i] = adF[i];
            arrdOffP[i] = adP[i];
            arrdOffA[i] = adA[i];
        }

        oXlsx.save();


        /* 斩波关 目的文件数组 定义 初始化 */
        double *arrdCustomOffF = new double[iCustomCnt];
        double *arrdCustomOffP = new double[iCustomCnt];
        double *arrdCustomOffA = new double[iCustomCnt];

        for(int i = 0; i < iCustomCnt; i++)
        {
            arrdCustomOffF[i] = adListF.at(i);
            arrdCustomOffP[i] = 0;
            arrdCustomOffA[i] = 1;
        }

        this->InsertValues(arrdOffF, arrdOffP, iCnt, arrdCustomOffF, arrdCustomOffP, iCustomCnt);
        this->InsertValues(arrdOffF, arrdOffA, iCnt, arrdCustomOffF, arrdCustomOffA, iCustomCnt);

        /* 写结果文件 */
        QFileInfo oFileInfo(oStrFileName);

        QFile oCustom(oFileInfo.path() + "/H_" + oFileInfo.baseName()+".csv");

        qDebugV0()<<"结果文件："<<oFileInfo.baseName()<< oCustom.fileName();

        if (!oCustom.open(QIODevice::WriteOnly))
        {
            return;
        }

        QTextStream oStream(&oCustom);

        /* 写首行信息 */
        oStream<<"CALI_COEF,"<<oFileInfo.baseName()<<",1,"<<"0,"<<"2"<<endl;

        /* 目的文件列定义：
         * 1，频率
         * 2，斩波开AMP
         * 3，斩波开Phase
         * 4，斩波关AMP
         * 5，斩波关Phase
         */
        for(int i = 0; i < iCustomCnt; i++)
        {
            oStream<<QString::number(adListF[i], 'f', 6)<<","
                  <<QString::number(arrdCustomOnA[i],  'f', 6)<<","
                 <<QString::number(arrdCustomOnP[i], 'f', 6)<<","
                <<QString::number(arrdCustomOffA[i],  'f', 6)<<","
               <<QString::number(arrdCustomOffP[i], 'f', 6)<<endl;
        }
        oStream.flush();
        oCustom.close();

        delete []arrdOnF;
        delete []arrdOnP;
        delete []arrdOnA;

        delete []arrdOffF;
        delete []arrdOffP;
        delete []arrdOffA;

        delete []arrdCustomOnF;
        delete []arrdCustomOnP;
        delete []arrdCustomOnA;

        delete []arrdCustomOffF;
        delete []arrdCustomOffP;
        delete []arrdCustomOffA;
    }
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
