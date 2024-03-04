#pragma once

#ifndef ICE_RUDP_CNCT
#define ICE_RUDP_CNCT

#include "../ice.core/ice_data.h"
#include "../ice.core/ice_logger.h"

#include "common/transport/end_point.h"
#include "common/rudp_peer.h"
#include "common/rudp.h"

#include <functional>
#include <map>

class rudp_connection final : public rudp_peer
{

	friend class rudp_server;

private:

	using SERV_C_H = std::function<void(rudp_connection&, ice_data::read&)>;

	using SERV_C_S = std::function<void(end_point&, ice_data::write&)>;

	using SERV_C_D = std::function<void(end_point&)>;

	using SERV_P_L = std::function<void(rudp_connection&, char*, unsigned short, unsigned short)>;

private:

	rudp_connection(
		SERV_C_H sch,
		SERV_C_S scs,
		SERV_C_D scd,
		SERV_P_L rpl);

private:

	end_point remote_point = end_point(0, 0);

private:

	SERV_C_H serv_callback_handle;

	SERV_C_S serv_callback_send;

	SERV_C_D serv_callback_disconnect;

	SERV_P_L serv_reliable_packet_lost;

private:

	end_point get_remote_point();

	const end_point* get_remote_point_ptr();

private:

	void update();

private:

	void connect(end_point remote_point);

private:

	void handle(ice_data::read& data);

private:

	void handle_connect_request();

private:

	void ch_handle(ice_data::read& data) override;
	
	void ch_send(ice_data::write& data) override;

	void ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id) override;

private:

	void send_connect_response();

private:

	void disconnect() override;
};

#endif