#pragma once

#ifndef ICE_RUDP_PEER
#define ICE_RUDP_PEER

#include "../../ice.core/ice_data.h"
#include "../../ice.core/ice_logger.h"

#include "rudp.h"

#include <map>
#include <string>
#include <shared_mutex>

class rudp_peer
{

protected:

	virtual ~rudp_peer();
	
protected:

	enum client_state
	{
		disconnected,
		connecting,
		connected,
	};

	client_state current_state = disconnected;

private:

	unsigned short rtt = 0;

	unsigned short smooth_rtt = 0;

	unsigned short last_packet_id = 0;

private:

	void set_rtt(unsigned long long rtt); 

private:

	unsigned short get_resend_time();

	unsigned short get_next_packet_id();

protected:

	scheduler scheduler;

private:

	scheduler::element* heartbeat_element;

	scheduler::element* disconnect_element;

	stopwatch rtt_watch;

private:

	struct pending_packet
	{
		~pending_packet();

		unsigned char attempts = 0;

		unsigned short packet_id = 0;

		ice_data::write* data = nullptr;

		scheduler::element* element = nullptr;
	};

	std::map<unsigned short, pending_packet> pending_packets;
	
protected:

	void rudp_init();

	void rudp_stop();

private:

	void rudp_reset();

protected:

	void handle_heartbeat_request();

	void handle_heartbeat_response();

	void handle_unreliable(ice_data::read& data);

	void handle_reliable(ice_data::read& data);

	void handle_ack(ice_data::read& data);

protected:

	virtual void ch_handle(ice_data::read& data) = 0;

	virtual void ch_send(ice_data::write& data) = 0;

	virtual void ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id) = 0;

private:

	void send_heartbeat_request();

	void send_heartbeat_response();

protected:

	void send_unreliable(ice_data::write& data);

	void send_reliable(ice_data::write& data);

private:

	void send_reliable_attempt(unsigned short packet_id);

	void send_ack(unsigned short packet_id);

private:

	void reliable_release(unsigned short packet_id);

private:

	void start_heartbeat_timer();

	void start_disconnect_timer();

	void stop_disconnect_timer();

protected:

	virtual void disconnect(bool notify = true) = 0;

protected:

	virtual inline char _flag_heartbeat_request() = 0;

	virtual inline char _flag_heartbeat_response() = 0;

	virtual inline char _flag_unreliable() = 0;

	virtual inline char _flag_reliable() = 0;

	virtual inline char _flag_ack() = 0;
};

#endif