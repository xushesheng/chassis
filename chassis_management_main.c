#include "chassis_management.h"

/************** UDP通信结构体 ****************/
struct receiveSingleBoardQuery receive_single_query;
struct backQueryorComman back_query;
struct backSingleBoardQuery back_single_board;
struct receiveHealthManagement receive_health_management, back_threshold_data;
struct backHealthQuery back_health_query;
struct FanState receive_fan_query;
struct BackFanState back_fan_state;

/************** 串口通信结构体 ****************/
struct serialChassisSend TxSerialData;
struct serialThresholdSend TxSerialThreshold;
struct serialSingleBoardSend TxSerialSingleBoard;
struct serialBackRecv RxSerialBack;
struct serialChassisRecv RxSerialData;
struct serialHealthRecv RxHealthSerialData;
struct serialFanData RxFanSerialData;

unsigned char backCommand = 0;                            //定义板卡返回web设置指令
unsigned char linkCnt = 0;                                //定义建链回执发送帧计数
unsigned short singleQueryCnt = 0;                        //定义单板维护查询回执帧计数
unsigned short healthQueryCnt = 0;                        //定义健康管理查询回执帧计数
unsigned short healthSetCnt = 0;                          //定义健康管理阈值设置回执帧计数
unsigned short fanQueryCnt = 0;                           //定义风扇管理查询回执帧计数
unsigned short txdataCnt = 0;                             //定义串口发送查询数据帧计数
unsigned short txThresholdCnt = 0;                        //定义串口发送阈值返回数据帧计数
unsigned short txSingleBoardCnt = 0;                      //定义串口发送单板信息数据帧计数
unsigned char chassisQuery = 0;                           //定义机箱管理查询指令缓存变量
unsigned char querySlot = 0;                              //定义机箱管理查询槽位缓存变量
unsigned char queryFlag = 0;                              //定义查询标识
unsigned char singleBoardBuffer[SAVA_DATA_LENG_SINGLE];   //定义存储单板维护信息缓存
unsigned char healthManageBuffer[SAVA_DATA_LENG_HEALTH];  //定义存储健康管理信息缓存
unsigned char fanManageBuffer[SAVA_DATA_LENG_FAN];        //定义存储风扇管理信息缓存

/*********************************定义UDP套接字*********************************/
int sockfd;
struct sockaddr_in server_addr, client_addr;
struct sockaddr_in dest_addr;
pthread_t receive_tid, send_tid,serialRecvId,serialSendId;//定义线程ID
/*****************************定义串口发送相关变量****************************/
unsigned char tx_serial_buffer[12] = {0};//串口发送数据buffer
bool hasSentData = false; // 定义usart布尔变量用于记录usart是否已经发送过数据
bool hasSendUdpData = false;//定义udp发送布尔变量用于记录udp是否已经发送过数据
pthread_mutex_t serial_mutex = PTHREAD_MUTEX_INITIALIZER;//互斥锁（mutex）
/*****************************定义读写文件数据相关变量****************************/
unsigned char writeBuffer[47] = {0};//定义写入数据buffer
unsigned char readBuffer[47] = {0};//定义读取数据buffer

