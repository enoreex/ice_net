#pragma once

#ifndef ICE_RUDP_SERV
#define ICE_RUDP_SERV

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"

#include "common/transport/a_server.h"
#include "common/rudp.h"

#include "rudp_connection.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <shared_mutex>

class rudp_server final
{

private:

	enum server_state
	{
		disconnected,
		connected,
	};

	server_state current_state = disconnected;

public:

	a_server* socket = nullptr;

private:

	std::unordered_map<long long, scheduler::element*> connections_pending;

private:

	std::unordered_map<long long, rudp_connection*> connections;

	std::vector<rudp_connection*> connections_arr;

public:

	std::function<bool(end_point&)> predicate_add_connection;

public:

	std::function<void(rudp_connection&)> connection_added_callback;

	std::function<void(rudp_connection&)> connection_removed_callback;

public:

	std::function<void(rudp_connection&, ice_data::read&)> external_data_callback;

public:

	std::function<void(rudp_connection&, char*, unsigned short, unsigned short)> reliable_packet_lost;

private:

	const int connection_expire_timeout = 1000;

private:

	scheduler scheduler;

public:

	end_point get_local_point();

public:

	void update();

public:

	bool try_start(end_point local_point);

private:

	void receive();

private:

	void _connection_handle_request(end_point& remote_point);

	void _connection_handle_confirm(end_point& remote_point);

	void _connection_expired(end_point remote_point);

private:

	bool try_get_connection(rudp_connection*& connection, end_point& remote_point);

	bool try_add_connection(end_point& remote_point);

	bool try_remove_connection(end_point& remote_point);

public:

	void connection_internal_disconnect(rudp_connection*& connection);

	end_point connection_internal_get_remote_ep(rudp_connection*& connection);

	const end_point* connection_internal_get_remote_ep_ptr(rudp_connection*& connection);

private:

	void connection_callback_handle(rudp_connection& connection, ice_data::read& data);

	void connection_callback_send(end_point& remote_point, ice_data::write& data);

	void connection_callback_disconnect(end_point& remote_point);

	void connection_callback_reliable_packet_lost(rudp_connection& c, char* data, unsigned short size, unsigned short id) const;

public:

	void send_unreliable(end_point& ep, ice_data::write& data);

	void send_reliable(end_point& ep, ice_data::write& data);

private:

	inline void ext_connection_added(rudp_connection& c) const;

	inline void ext_connection_removed(rudp_connection& c) const;

	inline void ext_data_handled(rudp_connection& c, ice_data::read& d) const;

private:

	void send(end_point& remote_point, ice_data::write& data);

public:

	void stop();
};

#endif