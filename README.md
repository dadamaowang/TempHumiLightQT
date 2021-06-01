# Awesome CC2530 code && TempHumiLightQT
This repo contains several c files to help you realize some functions with CC2530 chips, and finally developed an intelligent system that combined the control of CC2530, Zigbee and User Interface with C++ QT Creater.

本项目包含一系列基于CC2530芯片开发的单片机控制代码，并最终开发出一个完整的智能温湿度光照监控程序；

Some functions: control LCD, control LED, control sensors, made a little wireless network topology, wireless nodes synchronize

一些实现功能：控制LCD,LED灯，控制传感器，无线节点组网，无线节点时间同步

The TempHumiLightQT : A complete project to check and adjust the temperature, humity and light in a room.

TempHumiLightQT :完整的检查并调整室内温度，光照，湿度的程序



## requirements

硬件(hardware)：
包含CC2530芯片，带LCD，LED的智能主板
(CC2530chips with LCD and LED)
温湿度及光电传感器SHT10
(sensor SHT10)
CC-Debuger ： 烧录.hex文件给智能主板
(CC-Debuger to write the code to the board)
PC一台
(A PC)
DB线 (公母口) ： 连接智能主板和PC主机箱
(A DB Line)
USB数据线若干
(Some USBs)

软件(software)：
IAR Embedded Workbench8051 6.4
QT Creater Community 4.11.1 / 4.13.2



## Content

function-code  realized some functions. Including:

01LEDControl: Light 6 LED in turn

02Interrupt: Test for interrupt

03SensorA : Control  sensors

04SimpleRF: Simple RF communication

05RF: RF communication

06Spectrum: Analyze the Spectrum of each channel

07Net: A tree-topology wireless network

08Syn: Synchronization of wireless nodes



source-code : the TempHumiLightQT project

