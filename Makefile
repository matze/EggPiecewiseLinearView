CFLAGS=`pkg-config --cflags gtk+-2.0` -g -ggdb -Wall -Werror -std=c99
LDFLAGS=`pkg-config --libs gtk+-2.0`
DEPS=egg-piecewise-linear-view.h
OBJ=pwl-test.o egg-piecewise-linear-view.o

all: pwl-test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pwl-test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJ)
	rm -f pwl-test
