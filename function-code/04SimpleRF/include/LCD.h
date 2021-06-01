/**********************************************************************************************************************************************************
* 文 件 名：LCD.h
* 功    能：SO12864FPD-13ASBE(3S)点阵图形液晶模块（ST7565P）驱动头文件。
* 硬件连接：SO12864FPD-13ASBE(3S)点阵图形液晶模块（ST7565P）与CC2530的硬件连接关系如下所示：
*
*
*           SO12864FPD-13ASBE(3S)                CC2530
*
*               SCL(PIN7)                         P1.5
*               SID(PIN8)                         P1.6
*               A0(PIN14)                         P1.7
*               CSn(PIN12)                        P1.4
*               RESETn(PIN13)                     P2.0
*               VDD(PIN9)
*               VSS(PIN10)
*               LED+(PIN11)
* 版    本：V1.0
* 作    者：WUXIANHAI
* 日    期：2011.1.19
* 奥尔斯电子主页：www.ourselec.com
**********************************************************************************************************************************************************/
#ifndef LCD_H
#define LCD_H
#include "hal.h"

// 注  意：使用GUI函数前，请先进行GUI初始化(GUI_Init())并设置前景色和背景色(GUI_SetColor())！

/* 相关引脚定义 */
/*===================================================*/
#define      LCM_SCL            P1_5
#define      LCM_SID            P1_6
#define      LCM_A0             P1_7
#define      LCM_CSn            P1_4
#define      LCM_RESETn         P2_0
#define      H_LCM_SCL()        LCM_SCL=1
#define      L_LCM_SCL()        LCM_SCL=0
#define      H_LCM_SID()        LCM_SID=1
#define      L_LCM_SID()        LCM_SID=0
#define      H_LCM_A0()         LCM_A0=1
#define      L_LCM_A0()         LCM_A0=0
#define      H_LCM_CSn()        LCM_CSn=1
#define      L_LCM_CSn()        LCM_CSn=0
#define      H_LCM_RESETn()     LCM_RESETn=1
#define      L_LCM_RESETn()     LCM_RESETn=0
/*===================================================*/

/* SO12864FPD-13ASBE(3S)的命令字 */
/*===================================================*/
#define      LCM_CMD_DisplayON                0xAF                 // 显示开
#define      LCM_CMD_DisplayOFF               0xAE                 // 显示关

#define      LCM_CMD_DisplayStartLineSet(x)   (0x40 | x)           // 指定显示起始行地址, 0<=x<=63
                                                                   // LCM从上到下依次对应显示RAM的32，31，30，......,1,0,63,62,......,33 行
                                                                   // 因此一般情况下，x应取32，即将显示起始行设在LCM最顶部。
                                                                   // 通过有规律的改变起始行，可以实现上下滚屏，但要注意在滚屏结束后，将原先
                                                                   // 设定的起始行重新设定。

#define      LCM_CMD_PageAddressSet(x)        (0xB0 | x)           // 指定页地址,0<=x<=8

#define      LCM_CMD_ColumnAddressHSet(x)     (0x10 | (x>>4))      // 列地址高四位，0<=x<=131
#define      LCM_CMD_ColumnAddressLSet(x)     (0x00 | (x & 0x0F))  // 列地址低四位，0<=x<=131

#define      LCM_CMD_ADC_Normal               0xA0                 // 模块正向安装，列地址从左到右为0~127
#define      LCM_CMD_ADC_Reverse              0xA1                 // 模块反向安装，列地址从左到右为131~4

#define      LCM_CMD_CommonOutputMode_Normal  0xC0                 // 模块的下端（注意：并不是指LCM最底部）为第0行，往上依次为63，62，......2，1
                                                                   // 当模块反向安装时使用
#define      LCM_CMD_CommonOutputMode_Reverse 0xC8                 // 模块的上端（注意：并不是指LCM最顶部）为第0行，往下依次为63，62，......2，1
                                                                   // 当模块正向安装时使用

#define      LCM_CMD_Display_Normal           0xA6                 // 正常显示
#define      LCM_CMD_Display_Reverse          0xA7                 // 反白显示

#define      LCM_CMD_DisplayAllPointsON       0xA5                 //
#define      LCM_CMD_DisplayAllPointsOFF      0xA4                 //

#define      LCM_CMD_LCDBias_1_9              0xA2                 // LCD的偏压比1/9(本模块固定为1/9)
#define      LCM_CMD_LCDBias_1_7              0xA3                 // LCD的偏压比1/7

