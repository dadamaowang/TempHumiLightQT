#include "CHistogram.h"

CHistogram::CHistogram(QWidget *parent)
    :QFrame(parent)
{
    initUI();
}

void CHistogram::refreshUI(QVector<int> vecData)
{
    if(vecData.isEmpty())
        return ;
    vecData.resize(7);
    for(int i = 0; i < vecData.size(); i++){
        m_pBarSet->replace(i,vecData.at(i));
    }
}

void CHistogram::initUI()
{
    this->resize(600,400);
    QFont fontText("SimHei");
    fontText.setPixelSize(12);

    QBarSeries *pBarSeries = new QBarSeries();
    pBarSeries->setLabelsVisible(true);
    pBarSeries->setLabelsFormat("@valueh");
    pBarSeries->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
    connect(pBarSeries,&QBarSeries::hovered,[=](bool bStatus,int index,QBarSet *pBarSet){ //鼠标悬浮柱状条
        Q_UNUSED(bStatus)
        Q_UNUSED(index)
        Q_UNUSED(pBarSet)
    });

    m_pBarSet = new QBarSet("运行时间");
    m_pBarSet->setBrush(QBrush("#dd5da4f2"));
    m_pBarSet->setBorderColor("#dd5da4f2");
    m_pBarSet->setLabelFont(fontText);
    *m_pBarSet<< 5 << 7 << 8 << 12 << 20 << 24 << 16;
    pBarSeries->append(m_pBarSet);


    /*X轴设置*/
    QBarCategoryAxis *pAxisX = new QBarCategoryAxis();
    QStringList strListDate;
    for(int i = 6;i >= 0; i--) {
        strListDate.append(QDate::currentDate().addDays(-i).toString("M.dd"));
    }
    pAxisX->append(strListDate);
    pAxisX->setGridLineColor("transparent");    //参考线颜色透明
    pAxisX->setLinePenColor("#5da4f2");         //标尺 线颜色
    pAxisX->setLabelsColor("#5da4f2");          //文本颜色
    pAxisX->setLabelsFont(fontText);
    /*Y轴设置*/
    QValueAxis *pAxisY = new QValueAxis();
    pAxisY->setTickCount(13); //包含0那一条横线
    pAxisY->setLabelFormat("%dh");
    pAxisY->setRange(0,24);
    pAxisY->setGridLineColor("#5da4f2");
    pAxisY->setLinePenColor("#5da4f2"); //标尺 线颜色
    pAxisY->setLabelsColor("#5da4f2");  //文本颜色
    pAxisY->setLabelsFont(fontText);

    QChart *pChart = new QChart();
    pChart->addSeries(pBarSeries);
    pChart->setTitle("一周运行时间");
    pChart->setAnimationOptions(QChart::SeriesAnimations);
    pChart->createDefaultAxes();
    pChart->setAxisX(pAxisX);
    pChart->setAxisY(pAxisY);
    pChart->legend()->setVisible(true); //设置图例为显示状态
    pChart->legend()->setAlignment(Qt::AlignBottom);//设置图例的显示位置在底部

    QChartView *pChartView = new QChartView(pChart,this);
    pChartView->setRenderHint(QPainter::Antialiasing);
    pChartView->resize(this->size());

}

