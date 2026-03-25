#include "chassis_management.h"
/**********************************************************************************************************************************************************************
* 函数名称：log_output
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2025年4月15日
* 功能描述：日志信息输出
* 函数输入：日志标识及信息内容
***********************************************************************************************************************************************************************/
void log_output(LogLevel level, const char* message)
{
    switch (level) {
    case LOG_DEBUG:
        printf("[DEBUG] --------- %s \r\n", message);
        break;
    case LOG_INFO:
        printf("[INFO] --------- %s \r\n", message);
        break;
    case LOG_WARN:
        printf("[WARN] --------- %s \r\n", message);
        break;
    case LOG_ERROR:
        printf("[ERROR] --------- %s \r\n", message);
        break;
    default:
        break;
    }
}
/**********************************************************************************************************************************************************************
* 函数名称：CalcChecksum
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2026年3月17日
* 功能描述：计算累加和校验
* 函数输入：数据报文与帧长度
***********************************************************************************************************************************************************************/
unsigned short CalcChecksum(unsigned char *data, int len)
{
    unsigned short checksum = 0;

    if (data == NULL || len <= 0) {
        return 0;
    }

    for (int i = 0; i < len; ++i) {
        checksum += data[i];
    }

    return checksum;
}
/**********************************************************************************************************************************************************************
* 函数名称：writeDataToFile  
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年11月7日
* 功能描述：创建文件并将数据写入文件
* 函数输入：buffer数据和数据长度
***********************************************************************************************************************************************************************/
int writeDataToFile(unsigned char data[], size_t dataSize) {
    FILE *file; //文件指针
    file = fopen("thresholdfile.txt", "wb"); //以二进制写入模式打开文件，如果不存在则创建
    if (file == NULL)// 检查文件是否成功打开
	{ 
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    // 向文件写入数据
    if (fwrite(data, sizeof(unsigned char), dataSize, file) != dataSize) {
        perror("Error writing to file");
        fclose(file);
        return EXIT_FAILURE;
    }
    // 关闭文件
    fclose(file);
    printf("Data successfully written to file.\n");
    // 设置文件权限为 0644 (rw-r--r--)
    if (chmod("thresholdfile.txt", 0644) != 0) 
	{
        perror("Error setting file permissions");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/**********************************************************************************************************************************************************************
* 函数名称：readDataFile 
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年11月7日
* 功能描述：读取文件中的数据
* 函数输入：buffer数据和数据长度
***********************************************************************************************************************************************************************/
int readDataFile(unsigned char data[], size_t dataSize) {
    FILE *file; // 文件指针

    // 尝试以读写模式打开文件
    file = fopen("thresholdfile.txt", "rb+");
    if (file == NULL) { // 文件不存在
        // 尝试创建文件
        file = fopen("thresholdfile.txt", "wb+");
        if (file == NULL) { // 文件创建失败
            perror("Error creating file");
            return EXIT_FAILURE;
        }
        // 文件已创建，但无数据可读
        printf("File created but no data to read.\n");
        fclose(file);
        return EXIT_SUCCESS;
    }

    // 文件存在，读取数据
    size_t bytesRead = fread(data, sizeof(unsigned char), dataSize, file);
    if (bytesRead != dataSize) {
        if (feof(file)) {
            printf("End of file reached before reading enough data.\n");
        } else {
            perror("Error reading file");
        }
        fclose(file);
        return EXIT_FAILURE;
    }

    // 关闭文件
    fclose(file);
    printf("Data successfully read from file.\n");
    return EXIT_SUCCESS;
}
/********************************************************************************************************************************************************************
* 函数名称：temperatureAndVoltageCompare
* 功能描述：温度电压产生警告状态数据对比
*********************************************************************************************************************************************************************/
void temperatureAndVoltageCompare(void){
    
    readDataFile(readBuffer,sizeof(readBuffer));//读取数据
	memcpy(&receive_threshold_data,readBuffer,sizeof(readBuffer));
    for (int i = 0; i < 17; i++) // 实际温度值与设置阈值温度数据比较，形成温度警告阈值
    {
	    printf("temperature %d \r\n",back_health_query.healthAttribute[i].temperature);
        if ((int)(back_health_query.healthAttribute[i].temperature)  < receive_threshold_data.temperatureLowerNR &&
            (int)(back_health_query.healthAttribute[i].temperature) >= receive_threshold_data.temperatureLowerCR)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x00; // 轻微超下限
        }else if ((int)(back_health_query.healthAttribute[i].temperature) < receive_threshold_data.temperatureLowerCR &&
                 (int)(back_health_query.healthAttribute[i].temperature) >= receive_threshold_data.temperatureLowerNC)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x01; // 较多超下限
        }else if ((int)(back_health_query.healthAttribute[i].temperature) < receive_threshold_data.temperatureLowerNC)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x02; // 严重超下限
        }else if ((int)(back_health_query.healthAttribute[i].temperature) > receive_threshold_data.temperatureUpperNR &&
                (int)(back_health_query.healthAttribute[i].temperature) <= receive_threshold_data.temperatureUpperCR)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x03; // 轻微超上限
        }else if ((int)(back_health_query.healthAttribute[i].temperature) > receive_threshold_data.temperatureUpperCR &&
                 (int)(back_health_query.healthAttribute[i].temperature) < receive_threshold_data.temperatureUpperNC)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x04; // 较多超上限
        }else if ((int)(back_health_query.healthAttribute[i].temperature) >= receive_threshold_data.temperatureUpperNC)
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x05; // 严重超上限
        }else
        {
            back_health_query.healthAttribute[i].temperatureWarning = 0x0f; // 正常
        }
    }

    for (int k = 0; k < 17; k++) {// 实际电压值与设置阈值比较，形成电压阈值警告
        float voltage = back_health_query.healthAttribute[k].voltage / 10.0f;
        if (voltage > receive_threshold_data.voltageMax) {
            back_health_query.healthAttribute[k].voltageWarning = 0x01; // 电压超上限
        }
        else if (voltage < receive_threshold_data.voltageMin) {
            back_health_query.healthAttribute[k].voltageWarning = 0x02; // 电压超下限
        }
        else {
            back_health_query.healthAttribute[k].voltageWarning = 0x0f; // 电压正常
        }
    }
}
/**********************************************************************************************************************************************************************
* 函数名称：makeSendData
* 功能描述：完成对网络发送数据的帧构建
* 函数输入：makeclasses（构建数据类别，1建链回执帧，2单板维护查询回执，3单板维护设置操作回执，4健康管理查询回执，5健康管理指令操作回执，6风扇管理回执，7建阈值查询返回帧）
***********************************************************************************************************************************************************************/
void makeSendData(unsigned char makeclasses){
	switch(makeclasses)
	{
	case 1:       //构建单板维护查询回执帧
	{
		back_single_board.frameHead=0x0FF0;
		back_single_board.frameCnt=singleQueryCnt;
		back_single_board.frameType=0x03;
		back_single_board.backup1=0x00;
		back_single_board.backup2=0x00;
		back_single_board.frameEnd=0xE00E;
		singleQueryCnt++;
		break;
	}
	case 2:   //构建设置操作回执帧(包含单板维护、健康管理和风扇管理)
	{
		back_query.frameHead=0x0FF0;
		back_query.frameCnt=singleQueryCnt;
        back_query.frameType = backCommand;//返回web设置指令		
		back_query.backup = 0x00;
		back_query.frameEnd=0xE00E;
		singleQueryCnt++;
		break;
	}
	case 3:  //构建健康管理查询回执帧
	{
	    back_health_query.frameHead=0x0FF0;
		back_health_query.frameCnt=healthQueryCnt;
		back_health_query.frameType=0x8E;
		back_health_query.backup1=0x00;
		back_health_query.backup2=0x00;
		back_health_query.frameEnd=0xE00E;
		healthQueryCnt++;
		break;
	}
	case 4: ////构建风扇管理回执帧
	{
		back_fan_state.frameHead=0x0FF0;
		back_fan_state.frameType=0x90;
		back_fan_state.frameCnt=fanQueryCnt;
		back_fan_state.backup1=0x00;
		back_fan_state.backup2=0x00;
		back_fan_state.frameEnd=0xE00E;
		fanQueryCnt++;
		break;
	}
	case 5://串口发送查询数据组帧
	{
		TxSerialData.frameHead = 0x0FF0;
		TxSerialData.frameCnt=txdataCnt;
		TxSerialData.slot = querySlot;
		TxSerialData.frameType = chassisQuery;
		chassisQuery = 0;
		TxSerialData.backup = 0x00;
		TxSerialData.frameSum = CalcChecksum((unsigned char *)&TxSerialData, sizeof(serialChassisSend)-4);
		TxSerialData.frameEnd = 0xE00E;
		txdataCnt++; 
		break;
	}
	case 6://构建健康管理阈值设置回执帧
	{
	    receive_threshold_data.frameHead=0x0FF0;
		receive_threshold_data.frameCnt=healthSetCnt;
		receive_threshold_data.frameEnd=0xE00E;
		healthSetCnt++;
		break;
	}
	case 7://串口发送构建阈值查询返回组帧
	{
		TxSerialThreshold.frameHead = 0x0FF0;
		TxSerialThreshold.frameCnt = txThresholdCnt;
		TxSerialThreshold.frameType = 0x02;
		TxSerialThreshold.voltage1RefValue = 0x00;//电压1基准值
		TxSerialThreshold.voltage2RefValue = 0x00;//电压2基准值
		TxSerialThreshold.voltage3RefValue = 0x00;//电压3基准值
		TxSerialThreshold.frameSum = CalcChecksum((unsigned char *)&TxSerialThreshold, sizeof(serialThresholdSend)-4);
		TxSerialThreshold.frameEnd = 0xE00E;
		txThresholdCnt++;
		break;
	}
	case 8://串口发送单板信息数据组帧
	{
		TxSerialSingleBoard.frameHead = 0x0FF0;
		TxSerialSingleBoard.frameCnt = txSingleBoardCnt;
		TxSerialSingleBoard.frameType = 0x83;
		TxSerialSingleBoard.frameEnd = 0xE00E;
		txSingleBoardCnt++;
		break;
	}
	case 9://串口发送建链数据组帧
	{
		TxSerialData.frameHead = 0x0FF0;
		TxSerialData.frameCnt=txdataCnt;
		TxSerialData.slot = 0x00;
		TxSerialData.frameType = 0x01;
		TxSerialData.backup = 0x00;
		TxSerialData.frameSum = CalcChecksum((unsigned char *)&TxSerialData,8);
		TxSerialData.frameEnd = 0xE00E;
		txdataCnt++; 
		break;
	}
	default : makeclasses = 0;
	    break;
	}
}
/**********************************************************************************************************************************************************************
* 函数名称：recvUdpSingleHealthFan
* 功能描述：接收udp单板信息、健康管理和风扇管理查询指令处理
***********************************************************************************************************************************************************************/
void recvUdpSingleHealthFan(void){
    if(receive_query.frameHead == 0x0FF0 && receive_query.frameEnd == 0xE00E)
	{
		querySlot = receive_query.slot;//缓存查询槽位
		chassisQuery = receive_query.frameType;//缓存查询指令
		switch(receive_query.frameType)//查询指令和控制指令
		{
		case 0xB0://返回IPMB-B使能
		{
			backCommand = 0x5A;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
			break;
		}
		case 0x03://单板维护查询指令
		{
			queryFlag = 1;
			break;
		}
		case 0x0E://健康管理查询指令
		{
		    queryFlag = 2;
			break;
		}
		case 0x10://风扇管理查询指令
		{
			queryFlag = 3;
			break;
		}
		case 0xB4://返回IPMB-B禁止使能
		{
			backCommand = 0x5A;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
			break;
		}
		case 0xB5://阈值查询指令
		{
		    /*readDataFile(readBuffer,sizeof(readBuffer));//读取数据
			memcpy(&receive_threshold_data,readBuffer,sizeof(readBuffer));
			makeSendData(6);
			sendto(sockfd,&receive_threshold_data,sizeof(struct receiveThreshold),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));//返回阈值
		     */
            queryFlag = 4;
			break;
		}
		case 0x0A://一档设置成功
		{
			backCommand = 0x8A;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x0B://二档设置成功
		{
		    backCommand = 0x8B;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x0C://三档设置成功
		{
		    backCommand = 0x8C;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x04://重新启动payload复位
		{
		    backCommand = 0x84;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x06: //payloa断电
		{
		    backCommand = 0x86;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x05://paylo上电
		{
		    backCommand = 0x85;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		case 0x07://IPMC复位
		{
		    backCommand = 0x87;
			makeSendData(2);
			sendto(sockfd,&back_query,sizeof(struct backQueryorComman),0,(struct sockaddr *)&dest_addr,sizeof(dest_addr));
		    break;
		}
		default: memset(&back_query,0,sizeof(struct backQueryorComman));//清0缓存
		}
	}
}
/**********************************************************************************************************************************************************************
* 函数名称：recvUdpThreshold
* 功能描述：接收udp阈值设置指令数据处理
***********************************************************************************************************************************************************************/
void recvUdpThreshold(void){
	if(receive_threshold_data.frameHead==0x0FF0 && receive_threshold_data.frameEnd==0xE00E && receive_threshold_data.frameType == 0x02)
	{	
        TxSerialThreshold.temperatureLowerNR = (int16_t)(receive_threshold_data.temperatureLowerNR * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.temperatureLowerCR = (int16_t)(receive_threshold_data.temperatureLowerCR * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.temperatureLowerNC = (int16_t)(receive_threshold_data.temperatureLowerNC * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.temperatureUpperNR = (int16_t)(receive_threshold_data.temperatureUpperNR * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.temperatureUpperCR = (int16_t)(receive_threshold_data.temperatureUpperCR * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.temperatureUpperNC = (int16_t)(receive_threshold_data.temperatureUpperNC * 10);     	//更新接收到的最新阈值
        TxSerialThreshold.voltage1MinThreshold = (int16_t)(receive_threshold_data.voltageMin * 100);			//更新接收到的最新阈值
        TxSerialThreshold.voltage2MinThreshold = (int16_t)(receive_threshold_data.voltageMin * 100);			//更新接收到的最新阈值
        TxSerialThreshold.voltage3MinThreshold = (int16_t)(receive_threshold_data.voltageMin * 100);			//更新接收到的最新阈值
        TxSerialThreshold.voltage1MaxThreshold = (int16_t)(receive_threshold_data.voltageMax * 100);			//更新接收到的最新阈值
        TxSerialThreshold.voltage2MaxThreshold = (int16_t)(receive_threshold_data.voltageMax * 100);			//更新接收到的最新阈值
        TxSerialThreshold.voltage3MaxThreshold = (int16_t)(receive_threshold_data.voltageMax * 100);			//更新接收到的最新阈值
	    memcpy(writeBuffer,&receive_threshold_data,sizeof(struct receiveThreshold));//复制数据到写入数组
		writeDataToFile(writeBuffer,sizeof(writeBuffer));//写入数据	
	}
	else
	{
		memset(&receive_threshold_data,0,sizeof(struct receiveThreshold));//清零缓存
	}
}
