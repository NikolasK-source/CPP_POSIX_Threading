SRC_FILES := $(wildcard *.cpp)
OBJ_FILES := $(patsubst %.cpp,%.o,$(SRC_FILES))

all: static_lib
static_lib: libThreading.a

libThreading.a: $(OBJ_FILES)
	ar rcs $@ $^

%.o: %.cpp
	g++ -std=c++11 -O2 -pthread -Wall -Wextra -c $< -o $@

clean:
	rm -f *.o *.a
