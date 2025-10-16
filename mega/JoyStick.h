/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            JoyStick.h
** Latest modified date: 2019-05-15
** Latest version:       V1.0.0
** Description:          SmartKit JoyStick API
**
**------------------------------------------------------------
** Created by:
** Created date:         2019-05-15
** Version:              V1.0.0
** Descriptions:         SmartKit JoyStick API
**
**------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*************************************************************/
#ifndef JoyStick_H
#define JoyStick_H

#include "Arduino.h"

class JOYSTICK
{
public:
    int Init(int joyStickPinX,  /* JoyStick ��ʼ�� */
             int joyStickPinY,  
             int joyStickPinZ); 
    int ReadXYValue(int axis);  /* ��ȡX,Y���ģ���� */
    int CheckPressState(void);  /* ���ҡ�˰���״̬ */
private:
    int gJoyStickPinX;          /* ҡ��X������ */
    int gJoyStickPinY;          /* ҡ��Y������ */
    int gJoyStickPinZ;          /* ҡ��Z������ */
};

#endif /* JoyStick_H */

