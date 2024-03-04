#include "rudp_client.h"

end_point rudp_client::get_local_point()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	return socket->get_local_point();
}

end_point rudp_client::get_remote_point()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	return socket->get_remote_point();
}

void rudp_client::update()
{
	if (!scheduler.empty()) scheduler.execute();

	receive();
}

void rudp_client::connect(end_point remote_point, end_point local_point)
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (socket == nullptr)
	{
		ice_logger::log_error("client-connect", "you cannot connect when the socket is null!");

		return;
	}

	if (current_state != disconnected) return;

	r_lock.unlock();

	std::unique_lock<std::shared_timed_mutex> w_lock(mutex);

	bool result = socket->connect(remote_point, local_point);

	if (result == false) return;

	current_state = connecting;

	connection_attempts = 0;

	w_lock.unlock();

	ice_logger::log("client-connect", ("socket created! local ep: [" +
		socket->get_local_point().get_address_str() + ":" +
		socket->get_local_point().get_port_str() + "]"));

	connect_attempt();
}

void rudp_client::connect_attempt()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state != connecting) return;

	r_lock.unlock();

	if (connection_attempts >= max_connection_attempts)
	{
		disconnect();

		return;
	}

	send_connect_request();

	ice_logger::log("try-connect", ("trying to connect... attempt[" + std::to_string(connection_attempts) + "]!"));

	std::unique_lock<std::shared_timed_mutex> w_lock(mutex);

	++connection_attempts;

	connect_element = scheduler.add([this]() { connect_attempt(); }, connection_timeout);
}

void rudp_client::receive()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state == disconnected) return;

	if (socket->receive_available() == false) return;

	auto result = socket->receive();

	r_lock.unlock();

	if (result.recv_arr == nullptr) return;

	ice_data::read data(result.recv_arr, result.recv_size);

	handle(data);
}

void rudp_client::handle(ice_data::read& data)
{
	char packet_id = data.get_flag();

	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state == connecting && packet_id != rudp::connect_response) return;

	r_lock.unlock();

	switch (packet_id)
	{

	case rudp::connect_response:
		handle_connect_response();
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

void rudp_client::handle_connect_response()
{
	std::unique_lock<std::shared_timed_mutex> w_lock(mutex);

	current_state = connected;

	scheduler.remove(connect_element);

	w_lock.unlock();

	rudp_peer::rudp_init();

	ice_logger::log("try-connect", "connected!");

	if (connected_callback) connected_callback();
}

void rudp_client::ch_handle(ice_data::read& data)
{
	if (external_data_callback) external_data_callback(data);
}

void rudp_client::ch_send(ice_data::write& data)
{
	socket->send(data.get_buffer(), data.get_buffer_size());
}

void rudp_client::ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id)
{
	if (reliable_packet_lost) reliable_packet_lost(data, size, id);
}

void rudp_client::send_unreliable(ice_data::write& data)
{
	rudp_peer::send_unreliable(data);
}

void rudp_client::send_reliable(ice_data::write& data)
{
	rudp_peer::send_reliable(data);
}

void rudp_client::send_connect_request()
{
	ice_data::write data(1);
	data.set_flag((char)rudp::connect_request);
	ch_send(data);
}

void rudp_client::disconnect()
{
	std::shared_lock<std::shared_timed_mutex> r_lock(mutex);

	if (current_state == disconnected) return;

	r_lock.unlock();

	std::unique_lock<std::shared_timed_mutex> w_lock(mutex);

	w_lock.unlock();

	rudp_peer::rudp_stop();

	ice_logger::log("client-disconnect", "disconnected!");

	if (disconnected_callback) disconnected_callback();
}