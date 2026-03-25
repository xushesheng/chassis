#ifndef CHASSIS_STRUCTS_WEB_H
#define CHASSIS_STRUCTS_WEB_H

#include <stdbool.h>
#include <stdint.h>

#pragma pack(1)

/*********************************************************************
 * 机箱管理软件 -------------> 网管软件 结构体定义
 *********************************************************************/

struct backQueryorComman                 /*指令反馈报文*/
{
  unsigned short frameHead;              //帧头
  unsigned short frameCnt;               //帧计数
  unsigned char  frameType;              //报文类型
  unsigned short backup;                 //备份
  unsigned short frameEnd;               //帧尾
};               						 //back_query

struct singleBoardAttribute                 /*定义机箱管理发往Web网管的单板属性报文(每个槽位49字节)*/
{
   unsigned char slot;                      //槽位 1
   unsigned char payloadHardwareVersion[8]; //payload硬件版本
   unsigned char payloadSofewareVersion[8]; //payload软件件版本
   unsigned char ipmcVersion;               //ipmc版本 1
   unsigned short boardType;                //板卡代号 2
   unsigned short boardName;                //板卡名称 2
   unsigned char boardNumber[8];            //板卡编号
   unsigned char boardState;                //板卡状态 1
   unsigned char factoryNumber[8];          //出厂编号
   unsigned char factoryDte[10];            //出厂日期
};

struct backSingleBoardQuery                         /*定义机箱管理发往web网管单板维护查询反馈帧格式*/
{
  unsigned short frameHead;                         //帧头
  unsigned short frameCnt;                          //帧计数
  unsigned char frameType;                          //报文类型
  struct singleBoardAttribute boardAttribute[17];   //所有板卡属性17*49字节
  unsigned short backup1;                           //备份1
  unsigned short backup2;                           //备份2
  unsigned short frameEnd;                          //帧尾
};               						            //back_single_board

struct healthManagement                    /*定义温度电压信息健康管理属性*/
{
   unsigned char slot;                     //槽位
   unsigned short boardName;               //板卡名称
   char temperature;                       //温度
   char voltage;                           //电压
   char current;                           //电流
   unsigned char temperatureWarning;       //温度警告
   unsigned char voltageWarning;           //电压警告
}; 

struct backHealthQuery             /*定义机箱管理返回WEB网管健康管理查询指令帧格式*/
{
  unsigned short frameHead;                   //帧头
  unsigned short frameCnt;                    //帧计数
  unsigned char frameType;                    //报文类型
  struct healthManagement healthAttribute[17];//所有板卡健康属性17*8=136
  unsigned short backup1;                     //备份1
  unsigned short backup2;                     //备份2
  unsigned short frameEnd;                    //帧尾
};            						          //back_health_query

struct BackFanState            /*定义机箱管理返回Web网管风扇管理查询指令帧格式*/
{
  unsigned short frameHead;              //帧头
  unsigned short frameCnt;               //帧计数
  unsigned char frameType;               //报文类型
  unsigned char slot;                    //固定槽位A2
  unsigned char fanSpeed[2];             //风扇转速
  unsigned short backup1;                //备份1
  unsigned short backup2;                //备份2
  unsigned short frameEnd;               //帧尾
};         						         //back_fan_state


/*********************************************************************
 * 网管软件 -------------> 机箱管理软件 结构体定义
 *********************************************************************/
struct receiveQuery       /*定义接收web设置数据帧格式和查询指令帧格式*/
{
  unsigned short frameHead;              //帧头
  unsigned short frameCnt;               //帧计数
  unsigned char slot;                    //槽位，执行查询指令时，Slots=0
  unsigned char frameType;               //查询指令 单板是03h 风扇是10h 温度电压是0Eh
  unsigned short backup;                 //备份
  unsigned short frameEnd;               //帧尾
};                                       //receive_query


struct receiveThreshold          /*定义接收Web阈值设置指令帧格式*/
{
  unsigned short frameHead;             //帧头
  unsigned short frameCnt;              //帧计数
  unsigned char frameType;              //报文类型
  int temperatureLowerNR;               //温度下限NR
  int temperatureLowerCR;               //温度下限CR
  int temperatureLowerNC;               //温度下限NC
  int temperatureUpperNR;               //温度上限NC
  int temperatureUpperCR;               //温度上限CR
  int temperatureUpperNC;               //温度上限NT
  float voltageMin;                     //电压最小阈值
  float voltageMax;                     //电压最大阈值
  unsigned char backup[8];              //备份
  unsigned short frameEnd;              //帧尾
};                                      //receive_threshold_data

#pragma pack()

#endif /* CHASSIS_STRUCTS_H */
