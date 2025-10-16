/*===========================================================

VoiceRecognition.cpp——�??音识�?库文件�?

上海智位机器人有限公�? 版权所�?
http://www.dfrobot.com.cn

2014-8-11

=============================================================
*/
//#include "utility/PinMap.h"
#include "VoiceRecognition.h"
#include "Arduino.h"
#include "SmartKitType.h"

uint8_t g_Mic;
int RSTB = 49;//RSTB引脚定义
int CS = 53;//RSTB引脚定义
uint8_t MIC_VOL=0x55;//ADC增益初�?��?
uint8_t speech_endpoint=0x10;//�?音�??点�?�测初始�?
uint8_t speech_start_time=0x08;//�?音�??点�?�测开始时间初始�?
uint8_t speech_end_time=0x10;//�?音�??点�?�测结束时间初始�?
uint8_t voice_max_length=0xC3;//最长�??音�?�时间，默�??20�?
uint8_t noise_time=0x02;//忽略上电�?声时�?
//uint8_t ASR_time最长时�?
int readflag=0;
int readnum=0;

VoiceRecognition::VoiceRecognition(){}

int VoiceRecognition::read()//识别结果读取
{
  if(readflag==1)
  { 
    readflag=0;
    return readnum;
  }
  return -1;
}
void update()//�?�?服务函数
{ 
  uint8_t Asr_Count=0;
  if((readReg(0x2b) & 0x10) && readReg(0xb2)==0x21 && readReg(0xbf)==0x35)//如果有�??音识�?�?�?、DSP闲、ASR正常结束
  {
    writeReg(0x29,0) ;///////////关中�?
    writeReg(0x02,0) ;/////////////关FIFO�?�?
    Asr_Count = readReg(0xba);//读中�?辅助信息
    if(Asr_Count>0 && Asr_Count<4) //////如果有识�?结果
    {
      readnum=readReg(0xc5);
      readflag=1;
    } 
    writeReg(0x2b,0);//////清�?�中�?编号
    writeReg(0x1C,0);////////貌似关麦克�?�啊~~为毛
  }
  readReg(0x06);  
  delay(10);
  readReg(0x06);  
  writeReg(0x89, 0x03);  
  delay(5);
  writeReg(0xcf, 0x43);  
  delay(5);
  writeReg(0xcb, 0x02);
  writeReg(0x11, PLL_11);  
  writeReg(0x1e,0x00);
  writeReg(0x19, PLL_ASR_19); 
  writeReg(0x1b, PLL_ASR_1B); 
  writeReg(0x1d, PLL_ASR_1D);
  delay(10);
  writeReg(0xcd, 0x04);
  writeReg(0x17, 0x4c); 
  delay(5);
  writeReg(0xcf, 0x4f);  
  writeReg(0xbd, 0x00);
  writeReg(0x17, 0x48);
  delay(10);
  writeReg(0x3c, 0x80);  
  writeReg(0x3e, 0x07);
  writeReg(0x38, 0xff);  
  writeReg(0x3a, 0x07);
  writeReg(0x40, 0);   
  writeReg(0x42, 8);
  writeReg(0x44, 0); 
  writeReg(0x46, 8); 
  delay(1); 
  writeReg(0x1c, 0x09);////////麦克风�?�置保留
  writeReg(0xbd, 0x20);/////////保留设置
  writeReg(0x08, 0x01);///////////→清�?FIFO_DATA
  delay( 1);
  writeReg(0x08, 0x00);////////////清除指定FIFO后再写入一�?00H
  delay( 1);
  writeReg(0xb2, 0xff);////////�?0xB2写FF
  writeReg(0x37, 0x06);////////开始识�?
  delay( 5 );
  writeReg(0x1c, g_Mic);////////选择麦克�?
  writeReg(0x29, 0x10);////////开同�?�中�?
  writeReg(0xbd, 0x00);/////////�?动为�?音识�?
} 
void cSHigh() {//CS拉高
  digitalWrite(CS, HIGH);
}
void cSLow() {//CS脚拉�?
  digitalWrite(CS, LOW);
}
void writeReg(unsigned char address,unsigned char value)////////写寄存器，参数（寄存器地址，数�?�?
{ 
  cSLow();////拉低CS
  delay(10);
  transfer(0x04);////////////写指�?
  transfer(address);
  transfer(value);
  cSHigh();////拉高CS
 }

