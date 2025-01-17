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
	if (current_state == disconnected) return;

	if (!planner.empty()) planner.execute();

	if (receive_with_update == true) receive();
}

void rudp_client::connect(end_point remote_point)
{
	if (socket == nullptr)
	{
		ice_logger::log_error("client-connect", "you cannot connect when the socket is null!");

		return;
	}

	if (current_state != disconnected) return;

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

	ice_logger::log("try-connect", ("trying to connect... attempt[" + std::to_string(connection_attempts + 1) + "]!"));

	++connection_attempts;

	connect_element = planner.add([this]() { connect_attempt(); }, connection_timeout.at(connection_attempts));
}

void rudp_client::receive()
{
	if (current_state == disconnected) return;

	if (socket->receive_available() == false) return;

	auto result = socket->receive([this](char flag, end_point& from) -> a_sock::recv_predicate_code
		{
			if (flag < rudp::headers_client::c_connect_request || flag > rudp::headers_server::s_ack ||
			   (flag > rudp::headers_client::c_ack && flag < rudp::headers_server::s_connect_request)) return a_sock::temp;

			if (remote_point.get_address() != from.get_address() || remote_point.get_port() != from.get_port()) return a_sock::reject;

			if (flag <= rudp::headers_client::c_ack) return a_sock::accept;

			return a_sock::reject;
		});

	if (result.recv_arr == nullptr) return;

	ice_data::read data(result.recv_arr, result.recv_size);

	if (result.auto_release) delete[] result.recv_arr;

	handle(data);
}

void rudp_client::handle(ice_data::read& data)
{
	char packet_id = data.get_flag();

	if (current_state == connecting && packet_id != rudp::headers_client::c_connect_response) return;

	switch (packet_id)
	{

	case rudp::headers_client::c_connect_response:
		handle_connect_response();
		break;

	case rudp::headers_client::c_heartbeat_request:
		handle_heartbeat_request();
		break;

	case rudp::headers_client::c_heartbeat_response:
		handle_heartbeat_response();
		break;

	case rudp::headers_client::c_unreliable:
		handle_unreliable(data);
		break;

	case rudp::headers_client::c_reliable:
		handle_reliable(data);
		break;

	case rudp::headers_client::c_ack:
		handle_ack(data);
		break;

	default:
		break;

	}
}

void rudp_client::handle_connect_response()
{
	current_state = connected;

	planner.remove(connect_element);

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
	data.set_flag(rudp::headers_server::s_connect_confirm);
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
	data.set_flag(rudp::headers_server::s_connect_request);
	ch_send(data);
}

void rudp_client::disconnect()
{
	if (current_state == disconnected) return;

	rudp_peer::rudp_stop();

	ice_logger::log("client-disconnect", "disconnected!");

	if (disconnected_callback) disconnected_callback();
}

inline char rudp_client::_flag_heartbeat_request()
{
	return rudp::headers_server::s_heartbeat_request;
}

inline char rudp_client::_flag_heartbeat_response()
{
	return rudp::headers_server::s_heartbeat_response;
}

inline char rudp_client::_flag_unreliable()
{
	return rudp::headers_server::s_unreliable;
}

inline char rudp_client::_flag_reliable()
{
	return rudp::headers_server::s_reliable;
}

inline char rudp_client::_flag_ack()
{
	return rudp::headers_server::s_ack;
}