#define      LCM_CMD_PowerControl             0x2F                 // 开关内部电路的电源。本模块应设置成0x2F

#define      LCM_CMD_V0Set(x)                 (0x20 | x)           // 粗调对比度，0<=x<=7
                                                                   // 在5V电压模式下，x取4较合适

#define      LCM_CMD_ElectronicVolumeMode     0x81                 // 进入细调对比度模式
#define      LCM_CMD_ElectronicVolumeSet(x)   (0x80 | x)           // 细调对比度，1<=x<=63
                                                                   // 注意：要细调对比度，首先要进入细调对比度模式
                                                                   // 在5V电压模式下，x取32较合适

#define      LCM_CMD_StaticIndicatorOFF       0xAC
#define      LCM_CMD_StaticIndicatorOFF_Feed  0x00
/*===================================================*/

/* 声明构成字体点阵的常量(0x00-0xff) */
/*===================================*/
#define	________	0x00
#define	_______X	0x01
#define	______X_	0x02
#define	______XX	0x03
#define	_____X__	0x04
#define	_____X_X	0x05
#define	_____XX_	0x06
#define	_____XXX	0x07
#define	____X___	0x08
#define	____X__X	0x09
#define	____X_X_	0x0a
#define	____X_XX	0x0b
#define	____XX__	0x0c
#define	____XX_X	0x0d
#define	____XXX_	0x0e
#define	____XXXX	0x0f
#define	___X____	0x10
#define	___X___X	0x11
#define	___X__X_	0x12
#define	___X__XX	0x13
#define	___X_X__	0x14
#define	___X_X_X	0x15
#define	___X_XX_	0x16
#define	___X_XXX	0x17
#define	___XX___	0x18
#define	___XX__X	0x19
#define	___XX_X_	0x1a
#define	___XX_XX	0x1b
#define	___XXX__	0x1c
#define	___XXX_X	0x1d
#define	___XXXX_	0x1e
#define	___XXXXX	0x1f
#define	__X_____	0x20
#define	__X____X	0x21
#define	__X___X_	0x22
#define	__X___XX	0x23
#define	__X__X__	0x24
#define	__X__X_X	0x25
#define	__X__XX_	0x26
#define	__X__XXX	0x27
#define	__X_X___	0x28
#define	__X_X__X	0x29
#define	__X_X_X_	0x2a
#define	__X_X_XX	0x2b
#define	__X_XX__	0x2c
#define	__X_XX_X	0x2d
#define	__X_XXX_	0x2e
#define	__X_XXXX	0x2f
#define	__XX____	0x30
#define	__XX___X	0x31
#define	__XX__X_	0x32
#define	__XX__XX	0x33
#define	__XX_X__	0x34
#define	__XX_X_X	0x35
#define	__XX_XX_	0x36
#define	__XX_XXX	0x37
#define	__XXX___	0x38
#define	__XXX__X	0x39
#define	__XXX_X_	0x3a
#define	__XXX_XX	0x3b
#define	__XXXX__	0x3c
#define	__XXXX_X	0x3d
#define	__XXXXX_	0x3e
#define	__XXXXXX	0x3f
#define	_X______	0x40
#define	_X_____X	0x41
#define	_X____X_	0x42
#define	_X____XX	0x43
#define	_X___X__	0x44
#define	_X___X_X	0x45
#define	_X___XX_	0x46
#define	_X___XXX	0x47
#define	_X__X___	0x48
#define	_X__X__X	0x49
#define	_X__X_X_	0x4a
#define	_X__X_XX	0x4b
#define	_X__XX__	0x4c
#define	_X__XX_X	0x4d
#define	_X__XXX_	0x4e
#define	_X__XXXX	0x4f
#define	_X_X____	0x50
#define	_X_X___X	0x51
#define	_X_X__X_	0x52
#define	_X_X__XX	0x53
#define	_X_X_X__	0x54
#define	_X_X_X_X	0x55
#define	_X_X_XX_	0x56
#define	_X_X_XXX	0x57
#define	_X_XX___	0x58
#define	_X_XX__X	0x59
#define	_X_XX_X_	0x5a
#define	_X_XX_XX	0x5b
#define	_X_XXX__	0x5c
#define	_X_XXX_X	0x5d
#define	_X_XXXX_	0x5e
#define	_X_XXXXX	0x5f
#define	_XX_____	0x60
#define	_XX____X	0x61
#define	_XX___X_	0x62
#define	_XX___XX	0x63
#define	_XX__X__	0x64
#define	_XX__X_X	0x65
#define	_XX__XX_	0x66
#define	_XX__XXX	0x67
#define	_XX_X___	0x68
#define	_XX_X__X	0x69
#define	_XX_X_X_	0x6a
#define	_XX_X_XX	0x6b
#define	_XX_XX__	0x6c
#define	_XX_XX_X	0x6d
#define	_XX_XXX_	0x6e
#define	_XX_XXXX	0x6f
#define	_XXX____	0x70
#define	_XXX___X	0x71
#define	_XXX__X_	0x72
#define	_XXX__XX	0x73
#define	_XXX_X__	0x74
#define	_XXX_X_X	0x75
#define	_XXX_XX_	0x76
#define	_XXX_XXX	0x77
#define	_XXXX___	0x78
#define	_XXXX__X	0x79
#define	_XXXX_X_	0x7a
#define	_XXXX_XX	0x7b
#define	_XXXXX__	0x7c
#define	_XXXXX_X	0x7d
#define	_XXXXXX_	0x7e
#define	_XXXXXXX	0x7f
#define	X_______	0x80
#define	X______X	0x81
#define	X_____X_	0x82
#define	X_____XX	0x83
#define	X____X__	0x84
#define	X____X_X	0x85
#define	X____XX_	0x86
#define	X____XXX	0x87
#define	X___X___	0x88
#define	X___X__X	0x89
#define	X___X_X_	0x8a
#define	X___X_XX	0x8b
#define	X___XX__	0x8c
#define	X___XX_X	0x8d
#define	X___XXX_	0x8e
#define	X___XXXX	0x8f
#define	X__X____	0x90
#define	X__X___X	0x91
#define	X__X__X_	0x92
#define	X__X__XX	0x93
#define	X__X_X__	0x94
#define	X__X_X_X	0x95
#define	X__X_XX_	0x96
#define	X__X_XXX	0x97
#define	X__XX___	0x98
#define	X__XX__X	0x99
#define	X__XX_X_	0x9a
#define X__XX_XX	0x9b
#define X__XXX__	0x9c
#define X__XXX_X	0x9d
#define	X__XXXX_	0x9e
#define	X__XXXXX	0x9f
#define	X_X_____	0xa0
#define	X_X____X	0xa1
#define	X_X___X_	0xa2
#define	X_X___XX	0xa3
#define	X_X__X__	0xa4
#define	X_X__X_X	0xa5
#define	X_X__XX_	0xa6
#define	X_X__XXX	0xa7
#define	X_X_X___	0xa8
#define	X_X_X__X	0xa9
#define	X_X_X_X_	0xaa
#define	X_X_X_XX	0xab
#define	X_X_XX__	0xac
#define	X_X_XX_X	0xad
#define	X_X_XXX_	0xae
#define	X_X_XXXX	0xaf
#define	X_XX____	0xb0
#define X_XX___X	0xb1
#define	X_XX__X_	0xb2
#define	X_XX__XX	0xb3
#define	X_XX_X__	0xb4
#define	X_XX_X_X	0xb5
#define	X_XX_XX_	0xb6
#define	X_XX_XXX	0xb7
#define	X_XXX___	0xb8
#define	X_XXX__X	0xb9
#define	X_XXX_X_	0xba
#define	X_XXX_XX	0xbb
#define	X_XXXX__	0xbc
#define	X_XXXX_X	0xbd
#define	X_XXXXX_	0xbe
#define	X_XXXXXX	0xbf
#define	XX______	0xc0
#define	XX_____X	0xc1
#define	XX____X_	0xc2
#define	XX____XX	0xc3
#define	XX___X__	0xc4
#define	XX___X_X	0xc5
#define	XX___XX_	0xc6
#define	XX___XXX	0xc7
#define	XX__X___	0xc8
#define	XX__X__X	0xc9
#define	XX__X_X_	0xca
#define	XX__X_XX	0xcb
#define	XX__XX__	0xcc
#define	XX__XX_X	0xcd
#define	XX__XXX_	0xce
#define XX__XXXX	0xcf
#define	XX_X____	0xd0
#define	XX_X___X	0xd1
#define	XX_X__X_	0xd2
#define	XX_X__XX	0xd3
#define	XX_X_X__	0xd4
#define	XX_X_X_X	0xd5
#define	XX_X_XX_	0xd6
#define	XX_X_XXX	0xd7
#define	XX_XX___	0xd8
#define	XX_XX__X	0xd9
#define	XX_XX_X_	0xda
#define	XX_XX_XX	0xdb
#define	XX_XXX__	0xdc
#define	XX_XXX_X	0xdd
#define	XX_XXXX_	0xde
#define	XX_XXXXX	0xdf
#define	XXX_____	0xe0
#define	XXX____X	0xe1
#define	XXX___X_	0xe2
#define	XXX___XX	0xe3
#define	XXX__X__	0xe4
#define	XXX__X_X	0xe5
#define	XXX__XX_	0xe6
#define	XXX__XXX	0xe7
#define	XXX_X___	0xe8
#define	XXX_X__X	0xe9
#define	XXX_X_X_	0xea
#define	XXX_X_XX	0xeb
#define	XXX_XX__	0xec
#define	XXX_XX_X	0xed
#define	XXX_XXX_	0xee
#define	XXX_XXXX	0xef
#define	XXXX____	0xf0
#define	XXXX___X	0xf1
#define	XXXX__X_	0xf2
#define	XXXX__XX	0xf3
#define	XXXX_X__	0xf4
#define	XXXX_X_X	0xf5
#define	XXXX_XX_	0xf6
#define	XXXX_XXX	0xf7
#define	XXXXX___	0xf8
#define	XXXXX__X	0xf9
#define	XXXXX_X_	0xfa
#define	XXXXX_XX	0xfb
#define	XXXXXX__	0xfc
#define	XXXXXX_X	0xfd
#define	XXXXXXX_	0xfe
#define	XXXXXXXX	0xff
/*===================================*/

