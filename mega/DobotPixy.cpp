/********************** Copyright(c)***************************
** Shenzhen Yuejiang Technology Co., Ltd.
**
** http:// www.dobot.cc
**
**------File Info----------------------------------------------
** File name:            VIS.cpp
** Latest modified date: 2019-05-15
** Latest version:       V1.0.0
** Description:          SmartKit VIS API
**
**------------------------------------------------------------
** Created by:
** Created date:         2019-05-15
** Version:              V1.0.0
** Descriptions:         SmartKit VIS API
**
**------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*************************************************************/
#include "DobotPixy.h"
#include "Magician.h"
#include "Pixy2I2C.h"
#include "SmartKitType.h"


VIS::BLOCKPARM gBlockParmRed;	  /* ��ɫ�������ṹ�� */
VIS::BLOCKPARM gBlockParmBlue;  /* ��ɫ�������ṹ�� */
VIS::BLOCKPARM gBlockParmYellow;/* ��ɫ�������ṹ�� */
VIS::BLOCKPARM gBlockParmGreen; /* ��ɫ�������ṹ�� */

VIS::VISAT	gVISAT;  /* ��е���Ӿ�ʶ��Ŀ��������� */

VIS::MATRIXPARM gMatrixParm; /* ����任���� */

Pixy2I2C pixy;  /* pixy object */


void VIS::CalcInvMat(float *Mat, float *InvMat)
{
    int i = 0;
    double Det = 0.0;
    Det = Mat[0] * (Mat[4] * Mat[8] - Mat[5] * Mat[7]) - Mat[3] * (Mat[1] * Mat[8] - Mat[2] * Mat[7]) + Mat[6] * (Mat[1] * Mat[5] - Mat[2] * Mat[4]);
    InvMat[0] = Mat[4] * Mat[8] - Mat[5] * Mat[7];
    InvMat[1] = Mat[2] * Mat[7] - Mat[1] * Mat[8];
    InvMat[2] = Mat[1] * Mat[5] - Mat[2] * Mat[4];
    InvMat[3] = Mat[5] * Mat[6] - Mat[3] * Mat[8];
    InvMat[4] = Mat[0] * Mat[8] - Mat[2] * Mat[6];
    InvMat[5] = Mat[3] * Mat[2] - Mat[0] * Mat[5];
    InvMat[6] = Mat[3] * Mat[7] - Mat[4] * Mat[6];
    InvMat[7] = Mat[1] * Mat[6] - Mat[7] * Mat[0];
    InvMat[8] = Mat[0] * Mat[4] - Mat[3] * Mat[1];
    for (i = 0; i < 9; i++)
    {
        InvMat[i] = InvMat[i] / Det;
    }
}

//matrix multiplication��A^-1*A*RT = A^-1*B => RT = A^-1*B
void VIS::MatMultiMat(float *Mat1, float *Mat2, float *Result)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int Value = 0;
    for (i = 0; i < 9; i++)
    {
        Result[i] = 0;
    }
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < 3; k++)
            {
                Result[i * 3 + j] += Mat1[i * 3 + k] * Mat2[k * 3 + j];
            }
        }
    }
}

/*************************************************************
** Function name:      GetColorBlockParmPtr
** Descriptions:       ��ȡ��ͬ��ɫ��BlockParm��ָ��
** Input parameters:   no
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::GetColorBlockParmPtr(int color, 
                                     PBLOCKPARM *ptr)
{
    switch (color)
    {
    case RED:
        *ptr = &gBlockParmRed;
        break;

    case BLUE:
        *ptr = &gBlockParmBlue;
        break;

    case YELLOW:
        *ptr = &gBlockParmYellow;
        break;

    case GREEN:
        *ptr = &gBlockParmGreen;
        break;

    default:
        *ptr = NULL;
        return FALSE;
    }
    return TRUE;
}

/*************************************************************
** Function name:      Init
** Descriptions:       VIS��ʼ����
**					   Ĭ������̧���߶�20��
**					   ��ʼ��֮ǰ��Ҫ�����ɫ�궨��λ�ñ궨
** Input parameters:   no
** Output parameters:  no
** Returned value:     no
*************************************************************/

