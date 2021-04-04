/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：面板窗口
 * ***************************************/
#include "framewidget.h"
#include "ui_framewidget.h"



// 构造函数
FrameWidget::FrameWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrameWidget)
{
    ui->setupUi(this);
    this->setGeometry(500, 300, 800, 600);

    label = new QLabel(this);
    lcdNumber = new QLCDNumber(this);
    textBrowser = new QTextBrowser(this);

    m_pH = new CHistogram2(this);
    m_pH->setGeometry(250, 50, 500, 250);

    init();
    this->startTimer(1000);
}

QString get_Time3()
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    return current_date_time.toString("yyyy.MM.dd hh:mm:ss\t");
}

void FrameWidget::init(){
    label->setText("亮度:");
    label->setGeometry(50, 50, 100, 70);
    label->setStyleSheet("font-family: Microsoft YaHei;font-size: 30px;");

    lcdNumber->setGeometry(50, 150, 180, 120);

    textBrowser->setGeometry(50, 300, 700, 250);
    textBrowser->setStyleSheet("color:#00FF00;background-color:black;font-size:20px");

    pushButton = new QPushButton(this);
    pushButton->setGeometry(50, 575, 75, 30);
    pushButton->setText("打开日志");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(on_pushButton_Clicked()));
}

void FrameWidget::on_pushButton_Clicked()
{
    QFile *f = new QFile("./lightLog.txt");
    if(!f->open(QFile::ReadOnly|QFile::Text)) return;

    QTextStream *in = new QTextStream(f);

    QDialog *dialog = new QDialog();
    dialog->setModal(false);
    dialog->resize(600, 500);
    dialog->setWindowTitle("亮度日志");

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setPlainText(in->readAll());
    textEdit->resize(600, 500);

    dialog->show();
}
void FrameWidget::timerEvent(QTimerEvent *event)
{
    lightList << light;
    if(lightList.length() > 7) lightList.pop_front();
    m_pH->refreshUI(lightList);

    lcdNumber->display(light);
    QString current_date = get_Time3();

    if(isLightWarning)
    {
        textBrowser->append("<font color=\"#FF0000\">" + current_date + "Light:" + QString::number(light) + "</font>");
        lcdNumber->setStyleSheet("color:red;");
    }
    else
    {
        textBrowser->append( current_date + "Light:" + QString::number(light));
        lcdNumber->setStyleSheet("color:black;");
    }
    QFile* f = new QFile("./lightLog.txt");
    if(!f->open(QIODevice::Append|QIODevice::Text)) return ;

    QTextStream txtOutPut(f);
    txtOutPut << current_date + "Light:" + QString::number(light) << endl;

    f->close();
}




// 析构函数
FrameWidget::~FrameWidget()
{
    delete ui;
}




CHistogram2::CHistogram2(QWidget *parent)
    :QFrame(parent)
{
    initUI();
}

void CHistogram2::refreshUI(QVector<int> vecData)
{
    if(vecData.isEmpty())
        return ;
    vecData.resize(7);
    for(int i = 0; i < vecData.size(); i++){
        m_pBarSet->replace(i,vecData.at(i));
    }
}

void CHistogram2::initUI()
{
    this->resize(500, 250);
    QFont fontText("SimHei");
    fontText.setPixelSize(12);

    QBarSeries *pBarSeries = new QBarSeries();
    pBarSeries->setLabelsVisible(true);
    pBarSeries->setLabelsFormat("@valuelx");
    pBarSeries->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
    connect(pBarSeries,&QBarSeries::hovered,[=](bool bStatus,int index,QBarSet *pBarSet){ //鼠标悬浮柱状条
        Q_UNUSED(bStatus)
        Q_UNUSED(index)
        Q_UNUSED(pBarSet)
    });

    m_pBarSet = new QBarSet("亮度");
    m_pBarSet->setBrush(QBrush("#dd5da4f2"));
    m_pBarSet->setBorderColor("#dd5da4f2");
    m_pBarSet->setLabelFont(fontText);
    *m_pBarSet << 0 << 500 << 800 << 1200 << 2000 << 3000 << 200;
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
    pAxisY->setRange(0, 3000);
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