/********************************************************************************************
* 函数名称：LCM_DataSend()
* 功    能：串行模式发送数据
* 入口参数：data    要发送的数据
* 出口参数：无
*********************************************************************************************/
void LCM_DataSend(UINT8 data);

/********************************************************************************************
* 函数名称：LCM_WriteCMD()
* 功    能：写命令
* 入口参数：command    命令
* 出口参数：无
*********************************************************************************************/
void LCM_WriteCMD(UINT8 command);

/********************************************************************************************
* 函数名称：LCM_WriteData()
* 功    能：写数据
* 入口参数：data    数据
* 出口参数：无
*********************************************************************************************/
void LCM_WriteData(UINT8 data);

/********************************************************************************************
* 函数名称：LCM_Init()
* 功    能：SO12864FPD-13ASBE(3S)初始化
* 入口参数：无
* 出口参数：无
*********************************************************************************************/
void LCM_Init(void);

/********************************************************************************************
* 函数名称：LCM_ShowBitmap12864()
* 功    能：在SO12864FPD-13ASBE(3S)上显示一幅128*64点阵单色位图。
* 入口参数：bitmap12864  128*64点阵单色位图数据
* 出口参数：无
*********************************************************************************************/
void LCM_ShowBitmap12864(UINT8 *bitmap12864);

