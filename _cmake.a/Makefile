CC := g++
CFLAGS := -std=c++20 -Wall -Wextra
LIBS := -pthread

SRC_DIR := ../src/_ice.net.static
CORE_DIR := ../src/ice.core
RUDP_DIR := ../src/ice.rudp
SOCK_DIR := ../src/ice.sock

SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(CORE_DIR)/*.cpp) \
        $(wildcard $(RUDP_DIR)/*.cpp) \
        $(wildcard $(RUDP_DIR)/common/*.cpp) \
        $(wildcard $(RUDP_DIR)/common/transport/*.cpp) \
        $(wildcard $(SOCK_DIR)/*.cpp)

INCS := -I../src/_ice.net.static \
        -I../src/ice.core \
        -I../src/ice.rudp \
        -I../src/ice.rudp/common \
        -I../src/ice.rudp/common/transport \
        -I../src/ice.sock

OBJS := $(SRCS:.cpp=.o)

LIBRARY := libice_net.a

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
        ar rcs $@ $(OBJS)

%.o: %.cpp
        $(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
        rm -f $(OBJS) $(LIBRARY)

.PHONY: all clean