void VIS::Init(void)
{
    Serial.begin(115200);
    Dobot_Init();
    Serial.print("Dobot Init\n");
    pixy.init();
    pixy.setLamp(1, 1);
    Serial.print("Pixy Init\n");
    Dobot_SetPTPJumpParams(30);              
    Dobot_SetPTPCommonParams(50, 50);
    Dobot_SetEndEffectorSuctionCup(false);	 /* �ɿ����� */
    float inv_pixy[9] =                      
    {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0
    };
    CalcInvMat(gMatrixParm.pixy, inv_pixy);
    MatMultiMat(gMatrixParm.dobot, inv_pixy, gMatrixParm.RT);
    Serial.print("Finally\n");
}

/*************************************************************
** Function name:      SetDobotMatrix
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

void VIS::SetDobotMatrix(float x1, float y1, 
                         float x2, float y2, 
                         float x3, float y3)
{
    gMatrixParm.dobot[3] = x1;
    gMatrixParm.dobot[4] = x2;
    gMatrixParm.dobot[5] = x3;
    gMatrixParm.dobot[0] = y1;
    gMatrixParm.dobot[1] = y2;
    gMatrixParm.dobot[2] = y3;
    gMatrixParm.dobot[6] = 1;
    gMatrixParm.dobot[7] = 1;
    gMatrixParm.dobot[8] = 1;
}

/*************************************************************
** Function name:      SetPixyMatrix
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

void VIS::SetPixyMatrix(
    float x1, float y1, float length1, float wide1,
    float x2, float y2, float length2, float wide2,
    float x3, float y3, float length3, float wide3)
{
    gMatrixParm.pixy[0] = x1 + length1 / 2;
    gMatrixParm.pixy[1] = x2 + length2 / 2;
    gMatrixParm.pixy[2] = x3 + length3 / 2;
    gMatrixParm.pixy[3] = y1 + wide1 / 2;
    gMatrixParm.pixy[4] = y2 + wide2 / 2;
    gMatrixParm.pixy[5] = y3 + wide3 / 2;
    gMatrixParm.pixy[6] = 1;
    gMatrixParm.pixy[7] = 1;
    gMatrixParm.pixy[8] = 1;
}


/*************************************************************
** Function name:      SetGrapAreaZ
** Descriptions:       ����ץȡ����z������
** Input parameters:   z: ��������߶�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void VIS::SetGrapAreaZ(float z)
{
    gGrapAreaZ = z;
}

/*************************************************************
** Function name:      GetPlacetAreaZ
** Descriptions:       ��ȡץȡ����z������
** Input parameters:   no
** Output parameters:  no
** Returned value:     ץȡ����߶�
*************************************************************/

float VIS::GetGrapAreaZ(void)
{
    return gGrapAreaZ;
}

/*************************************************************
** Function name:      SetDOBOTPIXYAT
** Descriptions:       ���û�е���Ӿ�ʶ����������
** Input parameters:   x:
**					   y:
**					   z:
**					   r: ��ת�Ƕ�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void VIS::SetAT(float x, float y, float z, float r)
{
    gVISAT.x = x;
    gVISAT.y = y;
    gVISAT.z = z;
    gVISAT.r = r;
}

/*************************************************************
** Function name:      SetColorSignature
** Descriptions:       ���������ɫ��ʶ
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   signature: ����ʶ
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::SetColorSignature(char color, char signature)
{
    PBLOCKPARM ptr;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        ptr->signature = signature;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************
** Function name:      SetBlockTA
** Descriptions:       ���ø���ɫ���Ŀ������
** Input parameters:   pBlockParm: ��Ҫ��ֵ�Ĳ����ṹ��ָ��
**					   x: ����x����
**					   y: ����y����
**					   z: ����z����
**					   r: ������ת�Ƕ�
** Output parameters:  no
** Returned value:     no
*************************************************************/

void VIS::SetBlockTAParm(PBLOCKPARM pBlockParm, 
                               float x, 
                               float y, 
                               float z, 
                               float r)
{
    pBlockParm->TargetX = x;
    pBlockParm->targetY = y;
    pBlockParm->targetZ = z;
    pBlockParm->targetR = r;
}

