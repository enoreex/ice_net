#include "rudp_connection.h"

rudp_connection::rudp_connection(
	SERV_C_H sch,
	SERV_C_S scs,
	SERV_C_D scd,
	SERV_P_L rpl)
{
	this->serv_callback_handle = sch;
	this->serv_callback_send = scs;
	this->serv_callback_disconnect = scd;
	this->serv_reliable_packet_lost = rpl;
}

end_point rudp_connection::get_remote_point()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	return remote_point;
}

const end_point* rudp_connection::get_remote_point_ptr()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	return &remote_point;
}

void rudp_connection::update()
{
	if (!scheduler.empty()) scheduler.execute();
}

void rudp_connection::connect(end_point remote_point)
{
	std::unique_lock<std::shared_timed_mutex> w_lock(mutex);

	this->current_state = connected;

	this->remote_point = remote_point;

	w_lock.unlock();

	rudp_peer::rudp_init();
}

void rudp_connection::handle(ice_data::read& data)
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state == disconnected) return;

	r_lock.unlock();

	char packet_id = data.get_flag();

	switch (packet_id)
	{

	case rudp::connect_request:
		handle_connect_request();
		break;

	case rudp::heartbeat_request:
		handle_heartbeat_request();
		break;

	case rudp::heartbeat_response:
		handle_heartbeat_response();
		break;

	case rudp::unreliable:
		handle_unreliable(data);
		break;

	case rudp::reliable:
		handle_reliable(data);
		break;

	case rudp::ack:
		handle_ack(data);
		break;

	default:
		break;

	}
}

void rudp_connection::handle_connect_request()
{
	send_connect_response();
}

void rudp_connection::ch_handle(ice_data::read& data)
{
	serv_callback_handle(*this, data);
}

void rudp_connection::ch_send(ice_data::write& data)
{
	serv_callback_send(remote_point, data);
}

void rudp_connection::ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id)
{
	serv_reliable_packet_lost(*this, data, size, id);
}

void rudp_connection::send_connect_response()
{
	ice_data::write data(1);
	data.set_flag((char)rudp::connect_response);
	ch_send(data);
}

void rudp_connection::disconnect()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state == disconnected) return;

	r_lock.unlock();

	rudp_peer::rudp_stop();

	serv_callback_disconnect(remote_point);
}