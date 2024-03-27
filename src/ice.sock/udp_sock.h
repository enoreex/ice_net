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

typedef int SOCKET;

#endif

#include "..\ice.core\ice_logger.h"
#include "..\ice.rudp\common\transport\a_sock.h"

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#endif

class udp_sock final : public a_sock
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

    ~udp_sock();

public:

    enum recv_mode 
    {
        single,
        shared,
    };

public:

    recv_mode recv_mode = single;

public:

    end_point get_local_point() override;

    bool start(end_point local_point) override;

    bool receive_available() override;

    recv_result receive(recv_predicate predicate) override;

    bool send(const char* data, unsigned short data_size, const end_point& remote_point) override;

    void stop() override;
};