/*************************************************************
** Function name:      SetBlockTA
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

char VIS::SetBlockTA(int color, 
                     float x, 
                     float y, 
                     float z, 
                     float r)
{
    PBLOCKPARM ptr;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        SetBlockTAParm(ptr, x, y, z, r);
        return	TRUE;
    }
    return FALSE;
}

/*************************************************************
** Function name:      GetBlockCheckNum
** Descriptions:       ��ȡ�Ӿ���⵽�Ĳ�ͬ��ɫ���������
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     �������
*************************************************************/

char VIS::GetBlockCheckNum(int color)
{
    PBLOCKPARM ptr;
    int checkNum = 0;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        checkNum = ptr->checkNum;
    }
    return	checkNum;
}

/*************************************************************
** Function name:      SetBlockPlaceNum
** Descriptions:       ��������������
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   placeNum: ���õ�����
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::SetBlockPlaceNum(int color, int placeNum)
{
    PBLOCKPARM ptr;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        ptr->placeNum = placeNum;
        return TRUE;
    }
    return	FALSE;
}

/*************************************************************
** Function name:      GetBlockPlaceNum
** Descriptions:       ��ȡ�����õ�����
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     �������
*************************************************************/

char VIS::GetBlockPlaceNum(int color)
{
    PBLOCKPARM ptr;
    int placeNum = 0;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        placeNum = ptr->placeNum;
    }
    return	placeNum;
}

/*************************************************************
** Function name:      SetBlockHeight
** Descriptions:       �������߶�
** Input parameters:   color: �����ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   height: ���߶�
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::SetBlockHeight(int color, float height)
{
    PBLOCKPARM ptr;
    GetColorBlockParmPtr(color, &ptr);
    if (ptr != NULL)
    {
        ptr->Height = height;
        return TRUE;
    }
    return	FALSE;
}

/*************************************************************
** Function name:      BlockParmPlaceNumClear
** Descriptions:       ����ĸ���ɫpixy��⵽���������
** Input parameters:   no
** Output parameters:  no
** Returned value:     no
*************************************************************/

void VIS::BlockParmCheckNumClear(void)
{
    gBlockParmRed.checkNum = 0;
    gBlockParmBlue.checkNum = 0;
    gBlockParmYellow.checkNum = 0;
    gBlockParmGreen.checkNum = 0;
}

/*************************************************************
** Function name:      transForm
** Descriptions:       ת��pixy�����dobot����
** Input parameters:   pixyX: ����ͷ����
**					   pixyY:
** Output parameters:  *pDobotX: ��е������
**					   *pDobotY
** Returned value:     no
*************************************************************/

void VIS::transForm(float pixyX,    float pixyY, 
                          float *pDobotX, float *pDobotY)
{
    *pDobotY = (gMatrixParm.RT[0] * pixyX) 
             + (gMatrixParm.RT[1] * pixyY) 
             + (gMatrixParm.RT[2] * 1);
    *pDobotX = (gMatrixParm.RT[3] * pixyX) 
             + (gMatrixParm.RT[4] * pixyY) 
             + (gMatrixParm.RT[5] * 1);

#ifdef __DEBUG
    Serial.print("pixy  x,y:");
    Serial.print(pixyX);
    Serial.print(" , ");
    Serial.println(pixyY);

    Serial.print("coordinate  x,y:");
    Serial.print(*pDobotX);
    Serial.print(" , ");
    Serial.println(*pDobotY);
#endif /* __DEBUG */

}

/*************************************************************
** Function name:      FloatEqual
** Descriptions:       ��鸡�����Ƿ����
** Input parameters:   data1: ����1
**					   data2: ����2
**					   precision: ��ȷ��,����������Ϊ���
** Output parameters:  no
** Returned value:     ����TRUE, FALSE���ƶ��ɹ�����ʧ��
*************************************************************/

