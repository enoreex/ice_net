#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

#include "..\ice_net.h"

#pragma comment(lib, "ws2_32.lib")

class win_udp_client final : public a_client
{
private:

    SOCKET sock = INVALID_SOCKET;
    sockaddr_in local_in = sockaddr_in();
    sockaddr_in remote_in = sockaddr_in();

public:

    ~win_udp_client();

public:

    end_point get_local_point() override;

    end_point get_remote_point() override;
    
    bool connect(end_point& remote_point, end_point& local_point) override;
    
    bool receive_available() override;
    
    recv_result receive() override;
    
    bool send(char* data, unsigned short data_size) override;
    
    void disconnect() override;
};