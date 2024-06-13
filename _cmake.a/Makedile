CC := g++
CFLAGS := -std=c++17 -Wall -Wextra

INC := -I../ice_net/src/_ice.net.lib 

LIB := -L../ice_net/_cmake.a -lice_net

SRCS := main.cpp

TARGET := main

all: $(TARGET)

$(TARGET): $(SRCS)
        $(CC) $(CFLAGS) $(INC) $^ -o $@ $(LIB)

clean:
        rm -f $(TARGET)