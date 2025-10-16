/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            Led.cpp
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
#include "SmartKit.h"
#include "JoyStick.h"
#include "Led.h"
#include "Button.h"
#include "VoiceRecognition.h"
#include "DobotPixy.h"
#include <SoftwareSerial.h>



JOYSTICK gJoyStick;         /* ҡ���� */
LED      gLed;              /* ���� */
BUTTON   gButton;           /* ������ */
VoiceRecognition gVoiveCN;  /* ���������� */
SoftwareSerial softSerial(SOFTSERIAL_RX_PIN,SOFTSERIAL_TX_PIN); /* Ӣ�������� */
VIS gVIS;                   /* �Ӿ�ʶ���� */

/*************************************************************
** Function name:	  SmartKitInit
** Descriptions:	  SmartKit��ʼ��,��ʼ��������ҡ��
**                                               ָʾ��
**                                               ��ť
**												 Ӣ������
**                             ע�⣺�Ӿ���Ҫ������ʼ��
**									 ����������Ҫ������ʼ��
** Input parameters:  no
** Output parameters: no
** Returned value:	  no
*************************************************************/
void SmartKit_Init(void)
{
    Serial.begin(115200);
    Serial.println("======Enter application======");
    Serial.println("======Version: 2.0.1======");
    gJoyStick.Init(JOYSTICK_XPIN,
                   JOYSTICK_YPIN, 
                   JOYSTICK_ZPIN);
	Serial.println("======JoyStick Init======");
    gLed.Init(LED_REDPIN,
              LED_BLUEPIN, 
              LED_GREENPIN);
	Serial.println("======Led Init======");
    gButton.Init(BUTTON_REDPIN,
                 BUTTON_BLUEPIN, 
                 BUTTON_GREENPIN);
	Serial.println("======Button Init======");
    softSerial.begin(9600);
	Serial.println("======VoiveEN Init======");
    Serial.println("======SmartKit Init======");
}

/*************************************************************
** Function name:	  SmartKit_VoiceCNInit
** Descriptions:	  ����������ʼ��
**                             ע�⣺�Ӿ���Ҫ������ʼ��
** Input parameters:  no
** Output parameters: no
** Returned value:	  no
*************************************************************/
void SmartKit_VoiceCNInit(void)
{
	Serial.begin(115200);
    gVoiveCN.init();
	Serial.println("======VoiveCN Init======");
}

/*************************************************************
** Function name:	  SmartKit_ButtonCheckState
** Descriptions:	  ��鰴��״̬
** Input parameters:  color: ��������ɫ��
**                           �ɴ���ֵBLUE,GREEN,RED
** Output parameters: no
** Returned value:	  ���ذ���״̬��UP��DOWN
*************************************************************/

int SmartKit_ButtonCheckState(char color)
{
    return gButton.CheckState(color);
}

/*************************************************************
** Function name:	  SmartKit_JoyStickReadXYValue
** Descriptions:	  ��ȡҡ����ֵ
** Input parameters:  axis����ϵ,�ɴ���AXISX��AXISY
** Output parameters: no
** Returned value:	  ҡ�˵���ֵ
*************************************************************/

int SmartKit_JoyStickReadXYValue(int axis)
{
    return gJoyStick.ReadXYValue(axis);
}

/*************************************************************
** Function name:	  SmartKit_JoyStickCheckPressState
** Descriptions:	  ���ҡ�˰�ѹ״̬
** Input parameters:  no
** Output parameters: no
** Returned value:	  ��ѹ״̬��UP,DOWN
*************************************************************/

int SmartKit_JoyStickCheckPressState(void)
{
    return gJoyStick.CheckPressState();
}

/*************************************************************
** Function name:	  SmartKit_LedCheckStatu
** Descriptions:	  ���LED��״̬
** Input parameters:  color: led�Ƶ���ɫ��
**                           �ɴ���ֵBLUE,GREEN,RED
** Output parameters: no
** Returned value:	  led��״̬ ON��OFF
*************************************************************/

int SmartKit_LedCheckState(char color)
{
    return gLed.CheckState(color);
}

/*************************************************************
** Function name:	  SmartKit_LedTurn
** Descriptions:	  ����LED
** Input parameters:  color: led�Ƶ���ɫ��
**                           �ɴ���ֵBLUE,GREEN,RED
**					  state: led�Ƶ�״̬��
**                           �ɴ���ֵON,OFF
** Output parameters: no
** Returned value:	  no
*************************************************************/

void SmartKit_LedTurn(char color, int state)
{
    gLed.Turn(color, state);
}