void variableInit(void)
{
    memset(&receive_single_query, 0, sizeof(struct receiveSingleBoardQuery)); //清0缓存
    memset(&back_query, 0, sizeof(struct backQueryorComman));                        //清0缓存
    memset(&back_single_board, 0, sizeof(struct backSingleBoardQuery));              //清0缓存
    memset(&receive_health_management, 0, sizeof(struct receiveHealthManagement));   //清0缓存
    memset(&back_threshold_data, 0, sizeof(struct receiveHealthManagement));         //清0缓存
    memset(&back_health_query, 0, sizeof(struct backHealthQuery));                   //清0缓存
    memset(&receive_fan_query, 0, sizeof(struct FanState));                          //清0缓存
    memset(&back_fan_state, 0, sizeof(struct BackFanState));                         //清0缓存
    memset(&TxSerialData, 0, sizeof(struct serialChassisSend));                      //清0缓存
    memset(&TxSerialThreshold, 0, sizeof(struct serialThresholdSend));               //清0缓存
    memset(&TxSerialSingleBoard, 0, sizeof(struct serialSingleBoardSend));           //清0缓存
    memset(&RxSerialBack, 0, sizeof(struct serialBackRecv));                         //清0缓存
    memset(&RxSerialData, 0, sizeof(struct serialChassisRecv));                      //清0缓存
    memset(&RxHealthSerialData, 0, sizeof(struct serialHealthRecv));                 //清0缓存
    memset(&RxFanSerialData, 0, sizeof(struct serialFanData));                       //清0缓存
    memset(singleBoardBuffer, 0, sizeof(singleBoardBuffer));                         //清0缓存
    memset(healthManageBuffer, 0, sizeof(healthManageBuffer));                       //清0缓存
    memset(fanManageBuffer, 0, sizeof(fanManageBuffer));                             //清0缓存
    memset(writeBuffer, 0, sizeof(writeBuffer));                                     //清0缓存
    memset(readBuffer, 0, sizeof(readBuffer));                                       //清0缓存
}                                                                                    
/*********************************************************************
* 函数名称：networkInit
* 版本标识：v3.00
* 创建人：	徐圣乔
* 创建时间：2026年3月20日
* 功能描述：给 UDP 发送线程准备发送 socket + 目标地址
**********************************************************************/
int networkInit(void)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(WEB_ADDR);
    dest_addr.sin_port = htons(WEB_PORT);

    return 0;
}
/*********************************************************************
* 函数名称：udp_receive_thread
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月11日
* 功能描述：实现UDP数据的接收处理
**********************************************************************/
static void *udp_receive_thread(void *arg)
{
    (void)arg;
    unsigned char buffer[BUF_SIZE];
    ssize_t rx_udp_cnt = 0;
    socklen_t addr_len = sizeof(client_addr);

    int recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sockfd < 0) {
        perror("socket");
        return NULL;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SELF_PORT);
    if (bind(recv_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {// 绑定套接字 
        perror("bind");
        close(recv_sockfd);
        return NULL;
    }
    while (1) {
        // 接收数据
        //addr_len = sizeof(client_addr);
        rx_udp_cnt = recvfrom(recv_sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (rx_udp_cnt < 0) {
            perror("recvfrom");
            usleep(100 * 1000);
            continue;
        }

        if ((size_t)rx_udp_cnt == sizeof(struct receiveSingleBoardQuery)) {
            hasSentData = false;//串口可以发送数据标识
            hasSendUdpData = false;//udp可以发送数据标识
            memcpy(&receive_single_query, buffer, (size_t)rx_udp_cnt);//复制数据到结构体缓存
            recvUdpSingleHealthFan();//处理指令
        }
        else if ((size_t)rx_udp_cnt == sizeof(struct receiveHealthManagement)) {
            hasSentData = false;
            hasSendUdpData = false;
            memcpy(&receive_health_management, buffer, (size_t)rx_udp_cnt);//缓存健康管理阈值设数据
            recvUdpHealth();//处理指令
        }
        else {
            memset(buffer, 0, sizeof(buffer));
        }

        usleep(100 * 1000);
    }

    close(recv_sockfd);
    return NULL;
}
/*********************************************************************
* 函数名称：udp_send_thread
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月11日
* 功能描述：实现UDP发送线程UDP套接字
**********************************************************************/
static void *udp_send_thread(void *arg)
{
    (void)arg;

    while (1) {
        if (!hasSendUdpData) {
            if (queryFlag == 1) {//单板维护查询指令
                sendto(sockfd, &back_single_board, sizeof(struct backSingleBoardQuery), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                queryFlag = 0;
                hasSendUdpData = true;
            }
            else if (queryFlag == 2) {/* 返回健康管理数据 */
                temperatureAndVoltageCompare();
                makeSendData(3);
                sendto(sockfd, &back_health_query, sizeof(struct backHealthQuery), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                queryFlag = 0;
                hasSendUdpData = true;
            }
            else if (queryFlag == 3) {/* 返回风扇管理数据 */
                sendto(sockfd, &back_fan_state, sizeof(struct BackFanState), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                queryFlag = 0;
                hasSendUdpData = true;
            }
            else if (queryFlag == 4) {/* 返回阈值数据 */
                readDataFile(readBuffer, sizeof(readBuffer));
                memcpy(&back_threshold_data, readBuffer, sizeof(readBuffer));
                makeSendData(6);
                sendto(sockfd, &back_threshold_data, sizeof(struct receiveHealthManagement), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                queryFlag = 0;
                hasSendUdpData = true;
            }
        }

        usleep(50 * 1000);
    }

    return NULL;
}
/*********************************************************************
* 函数名称：serialDataTX
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月13日
* 功能描述：实现串口机箱管理数据帧的组包发送
**********************************************************************/
static void *serialDataTX(void *arg)
{
    (void)arg;
    int wireId = 0;

    while (1) {
        if (linkCnt == 0) {
            makeSendData(9);  //构建建链指令数据帧
            pthread_mutex_lock(&serial_mutex);  //加锁
            wireId = write(serialfd, &TxSerialData, sizeof(struct serialChassisSend)); // 发送串口数据
            pthread_mutex_unlock(&serial_mutex);  //解锁
        }

        if (chassisQuery != 0 && chassisQuery != 0x03 && chassisQuery != 0x0E && chassisQuery != 0x10) {//串口不再执行查询指令
            if (!hasSentData) { //只有在未发送数据时才执行发送操作
                makeSendData(5);
                pthread_mutex_lock(&serial_mutex);  //加锁
                wireId = write(serialfd, &TxSerialData, sizeof(struct serialChassisSend)); // 发送串口数据
                if (wireId < 0) {
                    printf("Write serial port failed！！！！\n");
                } else {
                    memcpy(tx_serial_buffer, &TxSerialData, sizeof(struct serialChassisSend));
                    log_output(LOG_INFO, "send the serial data");
                    memset(tx_serial_buffer, 0, sizeof(tx_serial_buffer));
                    memset(&TxSerialData, 0, sizeof(struct serialChassisSend));
                    hasSentData = true;  // 设置状态变量为已发送
                }
                pthread_mutex_unlock(&serial_mutex);  //解锁
            }
        } else {
            hasSentData = false;  // 如果命令不符合条件，重置状态变量
        }

        usleep(20 * 1000); // 延时20毫秒
    }

    return NULL;
}
/*********************************************************************
* 函数名称：serialDataRX
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月13日
* 功能描述：实现串口数据帧的接收
**********************************************************************/
static void *serialDataRX(void *arg)
{
    (void)arg;
    ssize_t usartRXlen = 0;						        	//读取串口返回字节长度
    unsigned char usartbuf[1024] = {0};						//串口接收数据缓存

    Seria_initialization();									//初始化串口
    while (1) {
        usartRXlen = read(serialfd, usartbuf, sizeof(usartbuf));     //循环读取串口是否来数据

        if ((size_t)usartRXlen == sizeof(struct serialChassisRecv)) {                        //判断接收的数据长度是否为单板维护数据
            memcpy(&RxSerialData, usartbuf, (size_t)usartRXlen);                             //复制数据到串口接收单板维护结构体中
            memset(usartbuf, 0, (size_t)usartRXlen);                                         //清零0缓存
            if (RxSerialData.frameHead == 0xF00F && RxSerialData.frameType == 0x83 && RxSerialData.frameEnd == 0x0EE0) {       //数据校验
                memcpy(singleBoardBuffer, &RxSerialData, sizeof(struct serialChassisRecv));  //缓存单板信息数据
                memcpy(&back_single_board, &RxSerialData, sizeof(struct serialChassisRecv)); //将数据复制到返回单板维护结构体中
                log_output(LOG_INFO, "Rx the single borad serial data");
            } else {
                memset(&RxSerialData, 0, sizeof(struct serialChassisRecv));                  //清零0缓存
            }
        }
        else if ((size_t)usartRXlen == sizeof(struct serialHealthRecv)) {							//判断接收的数据长度是否为健康管理数据
            memcpy(&RxHealthSerialData, usartbuf, (size_t)usartRXlen);								//复制数据到串口接收健康管理结构体中
            memset(usartbuf, 0, (size_t)usartRXlen);												//清零0缓存
            if (RxHealthSerialData.frameHead == 0xF00F && RxHealthSerialData.frameType == 0x8E && RxHealthSerialData.frameEnd == 0x0EE0) {
                memcpy(healthManageBuffer, &RxHealthSerialData, sizeof(struct serialHealthRecv));	//缓存健康管理数据
                for (int j = 0; j < 17; j++) {  													//复制数据到返回健康管理结构体中
                    memcpy(&back_health_query.healthAttribute[j].slot,
                           &RxHealthSerialData.serialHealth[j].slot, 1);
                    memcpy(&back_health_query.healthAttribute[j].boardName,
                           &RxHealthSerialData.serialHealth[j].boardName, 2);
                    memcpy(&back_health_query.healthAttribute[j].temperature,
                           &RxHealthSerialData.serialHealth[j].temperature, 1);
                    memcpy(&back_health_query.healthAttribute[j].voltage,
                           &RxHealthSerialData.serialHealth[j].voltage, 1);
                    memcpy(&back_health_query.healthAttribute[j].current,
                           &RxHealthSerialData.serialHealth[j].current, 1);
                }
                log_output(LOG_INFO, "Rx the health serial data");
            } else {
                memset(&RxHealthSerialData, 0, sizeof(struct serialHealthRecv));
            }
        }
        else if ((size_t)usartRXlen == sizeof(struct serialFanData)) {							//判断接收的数据长度是否为风扇管理数据
            memcpy(&RxFanSerialData, usartbuf, (size_t)usartRXlen);								//复制数据到串口风扇管理结构体中
            memset(usartbuf, 0, (size_t)usartRXlen);											//清零0缓存
            if (RxFanSerialData.frameHead == 0xF00F && RxFanSerialData.frameType == 0x90 && RxFanSerialData.frameEnd == 0x0EE0) {
                memcpy(fanManageBuffer, &RxFanSerialData, sizeof(struct serialFanData));        //数据缓存至fanManageBuffer
                memcpy(&back_fan_state, &RxFanSerialData, sizeof(struct serialFanData));        //复制数据到风扇管理结构体中
                log_output(LOG_INFO, "Rx the fan serial data");
            } else {
                memset(&back_fan_state, 0, sizeof(struct BackFanState));
            }
        }else if((size_t)usartRXlen == sizeof(struct serialFanData))//判断接收的数据长度是否为建链回执
		{
		    memcpy(&RxSerialBack,usartbuf,(size_t)usartRXlen);//复制数据到串口建链回执结构体中
			//processBuffer(usartbuf,usartRXlen,"Rx the link serial data");
			memset(usartbuf,0,(size_t)usartRXlen);//清零0缓存
			if(RxSerialBack.frameHead == 0xF00F && RxSerialBack.frameType == 0x81 && RxSerialBack.frameEnd == 0x0EE0)
			{
				linkCnt++;							//建链成功，将建链成功参数置为1
                //printf("\r\n ====================================> Rx the fan serial data \r\n");
                log_output(LOG_INFO,"Rx the link serial data");
			}
			else
			{
			   memset(&RxSerialBack,0,sizeof(struct serialFanData));
			}
		}
        else if (usartRXlen > 0) {
            memset(usartbuf, 0, (size_t)usartRXlen);
        }

        usleep(50 * 1000);
    }

    return NULL;
}
/*********************************************************************
* 函数名称：main
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：主函数，程序入口，完成初始化、装配状态加载和线程创建
**********************************************************************/
int main(void)
{
    variableInit();
    if (networkInit() != 0) {
        log_output(LOG_ERROR, "network init failed");
        return 1;
    }

    if (pthread_create(&serialRecvId, NULL, serialDataRX, NULL) == -1) {					//创建串口数据接收线程
        log_output(LOG_ERROR, "create CManagement serialDataRX pthread failed");
        return 0;
    } else {
        log_output(LOG_INFO, "create CManagement serialDataRX pthread succeed");
    }

    if (pthread_create(&serialSendId, NULL, serialDataTX, NULL) == -1) {					//创建串口数据发送处理线程
        log_output(LOG_ERROR, "create CManagement serialDataTX pthread failed");
        return 0;
    } else {
        log_output(LOG_INFO, "create CManagement serialDataTX pthread succeed");
    }

    if (pthread_create(&send_tid, NULL, udp_send_thread, NULL) == -1) {						//创建UDP发送线程
        log_output(LOG_ERROR, "create CManagement udp_send_thread pthread failed");
        return 0;
    } else {
        log_output(LOG_INFO, "create CManagement udp_send_thread pthread succeed");
    }

    if (pthread_create(&receive_tid, NULL, udp_receive_thread, NULL) == -1) {				//创建UDP接收线程
        log_output(LOG_ERROR, "create CManagement udp_receive_thread pthread failed");
        return 0;
    } else {
        log_output(LOG_INFO, "create CManagement udp_receive_thread pthread succeed");
    }

    pthread_join(receive_tid, NULL);
    pthread_join(send_tid, NULL);
    pthread_join(serialRecvId, NULL);
    pthread_join(serialSendId, NULL);

    close(sockfd);
    return 0;
}
