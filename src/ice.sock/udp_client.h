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
#include "..\ice.rudp\common\transport\a_client.h"

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#endif

class udp_client final : public a_client
{

private:

    SOCKET sock = 0;
    sockaddr_in local_in = sockaddr_in();
    sockaddr_in remote_in = sockaddr_in();

#ifdef _WIN32

private:

    char win_error_msg[256];

#endif

public:

    ~udp_client();

public:

    end_point get_local_point() override;

    end_point get_remote_point() override;
    
    bool connect(end_point& remote_point, end_point& local_point) override;
    
    bool receive_available() override;
    
    recv_result receive() override;
    
    bool send(char* data, unsigned short data_size) override;
    
    void disconnect() override;
};