unsigned char readReg(unsigned char address)///读寄存器，参数（寄存器地址�?
{ 
  unsigned char result;
  cSLow();////拉低CS
  delay(10);
  transfer(0x05);///////////读指�?
  transfer(address);
  result=transfer(0x00);
  cSHigh();///拉高CS
  return(result);
 }
byte transfer(byte _data) /////////////////SPI数据交换
{
  SPDR = _data;
  while (!(SPSR & _BV(SPIF)));
  return SPDR;
}

/*************************************************************
** Function name:	  VoiceCheck
** Descriptions:	  声音检�?
** Input parameters:  num: 需要�?�测的序号
** Output parameters: no
** Returned value:	  TRUE: 检测到; FALSE: 没有检测到
*************************************************************/

int VoiceRecognition::VoiceCheck(uint8_t num)
{
	static int lastNum = -1;
	int preNum = -1;
	preNum = VoiceRecognition::read();
	if (preNum != -1)
	{
		lastNum = preNum;
	}
	if (lastNum == num)
	{
		lastNum = -1;
		return TRUE;
	}
	return FALSE;
}

void VoiceRecognition::init(uint8_t mic)////////模块�?�?，参数为麦克风选择（MIC/MONO）与丝印对照,在SETUP�?调用
{
  if(mic==MIC)
  {
    g_Mic=MIC;
  }
  else if(mic==MONO)
  {
    g_Mic=MONO;
  }
  pinMode(RSTB,OUTPUT);
  pinMode(CS, OUTPUT);
  cSHigh();

  pinMode(SPI_MISO_PIN, INPUT);
  pinMode(SPI_MOSI_PIN, OUTPUT);
  pinMode(SPI_SCK_PIN, OUTPUT);

  #ifndef SOFTWARE_SPI
    // SS must be in output mode even it is not chip select
    pinMode(LD_CHIP_SELECT_PIN, OUTPUT);
    digitalWrite(LD_CHIP_SELECT_PIN, HIGH); // disable any SPI device using hardware SS 拉高ss
    // Enable SPI, Master, clock rate f_osc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);/////初�?�化SPI寄存�?
    // clear double speed
    SPSR &= ~(1 << SPI2X);//2倍�?
  #endif  // SOFTWARE_SPI 

  SPCR = (SPCR & ~SPI_MODE_MASK) | 0x08;//设置SCK常态电平与取样时间�?0x08为SCK常态为高电平，下降沿有�?
  reset();//LD3320复位操作
  
  #if defined(__AVR_ATmega32U4__)
    attachInterrupt(1,update,LOW);//开�?�? 
  #else
    attachInterrupt(4,update,LOW);//开�?�? 
  #endif
  
  ASR_init();///�?音识�?初�?�化函数 
}

void VoiceRecognition::reset()//LD3320复位操作
{
  digitalWrite(RSTB,HIGH);
  delay(1);
  digitalWrite(RSTB,LOW);
  delay(1);
  digitalWrite(RSTB,HIGH);
  delay(1);
  cSLow();
  delay(1);
  cSHigh();
  delay(1);  
  writeReg(0xb9, 0x00);
}

void VoiceRecognition::ASR_init()////////////初�?�化�?音识�?模式�?
{
    //添加状态标�? 
    readReg(0x06);  
//    writeReg(0x17, 0x35); 
    delay(10);
    readReg(0x06);  
    writeReg(0x89, 0x03);  
    delay(5);
    writeReg(0xcf, 0x43);  
    delay(5);
    writeReg(0xcb, 0x02);
    writeReg(0x11, PLL_11);  
    writeReg(0x1e,0x00);
    writeReg(0x19, PLL_ASR_19); 
    writeReg(0x1b, PLL_ASR_1B); 
    writeReg(0x1d, PLL_ASR_1D);
    delay(10);
    writeReg(0xcd, 0x04);
    writeReg(0x17, 0x4c); 
    delay(5);
//    writeReg(0xb9, 0x00);
    writeReg(0xcf, 0x4f);  
    writeReg(0xbd, 0x00);
    writeReg(0x17, 0x48);
    delay(10);
    writeReg(0x3c, 0x80);  
    writeReg(0x3e, 0x07);
    writeReg(0x38, 0xff);  
    writeReg(0x3a, 0x07);
    writeReg(0x40, 0);   
    writeReg(0x42, 8);
    writeReg(0x44, 0); 
    writeReg(0x46, 8); 
    delay(1);
}
void VoiceRecognition::addCommand(char *pass,int num)
{

  int i;
    writeReg(0xc1, num);//字�?�编�?
    writeReg(0xc3, 0 );//添加时输�?00
    writeReg(0x08, 0x04);//不清�?
  
    delay(1);
    writeReg(0x08, 0x00);//
    delay(1);
  for(i=0;i<=80;i++)
  {
    if (pass[i] == 0)
      break;
    writeReg(0x5, pass[i]);///写入FIFO_EXT
  }
  writeReg(0xb9, i);//写入当前添加字�?�串长度
  writeReg(0xb2, 0xff);//////////B2全写ff 
  writeReg(0x37, 0x04);//添加�?�?
}


