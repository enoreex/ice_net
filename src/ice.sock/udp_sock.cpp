#include "udp_sock.h"

udp_sock::~udp_sock()
{
    stop();
}

end_point udp_sock::get_local_point()
{
    return end_point(
        ntohl(local_in.sin_addr.s_addr),
        ntohs(local_in.sin_port));
}

bool udp_sock::start(end_point local_point)
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

bool udp_sock::receive_available()
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

a_sock::recv_result udp_sock::receive(recv_predicate predicate)
{
    sockaddr_in remote_in;

    recv_result result;

#ifdef _WIN32
    int remote_size = sizeof(sockaddr_in);
#else
    socklen_t remote_size = sizeof(sockaddr_in);
#endif

    int recv_header = recvfrom(sock, buffer, sizeof(buffer), MSG_PEEK, (sockaddr*)&remote_in, &remote_size);

    if (recv_header == -1)
    {
        recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&remote_in, &remote_size);

        return result;
    }

    end_point from = end_point(ntohl(remote_in.sin_addr.s_addr), ntohs(remote_in.sin_port));

    auto recv_predicate_result = predicate(buffer[0], from);

    switch (recv_predicate_result)
    {
        case a_sock::accept:
            break;

        case a_sock::reject:
            if (this->recv_mode == single) recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&remote_in, &remote_size);
            return result;    

        case a_sock::temp:
            recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&remote_in, &remote_size);
            return result;
    }

    int recv = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&remote_in, &remote_size);

    if (recv == -1) return result;

    result.recv_point = from;

    result.recv_arr = buffer;

    result.recv_size = static_cast<unsigned short>(recv);

    result.auto_release = false;

    return result;
}

bool udp_sock::send(const char* data, unsigned short data_size, end_point& remote_point)
{
    sockaddr_in client_in;
    memset(&client_in, 0, sizeof(client_in));

    client_in.sin_family = AF_INET;
    client_in.sin_addr.s_addr = inet_addr(remote_point.get_address_str().c_str());
    client_in.sin_port = htons(remote_point.get_port());

    int result = sendto(sock, data, data_size, 0, (const sockaddr*)&client_in, sizeof(client_in));

    if (result == -1)
    {

#ifdef _WIN32
        if (strerror_s(win_error_msg, sizeof(win_error_msg), WSAGetLastError()) == 0) ice_logger::log_error("sendto error", win_error_msg);
#else
        ice_logger::log_error("sendto error", strerror(errno));
#endif

        return false;
    }

    return true;
}

void udp_sock::stop()
{
    if (sock == 0) return;

#ifdef _WIN32

    int status = shutdown(sock, SD_BOTH);
    if (status == 0) { closesocket(sock); }

#else

    int status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { close(sock); }

#endif
}