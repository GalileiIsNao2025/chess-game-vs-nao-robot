/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            Button.h
** Latest modified date: 2019-05-15
** Latest version:       V1.0.0
** Description:          SmartKit Button API
**
**------------------------------------------------------------
** Created by:
** Created date:         2019-05-15
** Version:              V1.0.0
** Descriptions:         SmartKit Button API
**
**------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*************************************************************/
#ifndef Button_H
#define Button_H

#include "Arduino.h"

class BUTTON
{
public:
    void Init(int redPin,        /* Button��ʼ�� */
                    int bluePin,
                    int greenPin);
    int CheckState(int color);   /* ���Button״̬ */
private:

    int gRedPin;                 /* ��ɫ��ť�������� */
    int gBluePin;                /* ��ɫ��ť�������� */
    int gGreenPin;               /* ��ɫ��ť�������� */

    int ReadState(int button, int ioDown);
};

#endif /* Button_H */
