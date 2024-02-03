#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#include "..\ice.rudp\common\transport\a_server.h"

#pragma comment(lib, "ws2_32.lib")

class win_udp_server final : public a_server
{

private:

    SOCKET sock = INVALID_SOCKET;

    sockaddr_in local_in = sockaddr_in();

public:

    ~win_udp_server();

public:

    end_point get_local_point() override;

    bool start(end_point& local_point);

    bool receive_available() override;

    recv_result receive() override;

    bool send(char* data, unsigned short data_size, const end_point& remote_point) override;

    void stop() override;
};