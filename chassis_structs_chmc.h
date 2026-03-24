#ifndef CHASSIS_STRUCTS_CHMC_H
#define CHASSIS_STRUCTS_CHMC_H

#include <stdbool.h>
#include <stdint.h>

#pragma pack(2)

/*********************************************************************
 * 定义机箱管理 -------------> CHMC 的串口通信相关结构体
 *********************************************************************/
struct serialChassisSend            /*定义向CHMC端的串口发送查询数据帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  slot;                              //槽位
  unsigned char  frameType;                         //查询指令
  unsigned short backup;                            //备份
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //TxSerialData

struct serialThresholdSend            /*定义向CHMC端串口发送阈值设置数据帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  frameType;                         //报文类型 0x82
  int16_t temperatureLowerNR;                       //温度下限NR 缩放因子*10
  int16_t temperatureLowerCR;                       //温度下限CR 缩放因子*10
  int16_t temperatureLowerNC;                       //温度下限NC 缩放因子*10
  int16_t temperatureUpperNR;                       //温度上限NC 缩放因子*10
  int16_t temperatureUpperCR;                       //温度上限CR 缩放因子*10
  int16_t temperatureUpperNC;                       //温度上限NT 缩放因子*10
  uint16_t voltage1RefValue;                        //电压1基准值
  uint16_t voltage1MinThreshold;                    //电压1下限阈值  缩放因子*100
  uint16_t voltage1MaxThreshold;                    //电压1上限阈值  缩放因子*100
  uint16_t voltage2RefValue;                        //电压2基准值    缩放因子*100
  uint16_t voltage2MinThreshold;                    //电压2下限阈值  缩放因子*100
  uint16_t voltage2MaxThreshold;                    //电压2上限阈值  缩放因子*100
  uint16_t voltage3RefValue;                        //电压3基准值    缩放因子*100
  uint16_t voltage3MinThreshold;                    //电压3下限阈值  缩放因子*100
  uint16_t voltage3MaxThreshold;                    //电压3上限阈值  缩放因子*100
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //TxSerialThreshold

struct serialSingleBoardSend             /*定义发往CHMC串口的单板信息数据帧*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  frameType;                         //报文类型 0x83
  unsigned char payloadHardwareVersion[8];          //payload硬件版本
  unsigned char payloadSofewareVersion[8];          //payload软件件版本
  unsigned short boardID;                           //板卡厂家ID
  unsigned short boardName;                         //板卡名称 2
  unsigned char boardNumber[8];                     //板卡编号
  unsigned char boardState;                         //板卡状态 1
  unsigned char factoryNumber[8];                   //出厂编号
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //TxSerialSingleBoard


/*********************************************************************
 * 定义CHMC -------------> 机箱管理的串口通信相关结构体
 *********************************************************************/
struct serialBackRecv            /*定义从CHMC端的串口接收反馈指令数据帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  frameType;                         //报文类型
  unsigned short backup;                            //备份
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //RxSerialBack

struct serialChassisRecv             /*定义从CHMC端接收的串口单板维护数据帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  frameType;                         //报文类型 0x0E
  unsigned char chmcSingleData[867];                //接收chmc数据 51*17
  unsigned short backup1;                           //备份1
  unsigned short backup2;                           //备份2
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //RxSerialData

struct serialHealthManagement                 /*定义健康管理属性*/
{
   unsigned char slot;                     //槽位
   unsigned short boardName;               //板卡名称
   unsigned char temperature;              //温度
   unsigned char voltage;                  //电压
   unsigned char current;                  //电流
};

struct serialHealthRecv             /*定义从chmc端接收的串口健康管理数据帧格式*/
{
  unsigned short frameHead;
  unsigned short frameCnt;
  unsigned char  frameType;
  struct serialHealthManagement serialHealth[17];
  unsigned short backup1;                           //备份1
  unsigned short backup2;                           //备份2
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //RxHealthSerialData

struct serialFanData              /*定义从chmc端的串口接收的风扇管理数据帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char  frameType;                         //报文类型
  unsigned short fanSpeed;                          //fan转速
  unsigned short backup1;                           //备份1
  unsigned short backup2;                           //备份2
  unsigned short frameSum;                          //累加和校验
  unsigned short frameEnd;                          //帧尾
};                                                  //RxFanSerialData

typedef enum { /*定义日志级别-枚举类型*/
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

#pragma pack()

#endif /* CHASSIS_STRUCTS_H */
