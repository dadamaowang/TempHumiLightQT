/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：进度条窗口
 * ***************************************/
#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include "maincontent/controlwidget/bannacontrol/bannawidget.h"
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

namespace Ui {
class ProgressWidget;
}

class CHistogram3;
class ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget *parent = 0);
    ~ProgressWidget();

private:
    QLabel* label;
    QLCDNumber* lcdNumber;
    QTextBrowser* textBrowser;

    CHistogram3* m_pH;
    QVector<int> humiList;
    QPushButton* pushButton;

private:
    Ui::ProgressWidget *ui;
    void init();
    void timerEvent(QTimerEvent *event);

private slots:
    void on_pushButton_Clicked();
};


using namespace QtCharts;
class CHistogram3 : public QFrame
{
    Q_OBJECT
public:
    explicit CHistogram3(QWidget *parent = 0);

    void refreshUI(QVector<int> vecData);
private:
    void initUI();

private:
    QBarSet *m_pBarSet = nullptr;
};


#endif // PROGRESSWIDGET_H
