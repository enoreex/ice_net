#pragma once

#ifdef _WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <cstring>

#else

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno> 

typedef int SOCKET;

#endif

#include "../ice.core/ice_logger.h"
#include "end_point.h"

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#endif

class udp_sock
{

private:

    SOCKET sock = 0;

    sockaddr_in local_in = sockaddr_in();

#ifdef _WIN32

private:

    char win_error_msg[256];

#endif

private:

    char buffer[65535];

public:

    struct recv_result
    {
        char* recv_arr = nullptr;
        unsigned short recv_size = 0;
        end_point recv_point = end_point(0, 0);
        bool auto_release = false;
    };

    enum recv_predicate_code
    {
        accept,
        reject,
        temp,
    };

    typedef std::function<recv_predicate_code(char, end_point&)> recv_predicate;

public:

    enum recv_mode
    {
        single,
        shared,
    };

public:

    recv_mode recv_mode = single;

public:

    end_point get_local_point();

    bool start(end_point local_point);

    bool receive_available();

    recv_result receive(recv_predicate predicate);

    bool send(const char* data, unsigned short data_size, end_point& remote_point);

    void stop();
};
