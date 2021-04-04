/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：表格窗口
 * ***************************************/
#include "tablewidget.h"
#include "ui_tablewidget.h"

// 构造函数
TableWidget::TableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableWidget)
{
    ui->setupUi(this);
    this->initValue();
}

// 析构函数
TableWidget::~TableWidget()
{
    delete ui;
}

// 初始化数据
void TableWidget::initValue()
{

}
