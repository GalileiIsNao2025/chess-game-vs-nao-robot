
#ifndef DobotPixy_H
#define DobotPixy_H
#include "Arduino.h"
#include "SmartKitType.h"

#define BlockMaxNum 10

#ifndef __DEBUG
#define __DEBUG
#endif


class VIS
{

public:

    struct BlockParm			/* 物块结构体参数 */
    {
        int signature;			/* 物块颜色标记 */
        float TargetX;			/* 物块放置目标区域X */
        float targetY;			/* 物块放置目标区域Y */
        float targetZ;			/* 物块放置目标区域Z */
        float targetR;			/* 物块旋转角度R */
        float Height;				/* 物块高度 */
        int placeNum;			/* 物块当前堆垛数量 */
        int checkNum;			/* 物块视觉识别检测到的数量 */
        float coordinate[BlockMaxNum][2];	/* 物块坐标 */
    };
    typedef struct BlockParm BLOCKPARM;
    typedef BLOCKPARM *PBLOCKPARM;

    struct _VISAT			        /* 视觉识别区域坐标 */
    {
        float x;					/* 视觉识别区域X坐标 */
        float y;					/* 视觉识别区域Y坐标 */
        float z;					/* 视觉识别区域Z坐标 */
        float r;					/* 机械臂R坐标系 */
    };
    typedef struct _VISAT VISAT;
    typedef VISAT *PVISAT;

    struct MatrixParm			/* 矩阵变换结构体 */
    {
        float dobot[9];			/* 机械臂变换矩阵 */
        float pixy[9];			/* PIXY变换矩阵 */
        float RT[9];			/* 变换矩阵系数 */
    };
    typedef struct MatrixParm MATRIXPARM;
    typedef MATRIXPARM *PMATRIXPARM;

    void Init(void);
    void SetDobotMatrix(float x1, float y1, 
                        float x2, float y2, 
                        float x3, float y3);
    void SetPixyMatrix(float x1, float y1, float length1, float wide1,
                       float x2, float y2, float length2, float wide2,
                       float x3, float y3, float length3, float wide3); 
    void SetGrapAreaZ(float z);
    float GetGrapAreaZ(void);
    void SetAT(float x, float y, float z, float r);
    char SetColorSignature(char color, char signature);
    char SetBlockTA(int color, float x, float y, float z, float r);
    char GetBlockCheckNum(int color);
    char SetBlockPlaceNum(int color, int placeNum);
    char GetBlockPlaceNum(int color);
    char SetBlockHeight(int color, float height);
    void BlockParmCheckNumClear(void);
    char Run(void);
    int SetGrapBlockMark(int mark);
    int GetGrapBlockMark(void);
    char GrabBlock(int color, int blockNum, float r);
    char PlaceBlock(int color);

private:

    float gGrapAreaZ;			/* 物块抓取区域平台Z轴坐标 */
    int gGrabMark = FALSE;		/* 物块抓取标记,TRUE已经抓取了物块,FALSE,没有抓取物块 */

    char GetColorBlockParmPtr(int color, PBLOCKPARM *ptr);
    void CalcInvMat(float *Mat, float *InvMat);
    void MatMultiMat(float *Mat1, float *Mat2, float *Result);
    void SetBlockTAParm(PBLOCKPARM pBlockParm, float x, float y, float z, float r);
    int FloatEqual(float data1, float data2, float precision);
    int DobotMove(uint8_t Model, float x, float y, float z, float r);
    char DelBlockCoordinate(int color, int num);
    void transForm(float pixyX, float pixyY, float *pDobotX, float *pDobotY);
};


#endif /* DobotPixy_H */





