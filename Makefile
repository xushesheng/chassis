CC= /opt/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.2/bin/loongarch64-linux-gnu-gcc
CFLAGS = -Wall -Wextra -pthread   # 编译选项，-pthread用于编译和链接
LDFLAGS = -pthread                # 链接选项
TARGET = zk

SRCS = \
	chassis_management_main.c \
	chassis_function.c \
	serial_config.c

OBJS = $(SRCS:.c=.o)
DEPS = chassis_management.h chassis_structs_chmc.h chassis_structs_web.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run