/********************************************************************************************
* 函数名称：LCM_Refresh()
* 功    能：将显示缓冲区中的数据刷新到SO12864FPD-13ASBE(3S)上显示。
* 入口参数：无
* 出口参数：无
*********************************************************************************************/
void LCM_Refresh(void);

/******************************************************************************************************************
* 函数名称：WriteByteToDisplayBuffer()
* 功    能：向显示缓冲区中指定点写一字节数据
* 入口参数：x 		指定点横坐标值(0-127)
*           y           指定点纵坐标值(0-63)
*           data	所要写的数据
* 出口参数：无
*******************************************************************************************************************/
void WriteByteToDisplayBuffer(UINT8 x, UINT8 y, UINT8 data);

/******************************************************************************************************************
* 函数名称：ReadByteFromDisplayBuffer()
* 功    能：从显示缓冲区中指定点读一字节数据
* 入口参数：x 		指定点横坐标值(0-127)
*           y           指定点纵坐标值(0-63)
* 出口参数：返回该点上的字节数据。
*******************************************************************************************************************/
UINT8 ReadByteFromDisplayBuffer(UINT8 x, UINT8 y);

/******************************************************************************************************************
* 函数名称：GUI_FillScreen()
* 功    能：全屏填充。填充显示缓冲区。
* 入口参数：data      填充的数据
* 出口参数：无
*******************************************************************************************************************/
void GUI_FillScreen(UINT8 data);

/******************************************************************************************************************
* 函数名称：GUI_ClearScreen()
* 功    能：清屏。清除显示缓冲区。
* 入口参数：无
* 出口参数：无
*******************************************************************************************************************/
void GUI_ClearScreen(void);

