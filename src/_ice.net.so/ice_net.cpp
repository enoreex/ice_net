#include "ice_net.h"



void logger_set_info(void(*action)(const char*))
{
	if (action == nullptr)
	{
		ice_logger::log_listener = nullptr;
		return;
	}

	ice_logger::log_listener = [action](const std::string& message) { action(message.c_str()); };
}

void logger_set_error(void(*action)(const char*))
{
	if (action == nullptr)
	{
		ice_logger::log_error_listener = nullptr;
		return;
	}

	ice_logger::log_error_listener = [action](const std::string& message) { action(message.c_str()); };
}



a_sock* create_transport(i_USHORT port)
{
	udp_sock* sock = new udp_sock();
	sock->start(end_point(0, port));
	return sock;
}



void transport_set_single(udp_sock* sock)
{
	sock->recv_mode = udp_sock::single;
}

void transport_set_shared(udp_sock* sock)
{
	sock->recv_mode = udp_sock::shared;
}



rudp_client* create_client()
{
	rudp_client* client = new rudp_client;
	return client;
}

rudp_server* create_server()
{
	rudp_server* server = new rudp_server;
	return server;
}



void client_set_transport(rudp_client* sock, a_sock* udp)
{
	sock->socket = udp;
}

void server_set_transport(rudp_server* sock, a_sock* udp)
{
	sock->socket = udp;
}



void start_client(rudp_client* sock, i_STRING address, i_USHORT port)
{
	if (sock == 0) return;

	sock->connect(end_point(address, port));
}

void start_server(rudp_server* sock)
{
	if (sock == 0) return;

	sock->try_start();
}



void update_client(rudp_client* sock)
{
	if (sock == 0) return;

	sock->update();
}

void update_server(rudp_server* sock)
{
	if (sock == 0) return;

	sock->update();
}



void client_set_handle(rudp_client* sock, void(*action)(rudp_client*, i_ARRAY, i_USHORT))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->external_data_callback = nullptr;
		return;
	}

	sock->external_data_callback = [sock, action](ice_data::read& data)
		{
			action(sock, data.get_buffer_remaining(), data.get_buffer_size_remaining());
		};
}

void server_set_handle(rudp_server* sock, void(*action)(rudp_server*, i_ARRAY, i_USHORT, rudp_connection*))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->external_data_callback = nullptr;
		return;
	}

	sock->external_data_callback = [sock, action](rudp_connection* c, ice_data::read& data)
		{
			action(sock, data.get_buffer_remaining(), data.get_buffer_size_remaining(), c);
		};
}



void server_set_predicate_add_connection(rudp_server* sock, bool(*action)(rudp_server*, end_point*, i_STRING, i_USHORT))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->external_data_callback = nullptr;
		return;
	}

	sock->predicate_add_connection = [sock, action](end_point& ep)
		{
			return action(sock, &ep, ep.get_address_str().c_str(), ep.get_port());
		};
}



void client_set_reliable_packet_lost(rudp_client* sock, void(*action)(rudp_client*, i_ARRAY, i_USHORT, i_USHORT))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->reliable_packet_lost = nullptr;
		return;
	}

	sock->reliable_packet_lost = [sock, action](i_ARRAY data, i_USHORT size, i_USHORT id)
		{
			action(sock, data, size, id);
		};
}

void server_set_reliable_packet_lost(rudp_server* sock, void(*action)(rudp_server*, i_ARRAY, i_USHORT, i_USHORT, rudp_connection*))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->reliable_packet_lost = nullptr;
		return;
	}

	sock->reliable_packet_lost = [sock, action](rudp_connection* c, i_ARRAY data, i_USHORT size, i_USHORT id)
		{
			action(sock, data, size, id, c);
		};
}



void client_set_connected(rudp_client* sock, void(*action)(rudp_client*))
{
	if (sock == nullptr) return;

	sock->connected_callback = [sock, action]() { action(sock); };
}

void client_set_disconnected(rudp_client* sock, void(*action)(rudp_client*))
{
	if (sock == nullptr) return;

	sock->disconnected_callback = [sock, action]() { action(sock); };
}



void server_set_connected(rudp_server* sock, void(*action)(rudp_server*, rudp_connection*, end_point*, i_STRING, i_USHORT))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->connection_added_callback = nullptr;
		return;
	}

	sock->connection_added_callback = [sock, action](rudp_connection* c)
		{
			rudp_connection* ptr = c;

			std::string address_str = sock->connection_internal_get_remote_ep(ptr).get_address_str();

			action(sock, c, sock->connection_internal_get_remote_ep_ptr(ptr), address_str.c_str(), sock->connection_internal_get_remote_ep(ptr).get_port());
		};
}

void server_set_disconnected(rudp_server* sock, void(*action)(rudp_server*, rudp_connection*))
{
	if (sock == nullptr) return;

	if (action == nullptr)
	{
		sock->connection_removed_callback = nullptr;
		return;
	}

	sock->connection_removed_callback = [sock, action](rudp_connection* c)
		{
			action(sock, c);
		};
}



void client_send_unreliable(rudp_client* sock, i_ARRAY data, i_USHORT size)
{
	if (sock == nullptr) return;

	ice_data::write send_d(size + 1);
	send_d.add_buffer(data, size, false);

	sock->rudp_client::send_unreliable(send_d);
}

void client_send_reliable(rudp_client* sock, i_ARRAY data, i_USHORT size)
{
	if (sock == nullptr) return;

	ice_data::write send_d(size + 3);
	send_d.add_buffer(data, size, false);

	sock->rudp_client::send_reliable(send_d);
}



void server_send_unreliable(rudp_server* sock, i_ARRAY data, i_USHORT size, end_point* ep)
{
	if (sock == nullptr || ep == nullptr) return;

	ice_data::write send_d(size + 1);
	send_d.add_buffer(data, size, false);

	sock->send_unreliable(*ep, send_d);
}

void server_send_reliable(rudp_server* sock, i_ARRAY data, i_USHORT size, end_point* ep)
{
	if (sock == nullptr || ep == nullptr) return;

	ice_data::write send_d(size + 3);
	send_d.add_buffer(data, size, false);

	sock->send_reliable(*ep, send_d);
}



void server_remove_connection(rudp_server* sock, rudp_connection* c)
{
	sock->connection_internal_disconnect(c);
}



rudp_client* release_client(rudp_client* sock)
{
	if (sock == nullptr) return nullptr;

	sock->disconnect();

	delete sock;

	ice_logger::log("release", "client socket deleted!");

	return nullptr;
}

rudp_server* release_server(rudp_server* sock)
{
	if (sock == nullptr) return nullptr;

	sock->stop();

	delete sock;

	ice_logger::log("release", "server socket deleted!");

	return nullptr;
}



a_sock* release_transport(a_sock* udp)
{
	delete udp;

	ice_logger::log("release", "udp socket deleted!");

	return nullptr;
}