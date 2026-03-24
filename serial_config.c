/*********************************************************************
* 文件名称：serial_config
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 包含的模块名称：
* 主要功能描述：实现串口信息配置和和线程创建。
* 运行环境要求：运行于各LINUX系统上，设备至少具备一路网口和路串口，运行内存不小于512M，存储空间不小于4G。
* 修改内容1：
* 修改人1：
* 修改时间1：
* ...
* 修改内容n：
* 修改人n：
* 修改时间n：
**********************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>         //包含文件控件，像O_RDWR
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>       // 错误代码和strerror()函数
#include <sys/socket.h>
#include <unistd.h>		// write(), read(), close()
#include <termios.h>	

#define	SERIA_SITE	"/dev/ttyS7"	//定义LINUX下所调用串口端口位置及名称

int serialfd;
void Seria_initialization(void); 	//串口初始化函数
/*********************************************************************
* 函数名称：set_baudrate
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口波特率设置	
* 函数输入：opt（串口配置结构体指针）、baudrate(波特率，可选范围B0,  B50,  B75,  B110, 
			B134,  B150,  B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200,
			B38400, B57600, B115200, B230400, B460800)
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
static void set_baudrate(struct termios *opt,unsigned int baudrate)
{
    cfsetispeed(opt, baudrate);
    cfsetospeed(opt, baudrate);
}
/*********************************************************************
* 函数名称：set_data_bit
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口收/发数据位数的设置	
* 函数输入：opt（串口配置结构体指针）、databit(数据位数，可选范围CS5,CS6,CS7,CS8)
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
static void set_data_bit (struct termios *opt,unsigned int databit)
{
    switch (databit) {
    case 8:
        opt->c_cflag |= CS8;
        break;
    case 7:
        opt->c_cflag |= CS7;
        break;
    case 6:
        opt->c_cflag |= CS6;
        break;
    case 5:
        opt->c_cflag |= CS5;
        break;
    default:
        opt->c_cflag |= CS8;
        break;
    }
}
/*********************************************************************
* 函数名称：set_parity
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口收/发数据校验方式的设置	
* 函数输入：opt（串口配置结构体指针）、parity(校验，选值范围N，E，O)
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
static void set_parity (struct termios *opt, char parity)
{
    switch (parity) {
    case 'N':                  /* 无校验 */
        opt->c_cflag &= ~PARENB;
        break;
    case 'E':                  /* 偶校验 */
        opt->c_cflag |= PARENB;
        opt->c_cflag &= ~PARODD;
        break;
    case 'O':                  /* 奇校验 */
        opt->c_cflag |= PARENB;
        opt->c_cflag |= ~PARODD;
        break;
    default:                   /* 无校验 */
        opt->c_cflag &= ~PARENB;
        break;
    }
}
/*********************************************************************
* 函数名称：set_stopbit
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口收/发停止位数的设置	
* 函数输入：opt（串口配置结构体指针）、stopbit(选值范围1或2)
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
static void set_stopbit (struct termios *opt, const char *stopbit)
{
    if (0 == strcmp (stopbit, "1")) {
        opt->c_cflag &= ~CSTOPB; /* 1 stop bit */
    }  
	else if (0 == strcmp (stopbit, "2")) {
        opt->c_cflag |= CSTOPB;  /* 2 stop bits */
    } 
	else {
        opt->c_cflag &= ~CSTOPB; /* 1 stop bit */
    }
}
/*********************************************************************
* 函数名称：set_serial_attr
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口收/发所有属性的设置	
* 函数输入：fd（打开串口返回编号）、baudrate(波特率)、databit（数据位）
			stopbit（停止位）、parity（校验）、
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
static void set_serial_attr (int fd, int  baudrate, int  databit, const char *stopbit, char parity,int vtime,int vmin)
{
    struct termios opt;
    tcgetattr(fd, &opt);					//读取现有设置
    set_baudrate(&opt, baudrate);			//设置波特率
    set_data_bit(&opt, databit);			//设置数据位
    set_parity(&opt, parity);				//设置校验位
    set_stopbit(&opt, stopbit);				//设置停止位
	//其它设置
	opt.c_cflag |= CLOCAL | CREAD;			//打开READ，并忽视调制状态，禁用调制解调器特定的信号线
	opt.c_cflag &= ~CRTSCTS;				//禁用硬件RTS/CTS流控制
//    opt.c_lflag |= 0;
	opt.c_lflag &= ~ICANON;					//禁用规范模式
	opt.c_lflag &= ~ECHO; 					//禁用发送字符回显，Disable echo
	opt.c_lflag &= ~ECHOE; 					// Disable erasure
	opt.c_lflag &= ~ECHONL; 				// Disable new-line echo
	opt.c_lflag &= ~ISIG; 					// Disable interpretation of INTR, QUIT and SUSP
	opt.c_iflag &= ~(IXON | IXOFF | IXANY); // 禁用软件流控制（IXOFF，IXON，IXANY）
	opt.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);//禁用接收时字节的特殊处理
//	opt.c_oflag = 0;
    opt.c_oflag &= ~OPOST;					//禁用输出字符的特殊处理
	opt.c_oflag &= ~ONLCR;
    opt.c_cc[VTIME] = vtime;				//VMIN = 0，VTIME = 0：无阻塞，立即返回可用值，VMIN> 0，VTIME = 0：这将read()始终等待字节（确切地由决定多少个字节VMIN），因此read()可以无限期地阻塞。
    opt.c_cc[VMIN]  = vmin;					//VMIN = 0，VTIME> 0：这是对最大超时（由给出VTIME）的任何数字字符的阻塞读取。read()将阻塞直到有大量数据可用或发生超时为止。
											//VMIN> 0，VTIME> 0：阻塞直到VMIN接收到任何字符或VTIME第一个字符过去后超时。请注意，VTIME直到收到第一个字符，超时才会开始。
    tcflush (fd, TCIFLUSH);					//刷新输入缓存，丢弃接收的缓存数据
    if(tcsetattr (fd, TCSANOW, &opt)!= 0) {	//保存配置
		printf("Error %i from tcsetattr: %s ヘ(_ _ヘ)\n", errno, strerror(errno));
	}
}

/*********************************************************************
* 函数名称：Seria_initialization
* 版本标识：v3.00
* 创建人：	张法军
* 创建时间：2024年4月10日
* 功能描述：实现串口的初始化配置和对串口收发数据缓存清空
* 函数输入：无
* 函数输出：无
* 修改内容1：
* 修改人1：
* 修改时间1：
**********************************************************************/
void Seria_initialization(void)
{
	serialfd = open(SERIA_SITE,O_RDWR|O_NOCTTY);				//打开串口
	if (serialfd < 0) {														
		printf("Open Serial port failed, code %i from open: %s \n", errno, strerror(errno));
	}
	else printf("Open Serial port succeed!\n");
	set_serial_attr(serialfd,B460800,8,"1",'N',0,0);				//串口参数配置
}
