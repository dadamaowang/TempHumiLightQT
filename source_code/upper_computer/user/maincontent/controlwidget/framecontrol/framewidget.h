/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：面板窗口
 * ***************************************/
#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include <maincontent/controlwidget/bannacontrol/bannawidget.h>
#include <QWidget>
#include <QTime>
#include <QLabel>
#include <QLCDNumber>
#include <QTextBrowser>
#include <QtCharts>

#include <QFrame>
#include <QChart>
#include <QBarSet>
#include <QChart>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QChartView>
#include <QTime>
#include <QDebug>
#include <QStackedBarSeries>
#include <QValueAxis>
#include <QLabel>
#include <QCursor>

class FlowLayout;

namespace Ui {
class FrameWidget;
}

class CHistogram2;
class FrameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FrameWidget(QWidget *parent = 0);
    ~FrameWidget();

private:
    Ui::FrameWidget *ui;
    QLabel* label;
    QLCDNumber* lcdNumber;
    QTextBrowser* textBrowser;

    QChartView* view;
    QLineSeries* series;
    CHistogram2* m_pH;
    QVector<int> lightList;
    QPushButton* pushButton;

private:
    void init();
    void timerEvent(QTimerEvent *event);
private slots:
    void on_pushButton_Clicked();
};


using namespace QtCharts;
class CHistogram2 : public QFrame
{
    Q_OBJECT
public:
    explicit CHistogram2(QWidget *parent = 0);

    void refreshUI(QVector<int> vecData);
private:
    void initUI();

private:
    QBarSet *m_pBarSet = nullptr;
};


#endif // FRAMEWIDGET_H
