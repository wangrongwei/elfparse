
target := elfp

C_SOURCES = $(shell find . -name "*.c")

CC = gcc
C_FLAGS = -c -Wall -g

.PHONY:all
all:
	@echo "编译elf解析工具"
	$(CC) elfparse.c -o $(target)
.PHONY:clean
clean:
	rm -f $(target)





