CC=gcc
ld=gcc -Wall

CC_INCLUDE=
CFLAGS+=-c -m64 -std=c11 -g -lrt
CFLAGS+=$(CC_INCLUDE)


objs=madvise.o

target=madvise

$(target): $(objs)
	$(ld) $(objs) -o $@ -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find . -name "*.o" | xargs rm -f
	rm -f $(target)
	rm -rf statm*
