#include "bannawidget.h"
#include "ui_bannawidget.h"

int temp;
int humi;
int light;
bool isTempWarning;
bool isHumiWarning;
bool isLightWarning;

// 构造函数
BannaWidget::BannaWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannaWidget)
{
    ui->setupUi(this);
    connect(s, SIGNAL(sender()), this, SLOT(send_MSG()));

    label = new QLabel(this);
       label_2 = new QLabel(this);
       label_3 = new QLabel(this);
       label_4 = new QLabel(this);
       label_5 = new QLabel(this);
       label_6 = new QLabel(this);
       label_7 = new QLabel(this);
       label_8 = new QLabel(this);
       label_9 = new QLabel(this);

       lcdNumber = new QLCDNumber(this);
       lcdNumber_2 = new QLCDNumber(this);
       lcdNumber_3 = new QLCDNumber(this);

       spinBox = new QSpinBox(this);
       spinBox_2 = new QSpinBox(this);
       spinBox_3 = new QSpinBox(this);
       spinBox_4 = new QSpinBox(this);
       spinBox_5 = new QSpinBox(this);
       spinBox_6 = new QSpinBox(this);

       gridLayout = new QGridLayout(ui->frameBackground);
       init();
       init_SerialPort();
       ui->frameBackground->setLayout(gridLayout);

}

void BannaWidget::init()
{
    label->setText("温度:");
    label->setStyleSheet("font-size:24px;");
    gridLayout->addWidget(label, 0, 1, 2, 1);
    label_2->setText("湿度:");
    label_2->setStyleSheet("font-size:24px;");
    gridLayout->addWidget(label_2, 2, 1, 2, 1);
    label_3->setText("亮度:");
    label_3->setStyleSheet("font-size:24px;");
    gridLayout->addWidget(label_3, 4, 1, 2, 1);

    gridLayout->addWidget(lcdNumber, 0, 2, 2, 3);
    gridLayout->addWidget(lcdNumber_2, 2, 2, 2, 3);
    gridLayout->addWidget(lcdNumber_3, 4, 2, 2, 3);

    label_4->setText("阈值上限:");
    gridLayout->addWidget(label_4, 0, 5, 1, 1);
    label_5->setText("阈值下限：");
    gridLayout->addWidget(label_5, 1, 5, 1, 1);
    label_6->setText("阈值上限:");
    gridLayout->addWidget(label_6, 2, 5, 1, 1);
    label_7->setText("阈值下限：");
    gridLayout->addWidget(label_7, 3, 5, 1, 1);
    label_8->setText("阈值上限:");
    gridLayout->addWidget(label_8, 4, 5, 1, 1);
    label_9->setText("阈值下限：");
    gridLayout->addWidget(label_9, 5, 5, 1, 1);

    spinBox->setRange(-99, 199);
    spinBox->setValue(50);
    gridLayout->addWidget(spinBox, 0, 6, 1, 1);
    spinBox_2->setRange(-99, 199);
    spinBox_2->setValue(25);
    gridLayout->addWidget(spinBox_2, 1, 6, 1, 1);
    spinBox_3->setRange(0, 100);
    spinBox_3->setValue(50);
    gridLayout->addWidget(spinBox_3, 2, 6, 1, 1);
    spinBox_4->setRange(0, 99);
    spinBox_4->setValue(20);
    gridLayout->addWidget(spinBox_4, 3, 6, 1, 1);
    spinBox_5->setRange(0, 9999);
    spinBox_5->setValue(3000);
    gridLayout->addWidget(spinBox_5, 4, 6, 1, 1);
    spinBox_6->setRange(0, 9999);
    spinBox_6->setValue(500);
    gridLayout->addWidget(spinBox_6, 5, 6, 1, 1);

}


void BannaWidget::init_SerialPort()
{

    QStringList m_serialPortName;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        m_serialPortName << info.portName();
        // comboBox->addItem(info.portName());
        //qDebug() << "serialPortName:" << info.portName();
    }

//  baud_list = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    QVector<int> baud_list = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    /*foreach(const int &baud, baud_list)
    {
        // comboBox_2->addItem(QString::number(baud));
    }
    */

    if(m_serialPort->isOpen())
    {
        m_serialPort->clear();
        m_serialPort->close();
    }

    // comboBox_2->setCurrentIndex(baud_list.length() - 1);
    // comboBox->setCurrentText(m_serialPortName[0]);
    m_serialPort->setPortName(m_serialPortName[0]);

    if(!m_serialPort->open(QIODevice::ReadWrite))
    {
        //qDebug() << m_serialPortName[0] << ": open failed, please try again.";
        return ;
    }

    m_serialPort->setBaudRate(115200,QSerialPort::AllDirections);//设置波特率和读写方向
    m_serialPort->setDataBits(QSerialPort::Data8);		//数据位为8位
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
    m_serialPort->setParity(QSerialPort::NoParity);	//无校验位
    m_serialPort->setStopBits(QSerialPort::OneStop); //一位停止位

    connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(receiveInfo()));
}

void BannaWidget::receiveInfo()
{
    QByteArray info = this->m_serialPort->readAll();

    qDebug() << info;

    if(info.length()==8){

        char led[2];
        int alert=0 ;

        thl << info;

        temp = thl[0].mid(0, 2).toInt();
        humi = thl[0].mid(2, 2).toInt();
        light = thl[0].mid(4, 4).toInt();
        // qDebug() << "temp:" << temp << "humi:" << humi << "light:" << light;

        lcdNumber->display(temp);
        lcdNumber_2->display(humi);
        lcdNumber_3->display(light);


        if(temp > spinBox->value() || temp < spinBox_2->value())
        {lcdNumber->setStyleSheet("color:#FF0000"); isTempWarning = true; alert+=2;
            //m_serialPort->write("10");
        }else
        {lcdNumber->setStyleSheet("color:#000000"); isTempWarning = false;
            //m_serialPort->write("00");
        }


        if(humi > spinBox_3->value() || humi < spinBox_4->value())
        {lcdNumber_2->setStyleSheet("color:#FF0000");  isHumiWarning = true;alert+=2;
            //m_serialPort->write("10");
        }else
        {lcdNumber_2->setStyleSheet("color:#000000"); isHumiWarning = false;
            //m_serialPort->write("00");
        }

        if(light > spinBox_5->value() || light < spinBox_6->value() )
        {lcdNumber_3->setStyleSheet("color:#FF0000");  isLightWarning = true;alert+=2;
            //m_serialPort->write("10");
        }else
        {lcdNumber_3->setStyleSheet("color:#000000"); isLightWarning = false;
        }

        led[1]='0';
        if(alert!=0){
            if(alert==2)led[0]='2';
            else if(alert==4)led[0]='4';
            else if(alert==6)led[0]='6';
            m_serialPort->write(led, 1);

        }else led[0]='0';
        //led[1]='0';


        //m_serialPort->write(led, 1);

        qDebug() << endl << led[0] << endl;
        led[0] = 0;
        alert=0;
        // save_LED_Code = LED_Code;
        thl.clear();
    }
    // qDebug() << info << endl;

}


void BannaWidget::send_MSG()
{
    qDebug() << sendMsg << QString(sendMsg).length() << endl;
    m_serialPort->write(sendMsg, QString(sendMsg).length());
}


// 析构函数
BannaWidget::~BannaWidget()
{
    delete ui;
}
