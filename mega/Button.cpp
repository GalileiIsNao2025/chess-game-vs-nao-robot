/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            Button.cpp
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
#include "Button.h"
#include "SmartKitType.h"

/*************************************************************
** Function name:	  ButtonInit
** Descriptions:	  ������ʼ��
** Input parameters:  redPin:   ��ɫ��ť��������
**                    bluePin:  ��ɫ��ť��������
**                    greenPin: ��ɫ��ť��������
** Output parameters: no
** Returned value:	  no
*************************************************************/

void BUTTON::Init(int redPin,
                  int bluePin,
                  int greenPin)
{
    gRedPin = redPin;
    gBluePin = bluePin;
    gGreenPin = greenPin;
    pinMode(gRedPin, INPUT);
    pinMode(gGreenPin, INPUT);
    pinMode(gBluePin, INPUT);
}

/*************************************************************
** Function name:	  ReadButtonState
** Descriptions:	  ��ȡ����״̬��Ĭ����UP״̬
** Output parameters: button: ��Ҫ��ȡ�İ���
**					  ioDown: Ĭ�ϰ��µ�IO״̬
** Input parameters:  no
** Returned value:	  ���ذ���״̬��UP��DOWN
*************************************************************/

int BUTTON::ReadState(int button, int ioDown)
{
    if (digitalRead(button) == ioDown)
    {
        delay(10);							/* ������������ */
        if (digitalRead(button) == ioDown)
        {
            return DOWN;
        }
    }
    else
    {
        return UP;
    }
}

/*************************************************************
** Function name:	  CheckButtonState
** Descriptions:	  ��鰴��״̬
** Input parameters:  color: ��������ɫ��
**                           �ɴ���ֵBLUE,GREEN,RED
** Output parameters: no
** Returned value:	  ���ذ���״̬��UP��DOWN
*************************************************************/

int BUTTON::CheckState(int color)
{
    int down = 1;
    int statu = UP;
    switch (color)
    {
    case BLUE:
        statu = ReadState(gBluePin, down);
        break;

    case GREEN:
        statu = ReadState(gGreenPin, down);
        break;

    case RED:
        statu = ReadState(gRedPin, down);
        break;

    default:
        return UP;
    }
    return statu;
}

