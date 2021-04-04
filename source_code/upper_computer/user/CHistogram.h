#ifndef CHISTOGRAM_H
#define CHISTOGRAM_H

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

#endif // CHISTOGRAM_H

