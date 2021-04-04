/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：柱状图窗口
 * ***************************************/
#ifndef CYLINDERWIDGET_H
#define CYLINDERWIDGET_H

#include <QWidget>
#include <QTime>
#include <QLabel>
#include <QLCDNumber>
#include <QTextBrowser>
#include <QtCharts>
#include <QTimerEvent>
#include <QGridLayout>
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

#include <QPushButton>
namespace Ui {
class CylinderWidget;
}

class CHistogram;

class CylinderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CylinderWidget(QWidget *parent = 0);
    ~CylinderWidget();

private:
    QLabel* label;
    QLCDNumber* lcdNumber;
    QTextBrowser* textBrowser;

    Ui::CylinderWidget *ui;
    CHistogram* m_pH;
    QVector<int> tempList;
    QPushButton* pushButton;
    //QGridLayout* gridLayout;

private:
    void init();
    void timerEvent(QTimerEvent *event);

private slots:
    void on_pushButton_Clicked();
};


using namespace QtCharts;
class CHistogram : public QFrame
{
    Q_OBJECT
public:
    explicit CHistogram(QWidget *parent = 0);

    void refreshUI(QVector<int> vecData);
private:
    void initUI();

private:
    QBarSet *m_pBarSet = nullptr;
};

#endif // CYLINDERWIDGET_H
