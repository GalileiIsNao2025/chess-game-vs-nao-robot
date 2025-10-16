/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            Led.h
** Latest modified date: 2019-05-15
** Latest version:       V1.0.0
** Description:          SmartKit API
**
**------------------------------------------------------------
** Created by:
** Created date:         2019-05-15
** Version:              V1.0.0
** Descriptions:         SmartKit API
**
**------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*************************************************************/
#ifndef SmartKit_H
#define SmartKit_H

#include "Arduino.h"
#include "SmartKitType.h"
#include "Magician.h"

#define JOYSTICK_XPIN   7    /* JoyStick X�����ӵ����� */
#define JOYSTICK_YPIN   6    /* JoyStick Y�����ӵ����� */
#define JOYSTICK_ZPIN   A5   /* JoyStick Z�����ӵ����� */

#define LED_REDPIN      9    /* ��ɫ���������� */
#define LED_GREENPIN    A1   /* ��ɫ���������� */
#define LED_BLUEPIN     A3   /* ��ɫ���������� */

#define BUTTON_REDPIN   A0   /* ��ɫ��ť�������� */
#define BUTTON_GREENPIN A2   /* ��ɫ��ť�������� */
#define BUTTON_BLUEPIN  A4   /* ��ɫ��ť�������� */

#define SOFTSERIAL_RX_PIN  A11    /* Ӣ������RX���� */
#define SOFTSERIAL_TX_PIN  14     /* Ӣ������TX���� */

/*************************************************************
  Init: ��ʼ��
*************************************************************/

extern void SmartKit_Init(void);

/*************************************************************
  Button: ��ť
*************************************************************/

extern int SmartKit_ButtonCheckState(char color);

/*************************************************************
  JoyStick: ҡ��
*************************************************************/

extern int SmartKit_JoyStickReadXYValue(int axis);
extern int SmartKit_JoyStickCheckPressState(void);

/*************************************************************
  Led: ָʾ��
*************************************************************/

extern int SmartKit_LedCheckState(char color);
extern void SmartKit_LedTurn(char color, int state);

/*************************************************************
  VoiceCN: ��������
*************************************************************/

extern void SmartKit_VoiceCNInit(void);
extern void SmartKit_VoiceCNAddCommand(char *Voice, int num);
extern int SmartKit_VoiceCNVoiceCheck(int num);
extern void SmartKit_VoiceCNStart(void);

/*************************************************************
  VoiceENG: Ӣ������
*************************************************************/

extern int SmartKit_VoiceENGVoiceCheck(int num);
extern void SmartKit_VoiceENGStart(void);

/*************************************************************
  VIS: �Ӿ�ʶ��
*************************************************************/
extern void SmartKit_VISInit(void);
extern void SmartKit_VISSetDobotMatrix(
                float x1, float y1,
                float x2, float y2,
                float x3, float y3);
extern void SmartKit_VISSetPixyMatrix(
                float x1, float y1, float length1, float wide1,
                float x2, float y2, float length2, float wide2,
                float x3, float y3, float length3, float wide3);
extern void SmartKit_VISSetGrapAreaZ(float z);
extern float SmartKit_VISGetGrapAreaZ(void);
extern void SmartKit_VISSetAT(
                float x, 
                float y, 
                float z, 
                float r);
extern char SmartKit_VISSetColorSignature(
                char color,                                
                char signature);
extern char SmartKit_VISSetBlockTA(
                char color,
                float x,
                float y,
                float z,
                float r);
extern char SmartKit_VISGetBlockCheckNum(char color);
extern char SmartKit_VISSetBlockPlaceNum(char color, int placeNum);
extern char SmartKit_VISGetBlockPlaceNum(char color);
extern char SmartKit_VISSetBlockHeight(char color, float height);
extern void SmartKit_VISBlockParmCheckNumClear(void);
extern char SmartKit_VISRun(void);
extern int  SmartKit_VISSetGrapBlockMark(int mark);
extern int  SmartKit_VISGetGrapBlockMark(void);
extern char SmartKit_VISGrabBlock(
                char color, 
                int blockNum, 
                float r);
extern char SmartKit_VISPlaceBlock(char color);

#endif /* SmartKit_H */