int VIS::FloatEqual(float data1, 
                          float data2, 
                          float precision)
{
    if (fabs(data1 - data2) <= precision)
    {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************
** Function name:      DobotMove
** Descriptions:       ��е���ƶ�������ƶ��Ƿ�ɹ�
** Input parameters:   Model���ƶ��ķ�ʽ���鿴Dobot_SetPTPCmd�ĵ�����
**					   x:
**					   y:
**					   z:
**					   r: ��ת�Ƕ�
** Output parameters:  no
** Returned value:     ����TRUE, FALSE���ƶ��ɹ�����ʧ��
*************************************************************/

int VIS::DobotMove(uint8_t Model, 
                         float x, 
                         float y, 
                         float z, 
                         float r)
{
    Pose p;
    Dobot_SetPTPCmd(Model, x, y, z, r);     // The position of a target object
    delay(500);
    GetPose(&p);
#ifdef __DEBUG
    Serial.print("move: ");
    Serial.print(x);
    Serial.print(" , ");
    Serial.println(y);
    Serial.print(" , ");
    Serial.println(z);

    Serial.print("pose: ");
    Serial.print(p.x);
    Serial.print(" , ");
    Serial.println(p.y);
    Serial.print(" , ");
    Serial.println(p.z);
#endif /* __DEBUG */
    if (FloatEqual(p.x, x, 0.01) == TRUE && FloatEqual(p.y, y, 0.01) == TRUE && FloatEqual(p.z, z, 0.01) == TRUE)
    {
        return TRUE;
    }
    return FALSE;
    delay(500);
}

/*************************************************************
** Function name:      DOBOTPIXYRun
** Descriptions:       �Ӿ�ʶ��
** Input parameters:   no
** Output parameters:  no
** Returned value:     TRUE: �������;
**					   FALSE: ԭ�������¼��ֿ���
**							  1,û�м�⵽��飬
**							  2,������ɫ�����������
*************************************************************/

char VIS::Run(void)
{
    int signature = 0;
    Pose p;
    PBLOCKPARM ptr = NULL;
    GetPose(&p);
    delay(500);
    if (FloatEqual(p.x, gVISAT.x, 0.01) == FALSE || FloatEqual(p.y, gVISAT.y, 0.01) == FALSE || FloatEqual(p.z, gVISAT.z, 0.01) == FALSE)		/* �жϵ�ǰ���� */
    {
        while (DobotMove(JUMP_XYZ, gVISAT.x, gVISAT.y, gVISAT.z, gVISAT.r) != TRUE);       /* ������ʼλ�� */
    }
    Serial.print("Starting...\n");
    if (pixy.ccc.getBlocks() != 0)
    {
        delay(1500);
        pixy.ccc.getBlocks();																			 /* ��ȡ��� */
    }
    Serial.print("Number of blocks: ");
    Serial.println(pixy.ccc.numBlocks);
    delay(200);
    if (pixy.ccc.numBlocks == 0)																	 /* �ж�û����� */
    {
        return FALSE;
    }
    else
    {
        BlockParmCheckNumClear();																	 /* ������������� */
        for (int cir = 0; cir < pixy.ccc.numBlocks; cir++)
        {

            signature = pixy.ccc.blocks[cir].m_signature;
            if (signature == gBlockParmRed.signature)					 /* �ж�������Ƿ�һ�£������ж���������Ƿ񳬹������������ */
            {
                GetColorBlockParmPtr(RED, &ptr);
            }

            else if (signature == gBlockParmBlue.signature)
            {
                GetColorBlockParmPtr(BLUE, &ptr);
            }

            else if (signature == gBlockParmYellow.signature)
            {
                GetColorBlockParmPtr(YELLOW, &ptr);
            }

            else if (signature == gBlockParmGreen.signature)
            {
                GetColorBlockParmPtr(GREEN, &ptr);
            }

            if (ptr != NULL)
            {
                if (ptr->checkNum < BlockMaxNum)						/* �ж��Ƿ񳬳�������� */
                {
                    transForm(pixy.ccc.blocks[cir].m_x,
                        pixy.ccc.blocks[cir].m_y,
                        &ptr->coordinate[ptr->checkNum][0],
                        &ptr->coordinate[ptr->checkNum][1]);
                    ptr->checkNum++;
                }

            }
            else
            {
                return FALSE;											/* ָ��Ϊ�գ����ش�����Ϣ */
            }
        }
    }
    if (gBlockParmRed.checkNum == 0 && gBlockParmBlue.checkNum == 0 && gBlockParmYellow.checkNum == 0 && gBlockParmGreen.checkNum)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*************************************************************
** Function name:      DelBlockCoordinate
** Descriptions:       ɾ��ָ����ɫָ��λ���������
** Input parameters:   no
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::DelBlockCoordinate(int color, int num)
{
    PBLOCKPARM ptr = NULL;
    GetColorBlockParmPtr(color, &ptr);							/* ��ȡ��ɫָ�� */
    if (ptr != NULL && num != 0)								/* �ж�ָ���Ƿ�Ϊ�� */
    {
        if (num <= ptr->checkNum && ptr->checkNum != 0)
        {
            for (int i = num; i < ptr->checkNum; i++)
            {
                ptr->coordinate[i - 1][0] = ptr->coordinate[i][0];
                ptr->coordinate[i - 1][1] = ptr->coordinate[i][1];
#ifdef __DEBUG
                Serial.print("moveX:");
                Serial.print(ptr->coordinate[i - 1][0]);
                Serial.print("     moveY:");
                Serial.println(ptr->coordinate[i - 1][1]);
                Serial.print(ptr->checkNum);
                Serial.print(" , ");
                Serial.println(i);
#endif /* __DEBUG */
            }
            ptr->checkNum--;											/* ���������1 */
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************
** Function name:      SetGrapBlockMark
** Descriptions:       �������ץȡ���
** Input parameters:   mark: ���ץȡ���
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

int VIS::SetGrapBlockMark(int mark)
{
    if (mark == TRUE || mark == FALSE)
    {
        gGrabMark = mark;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/*************************************************************
** Function name:      GetGrapBlockMark
** Descriptions:       ��ȡ���ץȡ���
** Input parameters:   no
** Output parameters:  no
** Returned value:     ���ץȡ��ǣ�TRUE�Ѿ�ץȡ��FALSE��δץȡ
*************************************************************/

int VIS::GetGrapBlockMark(void)
{
    return gGrabMark;
}

/*************************************************************
** Function name:      GrabBlock
** Descriptions:       ץȡ���
** Input parameters:   color: ץȡ������ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
**					   blockNum: ���ı��
**					   r: ץȡ��ת�ĽǶ�
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::GrabBlock(int color, int blockNum, float r)
{
    PBLOCKPARM ptr = NULL;
    GetColorBlockParmPtr(color, &ptr);					  /* ��ȡ��ɫָ�� */
    if (ptr != NULL && GetGrapBlockMark() == FALSE && blockNum != 0)		  /* �жϿ�ָ���Լ����ץȡ��� */
    {
        if (ptr->checkNum >= blockNum)					  /* �жϵ�ǰ�Ƿ��������� */
        {
#ifdef __DEBUG
            Serial.print("coordinateX:");
            Serial.print(ptr->coordinate[blockNum - 1][0]);
            Serial.print("     coordinateY:");
            Serial.println(ptr->coordinate[blockNum - 1][1]);
#endif /* __DEBUG */
            while (DobotMove(JUMP_XYZ,								/* �ƶ���е�� */
                ptr->coordinate[blockNum - 1][0],
                ptr->coordinate[blockNum - 1][1],
                gGrapAreaZ + ptr->Height,
                r)
                != TRUE);
            Dobot_SetEndEffectorSuctionCup(true);
            DelBlockCoordinate(color, blockNum);		  /* ɾ��������� */
            SetGrapBlockMark(TRUE);						  /* ���ץȡ��� */
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

/*************************************************************
** Function name:      PlaceBlock
** Descriptions:       �������
** Input parameters:   color: ����������ɫ��
**                            �ɴ���ֵ:RED,BLUE,YELLOW,GREEN
** Output parameters:  no
** Returned value:     TRUE: �ɹ�; FALSE: ʧ��
*************************************************************/

char VIS::PlaceBlock(int color)
{
    PBLOCKPARM ptr = NULL;
    GetColorBlockParmPtr(color, &ptr);		/* ��ȡ��ɫָ�� */
    if (ptr != NULL && GetGrapBlockMark() == TRUE)	/* ���δץȡ��ʱ�򱨴� */
    {
        while (DobotMove(JUMP_XYZ,				/* �ƶ������������� */
            ptr->TargetX,
            ptr->targetY,
            ptr->targetZ + ptr->placeNum*ptr->Height + ptr->Height,
            ptr->targetR)
            != TRUE);
        Dobot_SetEndEffectorSuctionCup(false);	/* �ɿ����� */
        ptr->placeNum++;
        SetGrapBlockMark(FALSE);
        return TRUE;
    }
    return FALSE;
}

