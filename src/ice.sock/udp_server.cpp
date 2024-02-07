#include "udp_server.h"

udp_server::~udp_server()
{
    stop();
}

end_point udp_server::get_local_point()
{
    return end_point(
        ntohl(local_in.sin_addr.s_addr),
        ntohs(local_in.sin_port));
}

bool udp_server::start(end_point& local_point)
{

#ifdef _WIN32

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

#endif

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == 0 || sock == -1) return false;

    local_in.sin_family = AF_INET;
    local_in.sin_addr.s_addr = ntohl(local_point.get_address());
    local_in.sin_port = ntohs(local_point.get_port());

    if (bind(sock, (sockaddr*)&local_in, sizeof(local_in)) == -1) return false;

#ifdef _WIN32

    int len = sizeof(local_in);

    if (getsockname(sock, (sockaddr*)&local_in, &len) != 0) return false;

#else

    socklen_t len = sizeof(local_in);

    if (getsockname(sock, reinterpret_cast<sockaddr*>(&local_in), &len) != 0) return false;

#endif

    return true;
}

bool udp_server::receive_available()
{
    int available_data = 0;

#ifdef _WIN32

    u_long arg = 0;

    if (ioctlsocket(sock, FIONREAD, &arg) == SOCKET_ERROR) return false;

    else available_data = static_cast<int>(arg);

#else

    if (ioctl(sock, FIONREAD, &available_data) == -1) return false;

#endif

    return (available_data > 0);
}

a_server::recv_result udp_server::receive()
{
    recv_result result;

    sockaddr_in client_in;

#ifdef _WIN32
    int client_address_size = sizeof(client_in);
#else
    socklen_t client_address_size = sizeof(client_in);
#endif

    int available_data = 0;

#ifdef _WIN32

    u_long arg = 0;

    if (ioctlsocket(sock, FIONREAD, &arg) == SOCKET_ERROR) return result;

    else available_data = static_cast<int>(arg);

#else

    if (ioctl(sock, FIONREAD, &available_data) == -1) return result;

#endif

    if (available_data == 0) return result;

    char* recv_arr = new char[available_data];
    int recv = recvfrom(sock, recv_arr, available_data, 0, (sockaddr*)&client_in, &client_address_size);

    if (recv == -1)
    {
        delete[] recv_arr;
        return result;
    }

    result.recv_arr = recv_arr;
    result.recv_size = (unsigned short)available_data;

    result.recv_point.set_address(ntohl(client_in.sin_addr.s_addr));
    result.recv_point.set_port(ntohs(client_in.sin_port));

    return result;
}

bool udp_server::send(char* data, unsigned short data_size, const end_point& remote_point)
{
    sockaddr_in client_in;

#ifdef _WIN32
    int client_address_size = sizeof(client_in);
#else
    socklen_t client_address_size = sizeof(client_in);
#endif

    client_in.sin_family = AF_INET;
    client_in.sin_addr.s_addr = htonl(remote_point.get_address());
    client_in.sin_port = htons(remote_point.get_port());

    int result = sendto(sock, data, data_size, 0, (sockaddr*)&client_in, client_address_size);

    if (result == -1) return false;

    return true;
}

void udp_server::stop()
{
    if (sock == 0) return;

#ifdef _WIN32

    int status = shutdown(sock, SD_BOTH);
    if (status == 0) { closesocket(sock); }

#else

    int status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { close(sock); }

#endif

#ifdef _WIN32

    WSACleanup();

#endif
}