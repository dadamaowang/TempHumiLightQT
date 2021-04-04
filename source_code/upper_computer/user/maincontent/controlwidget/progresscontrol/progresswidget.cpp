/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：进度条窗口
 * ***************************************/
#include "progresswidget.h"
#include "ui_progresswidget.h"


// 构造函数
ProgressWidget::ProgressWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProgressWidget)
{
    ui->setupUi(this);
    this->setGeometry(500, 300, 800, 600);

    label = new QLabel(this);
    lcdNumber = new QLCDNumber(this);
    textBrowser = new QTextBrowser(this);

    m_pH = new CHistogram3(this);
    m_pH->setGeometry(250, 50, 500, 250);
    init();

    this->startTimer(1000);
}

QString get_Time2()
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    return current_date_time.toString("yyyy.MM.dd hh:mm:ss\t");
}

void ProgressWidget::init(){
    label->setText("湿度:");
    label->setGeometry(50, 50, 100, 70);
    label->setStyleSheet("font-family: Microsoft YaHei;font-size: 30px;");

    lcdNumber->setGeometry(50, 150, 180, 120);

    textBrowser->setGeometry(50, 300, 700, 250);
    textBrowser->setStyleSheet("color:red;background-color:black;font-size:20px");

    pushButton = new QPushButton(this);
    pushButton->setGeometry(50, 575, 75, 30);
    pushButton->setText("打开日志");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(on_pushButton_Clicked()));
}

void ProgressWidget::on_pushButton_Clicked()
{
    QFile *f = new QFile("./humiLog.txt");
    if(!f->open(QFile::ReadOnly|QFile::Text)) return;

    QTextStream *in = new QTextStream(f);

    QDialog *dialog = new QDialog();
    dialog->setModal(false);
    dialog->resize(600, 500);
    dialog->setWindowTitle("湿度日志");

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setPlainText(in->readAll());
    textEdit->resize(600, 500);

    dialog->show();
}

void ProgressWidget::timerEvent(QTimerEvent *event)
{
    humiList << humi;
    if(humiList.length() > 7) humiList.pop_front();
    m_pH->refreshUI(humiList);

    lcdNumber->display(humi);
    QString current_date = get_Time2();

    if(isHumiWarning)
        textBrowser->append( current_date + "Humidity:" + QString::number(humi)),
                lcdNumber->setStyleSheet("color:red;");
    else
        textBrowser->append("<font color=\"#00FF00\">" + current_date + "Humidity:" + QString::number(humi) + "</font>"),
                lcdNumber->setStyleSheet("color:black;");
    QFile* f = new QFile("./humiLog.txt");
    if(!f->open(QIODevice::Append|QIODevice::Text)) return ;

    QTextStream txtOutPut(f);
    txtOutPut << current_date + "Humidity:" + QString::number(humi) << endl;

    f->close();
}




CHistogram3::CHistogram3(QWidget *parent)
    :QFrame(parent)
{
    initUI();
}

void CHistogram3::refreshUI(QVector<int> vecData)
{
    if(vecData.isEmpty())
        return ;
    vecData.resize(7);
    for(int i = 0; i < vecData.size(); i++){
        m_pBarSet->replace(i,vecData.at(i));
    }
}

void CHistogram3::initUI()
{
    this->resize(500, 250);
    QFont fontText("SimHei");
    fontText.setPixelSize(12);

    QBarSeries *pBarSeries = new QBarSeries();
    pBarSeries->setLabelsVisible(true);
    pBarSeries->setLabelsFormat("@value%");
    pBarSeries->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
    connect(pBarSeries,&QBarSeries::hovered,[=](bool bStatus,int index,QBarSet *pBarSet){ //鼠标悬浮柱状条
        Q_UNUSED(bStatus)
        Q_UNUSED(index)
        Q_UNUSED(pBarSet)
    });

    m_pBarSet = new QBarSet("湿度");
    m_pBarSet->setBrush(QBrush("#dd5da4f2"));
    m_pBarSet->setBorderColor("#dd5da4f2");
    m_pBarSet->setLabelFont(fontText);
    *m_pBarSet << 0 << 7 << 8 << 12 << 20 << 100 << 16;
    pBarSeries->append(m_pBarSet);


    /*X轴设置*/
    QBarCategoryAxis *pAxisX = new QBarCategoryAxis();
    pAxisX->setGridLineColor("transparent");    //参考线颜色透明
    pAxisX->setLinePenColor("#5da4f2");         //标尺 线颜色
    pAxisX->setLabelsColor("#5da4f2");          //文本颜色
    pAxisX->setLabelsFont(fontText);

    /*Y轴设置*/
    QValueAxis *pAxisY = new QValueAxis();
    pAxisY->setTickCount(11); //包含0那一条横线
    pAxisY->setRange(0,100);
    pAxisY->setGridLineColor("#5da4f2");
    pAxisY->setLinePenColor("#5da4f2"); //标尺 线颜色
    pAxisY->setLabelsColor("#5da4f2");  //文本颜色
    pAxisY->setLabelsFont(fontText);

    QChart *pChart = new QChart();
    pChart->addSeries(pBarSeries);
    pChart->setAnimationOptions(QChart::SeriesAnimations);
    pChart->createDefaultAxes();
    pChart->setAxisX(pAxisX);
    pChart->setAxisY(pAxisY);
    pChart->legend()->setVisible(true); //设置图例为显示状态
    pChart->legend()->setAlignment(Qt::AlignBottom);//设置图例的显示位置在底部

    QChartView *pChartView = new QChartView(this);
    pChartView->setChart(pChart);
    pChartView->setRenderHint(QPainter::Antialiasing);
    pChartView->resize(this->size());

}

// 析构函数
ProgressWidget::~ProgressWidget()
{
    delete ui;
}