/*************************************************************
** Function name:	  SmartKit_VoiceCNAddCommand
** Descriptions:	  ������������
** Input parameters:  *Voice: ��Ҫ���ӵ�������ָ��
**					  num: ��Ҫ���ӵ����������к�
** Output parameters: no
** Returned value:	  no
*************************************************************/

void SmartKit_VoiceCNAddCommand(char *Voice, int num)
{
    gVoiveCN.addCommand(Voice, num);
}

/*************************************************************
** Function name:	  SmartKit_VoiceCNVoiceCheck
** Descriptions:	  �����������
** Input parameters:  num: ��Ҫ�������
** Output parameters: no
** Returned value:	  TRUE: ��⵽; FALSE: û�м�⵽
*************************************************************/

int SmartKit_VoiceCNVoiceCheck(int num)
{
    return gVoiveCN.VoiceCheck(num);
}

/*************************************************************
** Function name:	  SmartKit_VoiceCNStart
** Descriptions:	  ����������⿪ʼ
** Input parameters:  no
** Output parameters: no
** Returned value:	  no
*************************************************************/

void SmartKit_VoiceCNStart(void)
{
    gVoiveCN.start();
}

/*************************************************************
** Function name:	  SmartKit_VoiceENGVoiceCheck
** Descriptions:	  Ӣ���������
** Input parameters:  num: ��Ҫ�������
** Output parameters: no
** Returned value:	  TRUE: ��⵽; FALSE: û�м�⵽
*************************************************************/

int SmartKit_VoiceENGVoiceCheck(int num)
{
	static int lastNum = 0;
	int preNum = 0;
	preNum = softSerial.read();
	if (preNum > 0)
	{ 
		lastNum = preNum;
	}
	if (lastNum == num)
	{
		lastNum = 0;
		return TRUE;
	}
	return FALSE;    
}

/*************************************************************
** Function name:	  SmartKit_VoiceENGStart
** Descriptions:	  Ӣ��������⿪ʼ
** Input parameters:  no
** Output parameters: no
** Returned value:	  no
*************************************************************/

void SmartKit_VoiceENGStart(void)
{
    softSerial.listen();
}

/*************************************************************
** Function name:      Init
** Descriptions:       �Ӿ���ʼ����
**					   Ĭ������̧���߶�30��
**					   ��ʼ��֮ǰ��Ҫ�����ɫ�궨��λ�ñ궨
** Input parameters:   no
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISInit(void)
{
    gVIS.Init();
}

/*************************************************************
** Function name:      SmartKit_VISSetDobotMatrix
** Descriptions:       ���û�е�۱任����
** Input parameters:   x1: ����1��x����
**					   y1: ����1��y����
**					   x2: ����2��x����
**					   y2: ����2��y����
**					   x3: ����3��x����
**					   y3: ����3��y����
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISSetDobotMatrix(float x1, float y1,
                                float x2, float y2,
                                float x3, float y3)
{
    gVIS.SetDobotMatrix(x1, y1, x2, y2, x3, y3);
}

/*************************************************************
** Function name:      SmartKit_VISSetPixyMatrix
** Descriptions:       ����pixy�任����
** Input parameters:   x1:		����1��x����
**					   y1:		����1��y����
**					   length1: ����1�ĳ�
**					   weight1: ����1�Ŀ�
**					   x2:		����2��x����
**					   y2:		����2��y����
**					   length2: ����2�ĳ�
**					   weight2: ����2�Ŀ�
**					   x3:		����3��x����
**					   y3:		����3��y����
**					   length3: ����3�ĳ�
**					   weight3: ����3�Ŀ�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISSetPixyMatrix(
    float x1, float y1, float length1, float wide1,
    float x2, float y2, float length2, float wide2,
    float x3, float y3, float length3, float wide3)
{
    gVIS.SetPixyMatrix(
        x1, y1, length1, wide1,
        x2, y2, length2, wide2,
        x3, y3, length3, wide3);
}

/*************************************************************
** Function name:      SmartKit_VISSetGrapAreaZ
** Descriptions:       ����ץȡ����z������
** Input parameters:   z: ץȡ����߶�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISSetGrapAreaZ(float z)
{
    gVIS.SetGrapAreaZ(z);
}

/*************************************************************
** Function name:      SmartKit_VISGetGrapAreaZ
** Descriptions:       ��ȡץȡ����z������
** Input parameters:   no
** Output parameters:  no
** Returned value:     ץȡ����߶�
*************************************************************/

