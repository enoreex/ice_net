#pragma once

#ifndef ICE_RUDP_CLIENT
#define ICE_RUDP_CLIENT

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"
#include "../ice.sock/udp_sock.h"

#include "common/rudp_peer.h"
#include "common/rudp.h"
	
#include <functional>
#include <map>
#include <string>

class rudp_client final : public rudp_peer
{

public:

	rudp_client();

public:

	udp_sock* socket = nullptr;

public:

	std::function<void()> connected_callback;

	std::function<void()> disconnected_callback;

public:

	std::function<void(ice_data::read&)> external_data_callback;

public:

	std::function<void(char*, unsigned short, unsigned short)> reliable_packet_lost;

private:

	const int max_connection_attempts = 6;

	const std::map<int, int> connection_timeout = { {1, 500}, {2, 500}, {3, 500}, {4, 500}, {5, 500}, {6, 500} };

	int connection_attempts = 0;

private:

	scheduler::element* connect_element = nullptr;

public:

	end_point get_local_point();

	end_point get_remote_point();

private:

	end_point remote_point = end_point(0, 0);

public:

	bool receive_with_update = true;

public:

	void update();

public:

	void connect(end_point remote_point);

	void connect_attempt();

public:

	void receive();

private:

	void handle(ice_data::read& data);

private:

	void handle_connect_response();

private:

	void ch_handle(ice_data::read& data) override;

	void ch_send(ice_data::write& data) override;

	void ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id) override;

private:

	void send_connect_confirm();

public:

	void send_unreliable(ice_data::write& data);

	void send_reliable(ice_data::write& data);

private:

	void send_connect_request();

public:

	void disconnect() override;

private:

	inline char _flag_heartbeat_request() override;

	inline char _flag_heartbeat_response() override;

	inline char _flag_unreliable() override;

	inline char _flag_reliable() override;

	inline char _flag_ack() override;
};

#endif