unsigned char VoiceRecognition::start()//////开始识�?
{
  writeReg(0x35, MIC_VOL);////adc增益；会影响识别范围即噪�?
  
  writeReg(0xb3, speech_endpoint);//�?音�??点�?�测控�?

  writeReg(0xb4, speech_start_time);//�?音�??点起始时�?
  
  writeReg(0xb5, speech_end_time);//�?音结束时�?

  writeReg(0xb6, voice_max_length);//�?音结束时�?
  
  writeReg(0xb7, noise_time);//�?声时�?
  
  writeReg(0x1c, 0x09);////////麦克风�?�置保留

  writeReg(0xbd, 0x20);/////////保留设置
  writeReg(0x08, 0x01);///////////→清�?FIFO_DATA
  delay( 1);
  writeReg(0x08, 0x00);////////////清除指定FIFO后再写入一�?00H
  delay( 1);
  if(check_b2() == 0)////////读取0xB2寄存器函数�?�果DSP没在闲状态则RETURN 0
  {
    return 0;
  }
  writeReg(0xb2, 0xff);////////�?0xB2写FF

  writeReg(0x37, 0x06);////////开始识�?
  delay( 5 );
  writeReg(0x1c, g_Mic);////////选择麦克�?
  writeReg(0x29, 0x10);////////开同�?�中�?
  writeReg(0xbd, 0x00);/////////�?动为�?音识�?

  return 1;////返回1
}

int check_b2()////////用作检测芯片工作是否�?�常，或者DSP�?否忙，不需用户操作，�?�常/闲返�?1
{ 
  for (int j=0; j<10; j++)
  {
    if (readReg(0xb2) == 0x21)
    {
      return 1;
    }
    delay(10);    
  }
  return 0;
}

void VoiceRecognition::micVol(uint8_t vol)//调整ADC增益，参数（0x00~0xFF,建�??10-60）；
{
  MIC_VOL=vol;
  writeReg(0x35, MIC_VOL);////adc增益；会影响识别范围即噪�?
}
void VoiceRecognition::speechEndpoint(uint8_t speech_endpoint_)//调整�?音�??点�?�测，参数�?0x00~0xFF,建�??10-40）；
{
  speech_endpoint=speech_endpoint_;
  writeReg(0xb3, speech_endpoint);//�?音�??点�?�测控�?
}

void VoiceRecognition::speechStartTime(uint8_t speech_start_time_)//调整�?音�??点起始时间，参数�?0x00~0x30,单位10MS）；
{
  speech_start_time=speech_start_time_;
  writeReg(0xb4, speech_start_time);//�?音�??点起始时�?
}
void VoiceRecognition::speechEndTime(uint8_t speech_end_time_)//调整�?音�??点结束时间（吐字间隔时间），参数�?0x00~0xC3,单位10MS）；
{
  speech_end_time=speech_end_time_;
  writeReg(0xb5, speech_end_time);//�?音结束时�?
}
void VoiceRecognition::voiceMaxLength(uint8_t voice_max_length_)//最长�??音�?�时间，参数�?0x00~0xC3,单位100MS）；
{
  voice_max_length=voice_max_length_;
  writeReg(0xb6, voice_max_length);//�?�?
}
void VoiceRecognition::noiseTime(uint8_t noise_time_)//上电�?声略过，参数�?0x00~0xff,单位20MS）；
{
  noise_time=noise_time_;
  writeReg(0xb7, noise_time);//�?声时�?
}