float SmartKit_VISGetGrapAreaZ(void)
{
    return gVIS.GetGrapAreaZ();
}

/*************************************************************
** Function name:      SmartKit_VISSetAT
** Descriptions:       ���û�е���Ӿ�ʶ����������
** Input parameters:   x:
**					   y:
**					   z:
**					   r: ��ת�Ƕ�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISSetAT(float x, float y, float z, float r)
{
    gVIS.SetAT(x, y, z, r);
}

/*************************************************************
** Function name:      SmartKit_VISSetColorSignature
** Descriptions:       ���������ɫ��ʶ
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   signature: ����ʶ
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISSetColorSignature(char color, char signature)
{
    return gVIS.SetColorSignature(color, signature);
}

/*************************************************************
** Function name:      SmartKit_VISSetBlockTA
** Descriptions:       ���ø���ɫ���Ŀ������
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   x: ����x����
**					   y: ����y����
**					   z: ����z����
**					   r: ������ת�Ƕ�
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISSetBlockTA(char color, 
                            float x, 
                            float y, 
                            float z, 
                            float r)
{
    return gVIS.SetBlockTA(color, x, y, z, r);
}

/*************************************************************
** Function name:      SmartKit_VISGetBlockCheckNum
** Descriptions:       ��ȡ�Ӿ���⵽�Ĳ�ͬ��ɫ���������
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     �������
*************************************************************/

char SmartKit_VISGetBlockCheckNum(char color)
{
    return gVIS.GetBlockCheckNum(color);
}

/*************************************************************
** Function name:      SmartKit_VISSetBlockPlaceNum
** Descriptions:       ��������������
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   placeNum: ���õ�����
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISSetBlockPlaceNum(char color, int placeNum)
{
    return gVIS.SetBlockPlaceNum(color, placeNum);
}

/*************************************************************
** Function name:      SmartKit_VISGetBlockPlaceNum
** Descriptions:       ��ȡ�����õ�����
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     �������
*************************************************************/

char SmartKit_VISGetBlockPlaceNum(char color)
{
    return gVIS.GetBlockPlaceNum(color);
}

/*************************************************************
** Function name:      SmartKit_VISSetBlockHeight
** Descriptions:       �������߶�
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   height: ���߶�
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISSetBlockHeight(char color, float height)
{
    return gVIS.SetBlockHeight(color, height);
}

/*************************************************************
** Function name:      SmartKit_VISBlockParmCheckNumClear
** Descriptions:       ����ĸ���ɫpixy��⵽���������
** Input parameters:   no
** Output parameters:  no
** Returned value:     no
*************************************************************/

void SmartKit_VISBlockParmCheckNumClear(void)
{
    gVIS.BlockParmCheckNumClear();
}

/*************************************************************
** Function name:      SmartKit_VISRun
** Descriptions:       �Ӿ�ʶ��
** Input parameters:   no
** Output parameters:  no
** Returned value:     TRUE: �������;
**					   FALSE: ԭ�������¼��ֿ���
**							  1,û�м�⵽��飬
**							  2,������ɫ�����������
*************************************************************/

char SmartKit_VISRun(void)
{
    return gVIS.Run();
}

/*************************************************************
** Function name:      SmartKit_VISSetGrapBlockMark
** Descriptions:       �������ץȡ���
** Input parameters:   mark: ���ץȡ���
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

int SmartKit_VISSetGrapBlockMark(int mark)
{
    return gVIS.SetGrapBlockMark(mark);
}

/*************************************************************
** Function name:      SmartKit_VISGetGrapBlockMark
** Descriptions:       ��ȡ���ץȡ���
** Input parameters:   no
** Output parameters:  no
** Returned value:     ���ץȡ��ǣ�TRUE�Ѿ�ץȡ��
                                     FALSE��δץȡ
*************************************************************/

int SmartKit_VISGetGrapBlockMark(void)
{
    return gVIS.GetGrapBlockMark();
}

/*************************************************************
** Function name:      SmartKit_VISGrabBlock
** Descriptions:       ץȡ���
** Input parameters:   color: ץȡ������ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   blockNum: ���ı��
**					   r: ץȡ��ת�ĽǶ�
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISGrabBlock(char color, int blockNum, float r)
{
    return gVIS.GrabBlock(color, blockNum, r);
}

/*************************************************************
** Function name:      SmartKit_VISPlaceBlock
** Descriptions:       �������
** Input parameters:   color: ����������ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char SmartKit_VISPlaceBlock(char color)
{
    return gVIS.PlaceBlock(color);
}

