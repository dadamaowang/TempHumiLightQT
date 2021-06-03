# Awesome CC2530 code && TempHumiLightQT
This repo contains a series of MCU control codes based on the CC2530 chip and finally developed an intelligent  monitor system that combined the control of CC2530, Zigbee and User Interface with C++ QT Creater.

(本项目包含一系列基于CC2530芯片开发的单片机控制代码，并最终开发出一个完整的智能温湿度光照监控程序；)

Some implementation functions: control LCD, LED lights, control sensors, wireless node networking, wireless node time synchronization

一些实现功能：控制LCD,LED灯，控制传感器，无线节点组网，无线节点时间同步

TempHumiLightQT : A complete project to check and adjust the temperature, humity and light in a room.

TempHumiLightQT :完整的检查并调整室内温度，光照，湿度的程序


## TempHumiLightQT

![01](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/01.PNG)

![02](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/02.PNG)

![03](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/03.PNG)

![04](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/04.PNG)

![05](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/05.PNG)

![06](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/06.PNG)

![07](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/07.PNG)

![08](https://github.com/dadamaowang/TempHumiLightQT/blob/main/supplementaryMaterials/08.PNG)



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

