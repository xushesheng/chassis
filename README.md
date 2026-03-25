README
项目简介

本项目是一个基于 C 语言实现的机箱管理程序，运行于 Linux 环境。

系统通过以下方式与外部模块通信：

UDP 网络通信：与 Web 网管软件进行数据交互
串口通信：与机箱及 CHMC 模块进行数据交互

程序采用模块化设计，将协议定义、配置、业务处理和流程控制进行解耦。

项目结构
.
├── chassis_management_main.c   # 主程序入口与线程调度
├── chassis_function.c         # 业务逻辑实现
├── serial_config.c            # 串口配置与初始化
├── chassis_management.h       # 公共配置与接口声明
├── chassis_structs_chmc.h     # CHMC 协议结构定义
├── chassis_structs_web.h      # Web 协议结构定义
├── Makefile                  # 编译脚本
系统架构

程序分为五个核心模块：

1. 协议层
定义机箱与 CHMC / Web 的通信数据结构
2. 公共配置层
提供宏定义、全局变量、接口声明
3. 业务处理层
负责协议解析、数据处理、状态判断等逻辑
4. 调度与控制层
管理线程生命周期与主流程
5. 串口驱动层
负责串口初始化与参数配置（基于 termios）

编译说明
1. 依赖环境
Linux 系统
GCC 工具链（当前 Makefile 使用 LoongArch 交叉编译器）
Makefile 中默认编译器：
/opt/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.2/bin/loongarch64-linux-gnu-gcc
如需使用本机 gcc，可修改：
CC=gcc

2. 编译
在项目根目录执行：
make
生成可执行文件：
zk
3. 清理构建
make clean
运行方式
1. 直接运行
./zk
或使用 Makefile：
make run
2. 运行说明

程序启动后将：
初始化网络（UDP）
初始化串口设备
创建多线程执行：
UDP 接收线程
UDP 发送线程
串口接收线程
串口发送线程
编译选项说明

Makefile 中关键参数：
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

说明：
-Wall -Wextra：开启编译警告
-pthread：启用多线程支持（编译 + 链接）
扩展说明
协议扩展：修改 chassis_structs_*.h
业务扩展：新增或修改 chassis_function.c
流程控制：调整 chassis_management_main.c
串口参数：修改 serial_config.c
项目特点
模块职责清晰，易于维护
网络通信与串口通信解耦
多线程架构，支持并发处理
支持协议与业务灵活扩展