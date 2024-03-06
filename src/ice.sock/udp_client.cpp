#include "udp_client.h"

udp_client::~udp_client()
{
    disconnect();
}

end_point udp_client::get_local_point()
{
    return end_point(
        ntohl(local_in.sin_addr.s_addr),
        ntohs(local_in.sin_port));
}

end_point udp_client::get_remote_point()
{
    return end_point(
        ntohl(remote_in.sin_addr.s_addr),
        ntohs(remote_in.sin_port));
}

bool udp_client::connect(end_point& remote_point, end_point& local_point)
{

#ifdef _WIN32

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("WSAStartup error", win_error_msg);
        return false;
    }

#endif

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == 0 || sock == -1)
    {

#ifdef _WIN32
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("create socket error", win_error_msg);
#else
        ice_logger::log_error("create socket error", strerror(errno));
#endif      

        return false;
    }

    local_in.sin_family = AF_INET;
    local_in.sin_addr.s_addr = ntohl(local_point.get_address());
    local_in.sin_port = ntohs(local_point.get_port());

    remote_in.sin_family = AF_INET;
    remote_in.sin_addr.s_addr = ntohl(remote_point.get_address());
    remote_in.sin_port = ntohs(remote_point.get_port());

    if (bind(sock, (sockaddr*)&local_in, sizeof(local_in)) == -1)
    {

#ifdef _WIN32
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("bind error", win_error_msg);
#else
        ice_logger::log_error("bind error", strerror(errno));
#endif

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        return false;
    }

#ifdef _WIN32

    int len = sizeof(local_in);
    if (getsockname(sock, (sockaddr*)&local_in, &len) != 0)
    {
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("getsockname error", win_error_msg);
        closesocket(sock);
        return false;
    }

#else

    socklen_t len = sizeof(local_in);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&local_in), &len) != 0)
    {
        ice_logger::log_error("getsockname error", strerror(errno));
        close(sock);
        return false;
    }

#endif

    return true;
}

bool udp_client::receive_available()
{
    int available_data = 0;

#ifdef _WIN32

    u_long arg = 0;
    if (ioctlsocket(sock, FIONREAD, &arg) == SOCKET_ERROR)
    {
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("ioctlsocket error", win_error_msg);
        return false;
    }
    else
    {
        available_data = static_cast<int>(arg);
    }

#else

    if (ioctl(sock, FIONREAD, &available_data) == -1)
    {
        ice_logger::log_error("ioctl error", strerror(errno));
        return false;
    }

#endif

    return (available_data > 0);
}

a_client::recv_result udp_client::receive()
{
    recv_result result;

#ifdef _WIN32
    int remote_size = sizeof(sockaddr_in);
#else
    socklen_t remote_size = sizeof(sockaddr_in);
#endif

    int available_data = 0;

#ifdef _WIN32

    u_long arg = 0;
    if (ioctlsocket(sock, FIONREAD, &arg) == SOCKET_ERROR)
    {
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("ioctlsocket error", win_error_msg);
        return result;
    }
    else
    {
        available_data = static_cast<int>(arg);
    }

#else

    if (ioctl(sock, FIONREAD, &available_data) == -1)
    {
        ice_logger::log_error("ioctl error", strerror(errno));
        return result;
    }

#endif

    if (available_data == 0) return result;

    char* recv_arr = new char[available_data];
    int recv = recvfrom(sock, recv_arr, available_data, 0, (sockaddr*)&remote_in, &remote_size);

    if (recv == -1)
    {
        delete[] recv_arr;
        return result;
    }

    result.recv_arr = recv_arr;
    result.recv_size = static_cast<unsigned short>(recv);

    return result;
}

bool udp_client::send(char* data, unsigned short data_size)
{
    int result = sendto(sock, data, data_size, 0, (sockaddr*)&remote_in, sizeof(sockaddr_in));

    if (result == -1)
    {
        return false;
    }

    return true;
}

void udp_client::disconnect()
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