/******************************************************************************************************************
* 函数名称：GUI_Init()
* 功    能：初始化GUI。
* 入口参数：无
* 出口参数：无
*******************************************************************************************************************/
void GUI_Init(void);

/******************************************************************************************************************
* 函数名称：GUI_Point()
* 功    能：在LCM12864指定位置对应的显示缓冲区画点。
* 入口参数：x        指定点所在列的位置
*           y        指定点所在行的位置
*           color    对于黑白色或蓝白色LCM12864，为0时灭，为1时显示
* 出口参数：返回值为1时表示操作成功，为0时表示操作失败。操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
UINT8  GUI_Point(UINT8 x, UINT8 y, UINT8 color);

/******************************************************************************************************************
* 函数名称：GUI_ReadPoint()
* 功    能：在LCM12864指定位置对应的显示缓冲区读取指定点的颜色。
* 入口参数：x        指定点所在列的位置
*           y        指定点所在行的位置
*           ret      保存颜色值的指针
* 出口参数：返回0表示指定地址超出缓冲区范围。
* 说    明：对于单色，设置ret的d0位为1或0
*******************************************************************************************************************/
UINT8  GUI_ReadPoint(UINT8 x, UINT8 y, UINT8 *ret);

/******************************************************************************************************************
* 函数名称：GUI_HLine()
* 功    能：在LCM12864的显示缓冲区画水平线。
* 入口参数：x0		水平线起点所在列的位置
*           y0		水平线起点所在行的位置
*           x1      水平线终点所在列的位置
*           color	对于黑白色或蓝白色LCM12864，为0时灭，为1时显示
* 出口参数：无
*******************************************************************************************************************/
void  GUI_HLine(UINT8 x0, UINT8 y0, UINT8 x1, UINT8 color) ;

/******************************************************************************************************************
* 函数名称：GUI_RLine()
* 功    能：在LCM12864的显示缓冲区画竖直线。
* 入口参数：x0		垂直线起点所在列的位置
*           y0		垂直线起点所在行的位置
*           y1      垂直线终点所在行的位置
*           color	对于黑白色或蓝白色LCM12864，为0时灭，为1时显示
* 出口参数：无
*******************************************************************************************************************/
void  GUI_RLine(UINT8 x0, UINT8 y0, UINT8 y1, UINT8 color);

/******************************************************************************************************************
* 函数名称：GUI_Rectangle()
* 功    能：画矩形。
* 入口参数：x0		矩形左上角的x坐标值
*           y0		矩形左上角的y坐标值
*           x1      矩形右下角的x坐标值
*           y1      矩形右下角的y坐标值
*           color	对于黑白色或蓝白色LCM12864，为0时灭，为1时显示
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_Rectangle(UINT8 x0, UINT8 y0, UINT8 x1, UINT8 y1, UINT8 color);

/******************************************************************************************************************
* 函数名称：GUI_RectangleFill()
* 功    能：填充矩形。画一个填充的矩形，填充色与边框色一样。
* 入口参数：x0		矩形左上角的x坐标值
*           y0		矩形左上角的y坐标值
*           x1      矩形右下角的x坐标值
*           y1      矩形右下角的y坐标值
*           color	对于黑白色或蓝白色LCM12864，为0时灭，为1时显示
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_RectangleFill(UINT8 x0, UINT8 y0, UINT8 x1, UINT8 y1, UINT8 color);

/******************************************************************************************************************
* 函数名称：GUI_CmpColor()
* 功    能：判断颜色值是否一致。
* 入口参数：color1	颜色值1
*	    color2	颜色值2
* 出口参数：返回1表示相同，返回0表示不相同。
* 注    意：此函数定义成宏函数，以提高程序效率。
*******************************************************************************************************************/
#define  GUI_CmpColor(color1, color2)	(color1==color2)

/******************************************************************************************************************
* 函数名称：GUI_CopyColor()
* 功    能：颜色值复制。
* 入口参数：color1	目标颜色变量
*	    color2	源颜色变量
* 出口参数：无
* 注    意：此函数定义成宏函数，以提高程序效率。
*******************************************************************************************************************/
#define  GUI_CopyColor(color1, color2) 	*color1 = color2

/******************************************************************************************************************
* 函数名称：GUI_SetColor()
* 功    能：设置显示色及背景色。用于ASCII字符显示及汉字显示。
* 入口参数：color1	显示色的值
*           color2	背景色的值
* 出口参数：无
* 说    明：
*******************************************************************************************************************/
void GUI_SetColor(UINT8 color1, UINT8 color2);

