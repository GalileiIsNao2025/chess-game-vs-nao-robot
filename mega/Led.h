/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            Led.h
** Latest modified date: 2019-05-15
** Latest version:       V1.0.0
** Description:          SmartKit Led API
**
**------------------------------------------------------------
** Created by:
** Created date:         2019-05-15
** Version:              V1.0.0
** Descriptions:         SmartKit Led API
**
**------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*************************************************************/
#ifndef Led_H
#define Led_H

#include "Arduino.h"

class LED
{
public:
    void Init(int redPin,           /* Led ��ʼ�� */
                 int bluePin, 
                 int greenPin);
    int CheckState(int color);      /* ������ɫ�Ƶ�״̬ */
    int Turn(int color, int state); /* ���ظ���ɫ�� */
private:
    int gRedPin;                       /* ��ɫ���������� */
    int gBluePin;                      /* ��ɫ���������� */
    int gGreenPin;                     /* ��ɫ���������� */
};

#endif /* Led_H */
