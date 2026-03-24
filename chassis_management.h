#ifndef CHASSIS_MANAGEMENT_H
#define CHASSIS_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <endian.h>
#include <stdint.h>

#include "chassis_structs_web.h"
#include "chassis_structs_chmc.h"
/*▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ▓                          程序初始参量配置信息                              ▓
  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓*/
#define SELF_PORT              3408                                   //定义本机网络通讯端口号
//#define WEB_ADDR               "188.188.32.12"                        //定义网管程序地址配置
#define WEB_ADDR               "10.239.15.70"                         //定义网管程序地址配置
#define WEB_PORT               3407                                   //定义网管程序端口号
#define BUF_SIZE               1024                                   //定义网络接收数据长度
#define LINK_BAD			   0				                      //定义建链失败
#define LINK_OK				   1				                      //定义建链成功
#define SEND_EN				   1				                      //定义允许发送使能
#define SEND_OFF			   0				                      //定义允许发送禁止
#define SERIAL_MAX_LENTH       920                                    //定义串口接收单板维护数据最大长度
#define SERIAL_HEALTH_SIZE     202                                    //定义串口接收健康信息数据的最大长度
#define SAVA_DATA_LENG_HEALTH  208                                    //定义存储健康管理数据最大长度
#define SAVA_DATA_LENG_SINGLE  924                                    //定义存储单板信息数据的最大长度字节数
#define SAVA_DATA_LENG_FAN     17                                      //定义存储风扇信息数据的最大长度字节数
#define TX_FIFO_LENG		 2048*4			                          //定义缓存发送语音数据FIFO的最大长度
#define	RX_FIFO_LENG		 2048*4			                          //定义接收语音数据FIFO的最大长度

/************** UDP通信结构体 ****************/
extern struct receiveAndBackSingleBoardQuery receive_single_query;
extern struct backQueryorComman back_query;
extern struct backSingleBoardQuery back_single_board;
extern struct receiveHealthManagement receive_health_management;
extern struct receiveHealthManagement back_threshold_data;
extern struct backHealthQuery back_health_query;
extern struct FanState receive_fan_query;
extern struct BackFanState back_fan_state;

/************** 串口通信结构体 ****************/
extern struct serialChassisSend TxSerialData;
extern struct serialThresholdSend TxSerialThreshold;
extern struct serialSingleBoardSend TxSerialSingleBoard;
extern struct serialBackRecv RxSerialBack;
extern struct serialChassisRecv RxSerialData;
extern struct serialHealthRecv RxHealthSerialData;
extern struct serialFanData RxFanSerialData;

extern unsigned char backCommand;                               //定义板卡返回web设置指令
extern unsigned char linkCnt;                                   //定义建链回执发送帧计数
extern unsigned short singleQueryCnt;                           //定义单板维护查询回执帧计数
extern unsigned short healthQueryCnt;                           //定义健康管理查询回执帧计数
extern unsigned short healthSetCnt;                             //定义健康管理阈值设置回执帧计数
extern unsigned short fanQueryCnt;                              //定义风扇管理查询回执帧计数
extern unsigned short txdataCnt;                                //定义串口发送查询数据帧计数
extern unsigned short txThresholdCnt;                           //定义串口发送阈值返回数据帧计数
extern unsigned short txSingleBoardCnt;                          //定义串口发送单板信息数据帧计数
extern unsigned char chassisQuery;                              //定义机箱管理查询指令缓存变量
extern unsigned char querySlot;                                 //定义机箱管理查询槽位缓存变量
extern unsigned char queryFlag;                                 //定义查询标识
extern unsigned char singleBoardBuffer[SAVA_DATA_LENG_SINGLE];  //定义存储单板维护信息缓存
extern unsigned char healthManageBuffer[SAVA_DATA_LENG_HEALTH]; //定义存储健康管理信息缓存
extern unsigned char fanManageBuffer[SAVA_DATA_LENG_FAN];       //定义存储风扇管理信息缓存

extern void Seria_initialization(void);
extern int serialfd;
/*********************************定义UDP套接字*********************************/
extern int sockfd;                                              
extern struct sockaddr_in server_addr;
extern struct sockaddr_in client_addr;
extern struct sockaddr_in dest_addr;
/*****************定义线程ID********************/
extern pthread_t receive_tid;
extern pthread_t send_tid;
extern pthread_t serialRecvId;
extern pthread_t serialSendId;
/*****************************定义串口发送相关变量****************************/
extern unsigned char tx_serial_buffer[11];
extern bool hasSentData;
extern bool hasSendUdpData;
extern pthread_mutex_t serial_mutex;
/*****************************定义读写文件数据相关变量****************************/
extern unsigned char writeBuffer[47];
extern unsigned char readBuffer[47];


void variableInit(void);
int networkInit(void);
void log_output(LogLevel level, const char* message);
unsigned short CalcChecksum(unsigned char *data, int len);
int writeDataToFile(unsigned char data[], size_t dataSize);
int readDataFile(unsigned char data[], size_t dataSize);
void temperatureAndVoltageCompare(void);
void makeSendData(unsigned char makeclasses);
void recvUdpSingleHealthFan(void);
void recvUdpHealth(void);

#endif
