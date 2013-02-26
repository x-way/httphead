all: httphead

httphead: httphead.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f httphead
