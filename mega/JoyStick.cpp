
#include "JoyStick.h"
#include "SmartKitType.h"

/*************************************************************
** Function name:	  JoyStickInit
** Descriptions:	  摇杆初始化
** Output parameters: int joyStickPinX: JoyStick X轴连接的引脚
**                    int joyStickPinY: JoyStick Y轴连接的引脚
**                    int joyStickPinZ; JoyStick Z轴连接的引脚
** Input parameters:  no
** Returned value:	  no
*************************************************************/

int JOYSTICK::Init(int joyStickPinX,
                   int joyStickPinY,
                   int joyStickPinZ)
{
    gJoyStickPinX = joyStickPinX;
    gJoyStickPinY = joyStickPinY;
    gJoyStickPinZ = joyStickPinZ;
    pinMode(gJoyStickPinZ, INPUT);
}


/*************************************************************
** Function name:	  ReadJoyStickValue
** Descriptions:	  读取摇杆数值
** Input parameters:  axis坐标系,可传递AXISX，AXISY
** Output parameters: no
** Returned value:	  摇杆的数值
*************************************************************/

int JOYSTICK::ReadXYValue(int axis)
{
    int value = 0;
    switch (axis)
    {
    case AXISX:
        value = analogRead(gJoyStickPinX);	/* 模拟量读取 */
        break;

    case AXISY:
        value = analogRead(gJoyStickPinY);	/* 模拟量读取 */
        break;

    default:
        return 0;
    }
    return value;
}


/*************************************************************
** Function name:	  CheckJoystickPressState
** Descriptions:	  检测摇杆按压状态
** Input parameters:  no
** Output parameters: no
** Returned value:	  按压状态，UP,DOWN
*************************************************************/

int JOYSTICK::CheckPressState(void)
{
    char down = 0;		        /* 定义按下的状态为 0 */
    if (digitalRead(gJoyStickPinZ) == down)
    {
        delay(10);				/* 按键防抖处理 */
        if (digitalRead(gJoyStickPinZ) == down)
        {
            return DOWN;
        }
    }
    else
    {
        return UP;
    }
}


