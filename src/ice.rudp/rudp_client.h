#pragma once

#ifndef ICE_RUDP_CLIENT
#define ICE_RUDP_CLIENT

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"

#include "common/transport/a_client.h"
#include "common/rudp_peer.h"
#include "common/rudp.h"
	
#include <functional>
#include <map>
#include <string>

class rudp_client final : public rudp_peer
{

public:

	a_client* socket = nullptr;

public:

	std::function<void()> connected_callback;

	std::function<void()> disconnected_callback;

public:

	std::function<void(ice_data::read&)> external_data_callback;

private:

	const int max_connection_attempts = 3;

	const int connection_timeout = 1500;

	int connection_attempts = 0;

private:

	scheduler::element* connect_element;

public:

	unsigned short get_ping();

public:

	end_point get_local_point();

	end_point get_remote_point();

public:

	void update();

public:

	void connect(end_point remote_point, end_point local_point);

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

public:

	void send(ice_data::write& data, bool reliable = false);

private:

	void send_connect_request();

public:

	void disconnect() override;
};

#endif