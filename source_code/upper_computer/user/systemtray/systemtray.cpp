﻿/*****************************************
 * 作者: YYC
 * 日期: 2020-04-26
 * 功能：系统托盘，未进行优化，只有最简单的
 * 功能，后续会继续优化
 * ***************************************/
#include "systemtray.h"
#include "globaldef.hpp"
#include <QDebug>
#include <QIcon>
#include <QUrl>
#include <QDesktopServices>

// 构造函数
SystemTray::SystemTray(QWidget * parent)
    :QSystemTrayIcon(parent)
{
    this->createAction();  //创建托盘菜单
    this->addAction();     //菜单添加事件
}

// 析构函数
SystemTray::~SystemTray()
{

}

// 添加菜单
void SystemTray::addAction()
{
    myMenu->addAction(showWidget);
    myMenu->addAction(minWidget);
    myMenu->addAction(aboutSoftWare);
    myMenu->addAction(exitSoftWare);
}

// 创建菜单
void SystemTray::createAction()
{
    myMenu = new QMenu();
    showWidget = new QAction(tr("显示窗口 "), this);
    minWidget = new QAction(tr("隐藏窗口 "), this);
    aboutSoftWare = new QAction(tr("关于软件 "), this);
    exitSoftWare = new QAction(tr("退出软件 "), this);

    this->setIcon(QIcon(GlobalSpace::LOGO_PATH));
    this->setToolTip(tr("智能温湿度光照监控系统"));

    connect(exitSoftWare, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    connect(aboutSoftWare, SIGNAL(triggered(bool)), this, SLOT(aboutSlot()));

    this->setContextMenu(myMenu);
    this->show();
}

// 获取关于事件
QAction *SystemTray::getAboutSoftWare() const
{
    return aboutSoftWare;
}

// 关于软件
void SystemTray::aboutSlot()
{
    QDesktopServices::openUrl(QUrl("https://github.com/YYC572652645/QCoolPage"));
}

// 获取最小化事件
QAction *SystemTray::getMinWidget() const
{
    return minWidget;
}

// 获取显示事件
QAction *SystemTray::getShowWidget() const
{
    return showWidget;
}
