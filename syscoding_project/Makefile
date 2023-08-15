# 定义编译器
CC = arm-linux-gcc

# 定义目标文件夹路径
OBJ_DIR = obj

# 定义需要链接的库文件路径和库名
LIB_DIR = public/lib
LDFLAGS = -L$(LIB_DIR) -lpthread -lm -lpng -lfontfzy

# 获取所有源文件
DRAW_SRCS = $(wildcard draw/src/*.c)
PTHREAD_SRCS = $(wildcard pthread/src/*.c)
LCD_SRCS = $(wildcard lcd/src/*.c)
USR_SRCS = $(wildcard usr/src/*.c)

# 将源文件路径替换为目标文件路径
DRAW_OBJS = $(patsubst draw/src/%.c, $(OBJ_DIR)/draw/%.o, $(DRAW_SRCS))
PTHREAD_OBJS = $(patsubst pthread/src/%.c, $(OBJ_DIR)/pthread/%.o, $(PTHREAD_SRCS))
LCD_OBJS = $(patsubst lcd/src/%.c, $(OBJ_DIR)/lcd/%.o, $(LCD_SRCS))
USR_OBJS = $(patsubst usr/src/%.c, $(OBJ_DIR)/usr/%.o, $(USR_SRCS))

# 定义头文件搜索路径
INC_DIRS = draw/inc pthread/inc lcd/inc usr/inc public/inc

# 创建目标文件夹
$(shell mkdir -p $(OBJ_DIR)/draw)
$(shell mkdir -p $(OBJ_DIR)/pthread)
$(shell mkdir -p $(OBJ_DIR)/lcd)
$(shell mkdir -p $(OBJ_DIR)/usr)

# 声明编译规则（将源文件编译为目标文件）
$(OBJ_DIR)/draw/%.o: draw/src/%.c
	$(CC) -o $@ -c $< $(addprefix -I , $(INC_DIRS))

$(OBJ_DIR)/pthread/%.o: pthread/src/%.c
	$(CC) -o $@ -c $< $(addprefix -I , $(INC_DIRS))

$(OBJ_DIR)/lcd/%.o: lcd/src/%.c
	$(CC) -o $@ -c $< $(addprefix -I , $(INC_DIRS))

$(OBJ_DIR)/usr/%.o: usr/src/%.c
	$(CC) -o $@ -c $< $(addprefix -I , $(INC_DIRS))

# 声明默认目标（生成可执行文件）
TARGET = $(OBJ_DIR)/main
$(TARGET): $(DRAW_OBJS) $(PTHREAD_OBJS) $(LCD_OBJS) $(USR_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# 声明伪目标（用于执行清理操作）
.PHONY: clean

# 清理规则（删除目标文件和生成的可执行文件）
clean:
	rm -rf $(OBJ_DIR)

