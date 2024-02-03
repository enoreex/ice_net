#include "win_udp_server.h"

win_udp_server::~win_udp_server()
{
    stop();
}

end_point win_udp_server::get_local_point()
{
    return end_point(
        ntohl(local_in.sin_addr.s_addr),
        ntohs(local_in.sin_port));
}

bool win_udp_server::start(end_point& local_point)
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return false;

    local_in.sin_family = AF_INET;
    local_in.sin_addr.s_addr = ntohl(local_point.get_address());
    local_in.sin_port = ntohs(local_point.get_port());

    if (bind(sock, (sockaddr*)&local_in, sizeof(local_in)) == SOCKET_ERROR) return false;

    int addrLen = sizeof(local_in);
    if (getsockname(sock, (sockaddr*)&local_in, &addrLen) != 0) return false;

    return true;
}

bool win_udp_server::receive_available()
{
    u_long available_data = 0;
    if (ioctlsocket(sock, FIONREAD, &available_data) == SOCKET_ERROR) return false;
    return (available_data > 0);
}

a_server::recv_result win_udp_server::receive()
{
    recv_result result;

    sockaddr_in client_in;
    int client_address_size = sizeof(sockaddr_in);

    u_long available_data = 0;
    if (ioctlsocket(sock, FIONREAD, &available_data) == SOCKET_ERROR) return result;

    if (available_data == 0) return result;

    char* recv_arr = new char[available_data];
    int recv = recvfrom(sock, recv_arr, available_data, 0, (sockaddr*)&client_in, &client_address_size);

    if (recv == SOCKET_ERROR)
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

bool win_udp_server::send(char* data, unsigned short data_size, const end_point& remote_point)
{
    sockaddr_in client_in;

    client_in.sin_family = AF_INET;
    client_in.sin_addr.s_addr = htonl(remote_point.get_address());
    client_in.sin_port = htons(remote_point.get_port());

    int result = sendto(sock, data, data_size, 0, (sockaddr*)&client_in, sizeof(client_in));

    if (result == SOCKET_ERROR) return false;

    return true;
}

void win_udp_server::stop()
{
    if (sock == INVALID_SOCKET) return;

    closesocket(sock);

    sock = INVALID_SOCKET;

    local_in.sin_addr.s_addr = 0;
    local_in.sin_port = 0;

    WSACleanup();
}