#include "rudp_client.h"

rudp_client::rudp_client() {  }

end_point rudp_client::get_local_point()
{
	return socket->get_local_point();
}

end_point rudp_client::get_remote_point()
{
	return remote_point;
}

void rudp_client::update()
{
	if (!scheduler.empty()) scheduler.execute();

	receive();
}

void rudp_client::connect(end_point remote_point, end_point local_point, bool sockInit)
{
	if (socket == nullptr)
	{
		ice_logger::log_error("client-connect", "you cannot connect when the socket is null!");

		return;
	}

	if (current_state != disconnected) return;

	bool result = sockInit == false ? true : socket->start(local_point);

	if (result == false) return;

	current_state = connecting;
	
	connection_attempts = 0;

	this->remote_point = remote_point;

	ice_logger::log("client-connect", ("socket created! local ep: [" +
		socket->get_local_point().get_address_str() + ":" +
		socket->get_local_point().get_port_str() + "]"));

	connect_attempt();
}

void rudp_client::connect_attempt()
{
	if (current_state != connecting) return;

	if (connection_attempts >= max_connection_attempts)
	{
		disconnect();

		return;
	}

	send_connect_request();

	ice_logger::log("try-connect", ("trying to connect... attempt[" + std::to_string(connection_attempts) + "]!"));

	++connection_attempts;

	connect_element = scheduler.add([this]() { connect_attempt(); }, connection_timeout);
}

void rudp_client::receive()
{
	if (current_state == disconnected) return;

	if (socket->receive_available() == false) return;

	auto result = socket->receive();

	if (result.recv_arr == nullptr || result.recv_point != remote_point) return;

	ice_data::read data(result.recv_arr, result.recv_size);

	handle(data);
}

void rudp_client::handle(ice_data::read& data)
{
	char packet_id = data.get_flag();

	if (current_state == connecting && packet_id != rudp::connect_response) return;

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
	current_state = connected;

	scheduler.remove(connect_element);

	rudp_peer::rudp_init();

	send_connect_confirm();

	ice_logger::log("try-connect", "connected!");

	if (connected_callback) connected_callback();
}

void rudp_client::ch_handle(ice_data::read& data)
{
	if (external_data_callback) external_data_callback(data);
}

void rudp_client::ch_send(ice_data::write& data)
{
	socket->send(data.get_buffer(), data.get_buffer_size(), remote_point);
}

void rudp_client::ch_reliable_packet_lost(char* data, unsigned short size, unsigned short id)
{
	if (reliable_packet_lost) reliable_packet_lost(data, size, id);
}

void rudp_client::send_connect_confirm()
{
	ice_data::write data(1);
	data.set_flag(rudp::connect_confirm);
	ch_send(data);
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
	if (current_state == disconnected) return;

	rudp_peer::rudp_stop();

	ice_logger::log("client-disconnect", "disconnected!");

	if (disconnected_callback) disconnected_callback();
}