#pragma once

#ifndef ICE
#define ICE

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"

#include "../ice.rudp/common/transport/a_sock.h"
#include "../ice.rudp/common/transport/end_point.h"

#include "../ice.rudp/common/rudp.h"
#include "../ice.rudp/common/rudp_peer.h"

#include "../ice.rudp/rudp_client.h"
#include "../ice.rudp/rudp_connection.h"
#include "../ice.rudp/rudp_server.h"

#include "../ice.sock/udp_sock.h"



#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef ICE_NET_EXPORTS
        #define ICE_NET_API extern "C" __declspec(dllexport)
    #else
        #define ICE_NET_API extern "C" __declspec(dllimport)
    #endif
#else
    #define ICE_NET_API
#endif



#define CLIENT_T 1

#define SERVER_T 2

#define i_ARRAY char*

#define i_STRING const char*

#define i_USHORT unsigned short


ICE_NET_API void logger_set_info(void(*action)(const char*));

ICE_NET_API void logger_set_error(void(*action)(const char*));



ICE_NET_API a_sock* create_transport(i_USHORT port);



ICE_NET_API void transport_set_shared(udp_sock* sock);

ICE_NET_API void transport_set_unique(udp_sock* sock);



ICE_NET_API rudp_client* create_client();

ICE_NET_API rudp_server* create_server();



ICE_NET_API void client_set_transport(rudp_client* sock, a_sock* udp);

ICE_NET_API void server_set_transport(rudp_server* sock, a_sock* udp);



ICE_NET_API void start_client(rudp_client* sock, i_STRING address, i_USHORT port);

ICE_NET_API void start_server(rudp_server* sock);



ICE_NET_API void update_client(rudp_client* sock);

ICE_NET_API void update_server(rudp_server* sock);



ICE_NET_API void client_set_handle(rudp_client* sock, void(*action)(rudp_client*, i_ARRAY, i_USHORT));

ICE_NET_API void server_set_handle(rudp_server* sock, void(*action)(rudp_server*, i_ARRAY, i_USHORT, rudp_connection*));



ICE_NET_API void server_set_predicate_add_connection(rudp_server* sock, bool(*action)(rudp_server*, end_point*, i_STRING, i_USHORT));



ICE_NET_API void client_set_reliable_packet_lost(rudp_client* sock, void(*action)(rudp_client*, i_ARRAY, i_USHORT, i_USHORT));

ICE_NET_API void server_set_reliable_packet_lost(rudp_server* sock, void(*action)(rudp_server*, i_ARRAY, i_USHORT, i_USHORT, rudp_connection*));



ICE_NET_API void client_set_connected(rudp_client* sock, void(*action)(rudp_client*));

ICE_NET_API void client_set_disconnected(rudp_client* sock, void(*action)(rudp_client*));



ICE_NET_API void server_set_connected(rudp_server* sock, void(*action)(rudp_server*, rudp_connection*, end_point*, i_STRING, i_USHORT));

ICE_NET_API void server_set_disconnected(rudp_server* sock, void(*action)(rudp_server*, rudp_connection*));



ICE_NET_API void client_send_unreliable(rudp_client* sock, i_ARRAY data, i_USHORT size);

ICE_NET_API void client_send_reliable(rudp_client* sock, i_ARRAY data, i_USHORT size);



ICE_NET_API void server_send_unreliable(rudp_server* sock, i_ARRAY data, i_USHORT size, end_point* ep);

ICE_NET_API void server_send_reliable(rudp_server* sock, i_ARRAY data, i_USHORT size, end_point* ep);



ICE_NET_API void server_remove_connection(rudp_server* sock, rudp_connection* c);



ICE_NET_API rudp_client* release_client(rudp_client* sock);

ICE_NET_API rudp_server* release_server(rudp_server* sock);



ICE_NET_API a_sock* release_transport(a_sock* udp);

#endif