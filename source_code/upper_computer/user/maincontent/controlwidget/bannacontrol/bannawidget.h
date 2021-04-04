/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：轮播图窗口
 * ***************************************/
#ifndef BANNAWIDGET_H
#define BANNAWIDGET_H

#include "maincontent/controlwidget/listcontrol/listwidget.h"
#include <QWidget>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QSpinBox>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QStringList>
#include <QString>

extern int temp;
extern int humi;
extern int light;
extern bool isTempWarning;
extern bool isHumiWarning;
extern bool isLightWarning;

namespace Ui {
class BannaWidget;
}

class BannaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannaWidget(QWidget *parent = 0);
    ~BannaWidget();

private:
    Ui::BannaWidget *ui;
    QGridLayout* gridLayout;
    QLabel* label, *label_2, *label_3, *label_4, *label_5, *label_6, *label_7, *label_8, *label_9;
    QVector<QLabel> labelList;
    QLCDNumber* lcdNumber, *lcdNumber_2, *lcdNumber_3;
    QSpinBox* spinBox, *spinBox_2, *spinBox_3, *spinBox_4, *spinBox_5, *spinBox_6;

    QSerialPort* m_serialPort = new QSerialPort();
    QStringList thl;
    QString LED_Code;
    QString save_LED_Code;

private slots:
    void receiveInfo();
    void send_MSG();

private:
    void init();
    void init_SerialPort();

};

#endif // BANNAWIDGET_H