/******************************************************************************************************************
* 函数名称：GUI_GetBackColor()
* 功    能：取得当前背景色。
* 入口参数：bakc        保存颜色的变量地址
* 出口参数：无
* 说    明：
*******************************************************************************************************************/
void GUI_GetBackColor(UINT8  *bakc);

/******************************************************************************************************************
* 函数名称：GUI_GetDispColor()
* 功    能：取得当前前景色（显示色）。
* 入口参数：bakc		保存颜色的变量地址
* 出口参数：无
* 说    明：
*******************************************************************************************************************/
void GUI_GetDispColor(UINT8  *bakc);

/******************************************************************************************************************
* 函数名称：GUI_ExchangeColor()
* 功    能：交换前景色（显示色）与背景色。用于反相显示。
* 入口参数：无
* 出口参数：无
* 说    明：
*******************************************************************************************************************/
void GUI_ExchangeColor(void);

/******************************************************************************************************************
* 函数名称：GUI_LoadLine()
* 功    能：输出单色图形的一行数据。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据。:横向取模，字节顺序。
*           no          要显示此行的点个数
* 出口参数：返回值为1时表示操作成功，为0时表示操作失败。
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
UINT8 GUI_LoadLine(UINT8 x, UINT8 y, UINT8 *dat, UINT8 no);

/******************************************************************************************************************
* 函数名称：GUI_LoadBitmap()
* 功    能：输出单色图形数据(位图)。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据:横向取模，字节顺序。
*           hno         要显示此行的点个数
*           lno         要显示此列的点个数
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_LoadBitmap(UINT8 x, UINT8 y, UINT8 *dat, UINT8 hno, UINT8 lno);

/******************************************************************************************************************
* 函数名称：GUI_LoadBitmapN()
* 功    能：输出单色图形数据(位图)，反色显示。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据:横向取模，字节顺序。
*           hno         要显示此行的点个数
*           lno         要显示此列的点个数
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_LoadBitmapN(UINT8 x, UINT8 y, UINT8 *dat, UINT8 hno, UINT8 lno);

/******************************************************************************************************************
* 函数名称：GUI_PutHZ()
* 功    能：显示汉字。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的汉字点阵数据:横向取模，字节顺序。
*           hno         要显示此行的点个数
*           lno         要显示此列的点个数
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_PutHZ(UINT8 x, UINT8 y, UINT8 *dat, UINT8 hno, UINT8 lno);

/******************************************************************************************************************
* 函数名称：GUI_PutHZN()
* 功    能：显示汉字，反色显示。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的汉字点阵数据:横向取模，字节顺序。
*           hno         要显示此行的点个数
*           lno         要显示此列的点个数
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_PutHZN(UINT8 x, UINT8 y, UINT8 *dat, UINT8 hno, UINT8 lno);

/******************************************************************************************************************
* 函数名称：GUI_PutHZ_By_TAB()
* 功    能：根据用户建立的汉字索引表显示汉字。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的汉字点阵数据:横向取模，字节顺序。
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_PutHZ_By_TAB(UINT8 x,UINT8 y,UINT8 ch[]);

/******************************************************************************************************************
* 函数名称：GUI_PutChar5_7()
* 功    能：显示5*7字体ASCII码，显示值为20H-7FH(若为其它值，则显示' ')。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           ch		要显示的ASCII码值。
* 出口参数：返回值为1时表示操作成功，为0时表示操作失败。
* 说    明：操作失败原因是指定地址超出有效范围。(显示格式为6*8)
*******************************************************************************************************************/
UINT8 GUI_PutChar5_7(UINT8 x, UINT8 y, UINT8 ch);

/******************************************************************************************************************
* 函数名称：GUI_PutString5_7()
* 功    能：输出显示5*7字体ASCII码字符串(没有自动换行功能)。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           str		要显示的ASCII码字符串
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_PutString5_7(UINT8 x, UINT8 y, char *str);

/******************************************************************************************************************
* 函数名称：GUI_PutNoString5_7()
* 功    能：输出显示5*7字体ASCII码字符串(没有自动换行功能)，若显示的字符个数大于指定个数，则只显示指定个数的字符。
* 入口参数：x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           str		要显示的ASCII码字符串
*           no          最大显示字符的个数
* 出口参数：无
* 说    明：操作失败原因是指定地址超出有效范围。
*******************************************************************************************************************/
void GUI_PutNoString5_7(UINT8 x, UINT8 y, char *str, UINT8 no);

#endif
