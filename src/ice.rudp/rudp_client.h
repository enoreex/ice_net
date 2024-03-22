#pragma once

#ifndef ICE_RUDP_CLIENT
#define ICE_RUDP_CLIENT

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"

#include "common/transport/a_sock.h"
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

	a_sock* socket = nullptr;

public:

	std::function<void()> connected_callback;

	std::function<void()> disconnected_callback;

public:

	std::function<void(ice_data::read&)> external_data_callback;

public:

	std::function<void(char*, unsigned short, unsigned short)> reliable_packet_lost;

private:

	const int max_connection_attempts = 50;

	const std::map<int, int> connection_timeout = 
	{
		{1, 10}, {2, 10}, {3, 10}, {4, 10}, {5, 500},
		{6, 10}, {7, 10}, {8, 10}, {9, 10}, {10, 500},
		{11, 10}, {12, 10}, {13, 10}, {14, 10}, {15, 500}, 
		{16, 10}, {17, 10}, {18, 10}, {19, 10}, {20, 500}, 
		{21, 10}, {22, 10}, {23, 10}, {24, 10}, {25, 500}, 
		{26, 10}, {27, 10}, {28, 10}, {29, 10}, {30, 500}, 
		{31, 10}, {32, 10}, {33, 10}, {34, 10}, {35, 500}, 
		{36, 10}, {37, 10}, {38, 10}, {39, 10}, {40, 500}, 
		{41, 10}, {42, 10}, {43, 10}, {44, 10}, {45, 500},
		{46, 10}, {47, 10}, {48, 10}, {49, 10}, {50, 500},
	};

	int connection_attempts = 0;

private:

	scheduler::element* connect_element;

public:

	end_point get_local_point();

	end_point get_remote_point();

private:

	end_point remote_point = end_point(0, 0);

public:

	void update();

public:

	void connect(end_point remote_point, end_point local_point, bool sockInit = true);

	void connect_attempt();

private:

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

	void disconnect(bool notify) override;

private:

	inline char _flag_heartbeat_request();

	inline char _flag_heartbeat_response();

	inline char _flag_unreliable();

	inline char _flag_reliable();

	inline char _flag_ack();
};

#endif