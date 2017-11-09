CC=gcc

CC_INCLUDE+=
CFLAGS+=-Wall -fPIC -shared -m64 -std=c11 -g -lrt
CFLAGS+=$(CC_INCLUDE)
LFLAGS+=-llua

SRC=shared_mem.c
TARGET=libshared_dict.so

all:
	$(CC) $(CFLAGS) $(LFLAGS) -o $(TARGET) $(SRC)

clean:
	@rm $(TARGET)
	@rm -rf $(TARGET).dSYM
