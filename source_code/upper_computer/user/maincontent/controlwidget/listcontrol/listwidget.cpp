/*****************************************
 * 作者: YYC
 * 日期: 2021-01-07
 * 功能：列表窗口，放置列表控件
 * ***************************************/
#include "listwidget.h"
#include "ui_listwidget.h"

char sendMsg[64];
TEMP *s = new TEMP();

// 构造函数
ListWidget::ListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ListWidget)
{
    ui->setupUi(this);
    this->initValue();
    textEdit = new QTextEdit(this);
    textEdit->setGeometry(50, 50, 700, 250);

    pushButton = new QPushButton(this);
    pushButton->setGeometry(600, 310, 100, 30);
    pushButton->setText("发送");

    textBrowser = new QTextBrowser(this);
    textBrowser->setGeometry(50, 350, 700, 250);
    textBrowser->setStyleSheet("color:#00FF00;background-color:black;font-size:20px");

    connect(pushButton, SIGNAL(clicked()), this, SLOT(on_pushButton_clicked()));
}

// 析构函数
ListWidget::~ListWidget()
{
    delete ui;
}

// 初始化数据
void ListWidget::initValue()
{

}

void ListWidget::on_pushButton_clicked()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString time = current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    QString msg = textEdit->toPlainText();
    memset(sendMsg, 0, 64);

    sendMsg[0] = 30;
    sendMsg[1] = msg.length();
    for (int i = 0; i < msg.length(); i++)
    {
        *(sendMsg + 2 + i) = msg.at(i).unicode();
    }

    emit s->sender();

    textBrowser->append(time + "\t" + msg);
}
