#include "rudp_server.h"

rudp_server::rudp_server() {  }

end_point rudp_server::get_local_point()
{
	return socket->get_local_point();
}

void rudp_server::update()
{
	if (current_state == disconnected) return;

	if (!planner.empty()) planner.execute();

	if (!connections_arr.empty())
	{
		for (int i = 0; i < connections_arr.size(); i++)
		{
			if (connections_arr[i]->current_state == disconnected) 
			{
				try_remove_connection(connections_arr[i]->remote_point);

				return;
			}
		}
	}

	if (!connections_arr.empty())
	{
		for (int i = 0; i < connections_arr.size(); i++)
		{
			connections_arr[i]->update();
		}
	}

	if (receive_with_update == true) receive();
}

bool rudp_server::try_start()
{
	if (socket == nullptr)
	{
		ice_logger::log_error("server-start", "you cannot start when the socket is null!");

		return false;
	}

	if (current_state == connected) return false;

	current_state = connected;

	ice_logger::log("server-start", ("socket created! local ep: [" +
		socket->get_local_point().get_address_str() + ":" +
		socket->get_local_point().get_port_str() + "]"));

	return true;
}

void rudp_server::receive()
{
	if (socket->receive_available() == false) return;

	auto result = socket->receive([](char flag, end_point& from) -> a_sock::recv_predicate_code
		{
			if (flag < rudp::headers_client::c_connect_request || flag > rudp::headers_server::s_ack ||
			   (flag > rudp::headers_client::c_ack && flag < rudp::headers_server::s_connect_request)) return a_sock::temp;

			if (flag >= rudp::headers_server::s_connect_request) return a_sock::accept;
			
			return a_sock::reject;

		});

	if (result.recv_arr == nullptr) return;

	char raw_packet_id = result.recv_arr[0];

	rudp_connection* connection = nullptr;

	try_get_connection(connection, result.recv_point);

	if (connection != nullptr)
	{
		ice_data::read data(result.recv_arr, result.recv_size);

		if (result.auto_release) delete[] result.recv_arr;

		connection->handle(data);

		return;
	}

	if (result.auto_release) delete[] result.recv_arr;

	if (!(raw_packet_id <= rudp::headers_server::s_connect_confirm && connection == nullptr)) return;

	switch (raw_packet_id)
	{

	case rudp::headers_server::s_connect_request:
		_connection_handle_request(result.recv_point);
		return;

	case rudp::headers_server::s_connect_confirm:
		_connection_handle_confirm(result.recv_point);
		return;

	default:
		return;

	}
}

void rudp_server::_connection_handle_request(end_point& remote_point)
{
	auto it = connections_pending.find(remote_point.get_hash());

	if (it != connections_pending.end()) return;

	bool predicate_result;

	try
	{
		if (predicate_add_connection == nullptr) predicate_result = true;
		else predicate_result = predicate_add_connection(remote_point);
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection predicate error", "user predicate error: "
			+ std::string(exc.what()));

		predicate_result = false;
	}

	if (predicate_result == false) return;

	connections_pending[remote_point.get_hash()] = planner.add([this, remote_point]() { _connection_expired(remote_point); }, connection_expire_timeout);

	ice_data::write data(1);
	data.set_flag(rudp::headers_client::c_connect_response);

	send(remote_point, data);
}

void rudp_server::_connection_handle_confirm(end_point& remote_point)
{
	auto it = connections_pending.find(remote_point.get_hash());

	if (it == connections_pending.end()) return;

	planner.remove(connections_pending.at(remote_point.get_hash()));

	connections_pending.erase(it);

	try_add_connection(remote_point);
}

void rudp_server::_connection_expired(end_point remote_point)
{
	auto it = connections_pending.find(remote_point.get_hash());

	if (it == connections_pending.end()) return;

	connections_pending.erase(remote_point.get_hash());
}

bool rudp_server::try_get_connection(rudp_connection*& connection, end_point& remote_point)
{
	auto it = connections.find(remote_point.get_hash());

	if (it == connections.end()) return false;

	connection = it->second;

	return connection != nullptr;
}

bool rudp_server::try_add_connection(end_point& remote_point)
{
	const auto cch = [this](rudp_connection*& c, ice_data::read& d) { connection_callback_handle(c, d); };
	const auto ccs = [this](end_point& e, ice_data::write& d) { connection_callback_send(e, d); };
	const auto rpl = [this](rudp_connection*& c, char* m, unsigned short s, unsigned short id) { connection_callback_reliable_packet_lost(c, m, s, id); };

	rudp_connection* connection = nullptr;

	try
	{
		connection = new rudp_connection(cch, ccs, rpl);
		connection->connect(remote_point);

		connections[remote_point.get_hash()] = connection;
		connections_arr.push_back(connection);

		ice_logger::log("connection added", ("connection added! remote ep: [" +
			remote_point.get_address_str() + ":" +
			remote_point.get_port_str() + "]"));

		try
		{
			ext_connection_added(connection);
		}

		catch (const std::exception& exc)
		{
			ice_logger::log_error("connection add error", "user connection_add notify error: "
				+ std::string(exc.what()));
		}


		return true;
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection add error", "add connection error: "
			+ std::string(exc.what()));

		connection->disconnect();

		delete connection;

		connection = nullptr;

		return false;
	}
}

bool rudp_server::try_remove_connection(end_point& remote_point)
{
	rudp_connection* connection = nullptr;

	try
	{
		if (try_get_connection(connection, remote_point) == false) return false;

		ice_logger::log("connection removed", ("connection removed! remote ep: [" +
			remote_point.get_address_str() + ":" +
			remote_point.get_port_str() + "]"));

		try
		{
			ext_connection_removed(connection);
		}

		catch (const std::exception& exc)
		{
			ice_logger::log_error("connection remove error", "user connection_remove notify error: "
				+ std::string(exc.what()));
		}

		auto it = std::find(connections_arr.begin(), connections_arr.end(), connection);

		if (it != connections_arr.end()) connections_arr.erase(it);

		connections.erase(remote_point.get_hash());
		
		connection->disconnect();

		delete connection;

		return true;
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection remove error", "add connection error: "
			+ std::string(exc.what()));

		connection->disconnect();

		delete connection;

		return false;
	}

}

void rudp_server::clear_connections()
{
	for (auto& c : connections_arr) try_remove_connection(c->remote_point);
}

void rudp_server::connection_internal_disconnect(rudp_connection*& connection)
{
	auto it = std::find(connections_arr.begin(), connections_arr.end(), connection);

	if (it == connections_arr.end()) return;

	auto remote_point = connection->get_remote_point();

	try_remove_connection(remote_point);
}

end_point rudp_server::connection_internal_get_remote_ep(rudp_connection*& connection)
{
	auto it = std::find(connections_arr.begin(), connections_arr.end(), connection);

	if (it == connections_arr.end()) return end_point(0, 0);

	auto remote_point = connection->get_remote_point();

	return remote_point;
}

end_point* rudp_server::connection_internal_get_remote_ep_ptr(rudp_connection*& connection)
{
	auto it = std::find(connections_arr.begin(), connections_arr.end(), connection);

	if (it == connections_arr.end()) return nullptr;

	auto remote_point = connection->get_remote_point_ptr();

	return remote_point;
}

void rudp_server::connection_callback_handle(rudp_connection*& connection, ice_data::read& data)
{
	ext_data_handled(connection, connection->get_remote_point(), data);
}

void rudp_server::connection_callback_send(end_point& remote_point, ice_data::write& data)
{
	send(remote_point, data);
}

void rudp_server::connection_callback_reliable_packet_lost(rudp_connection*& c, char* data, unsigned short size, unsigned short id) const
{
	if(reliable_packet_lost) reliable_packet_lost(c, data, size, id);
}

void rudp_server::send_unreliable(end_point& ep, ice_data::write& data)
{
	auto it = connections.find(ep.get_hash());

	if (it == connections.end()) return;

	it->second->send_unreliable(data);
}

void rudp_server::send_reliable(end_point& ep, ice_data::write& data)
{
	auto it = connections.find(ep.get_hash());

	if (it == connections.end()) return;

	it->second->send_reliable(data);
}

inline void rudp_server::ext_connection_added(rudp_connection*& c) const
{
	if (connection_added_callback) connection_added_callback(c);
}

inline void rudp_server::ext_connection_removed(rudp_connection*& c) const
{
	if (connection_removed_callback) connection_removed_callback(c);
}

inline void rudp_server::ext_data_handled(rudp_connection*& c, end_point p, ice_data::read& d) const
{
	if (external_data_callback) external_data_callback(c, d);
	if (external_data_specific_callback) external_data_specific_callback(c, p, d);
}

void rudp_server::send(end_point& remote_point, ice_data::write& data)
{
	if (current_state == disconnected) return;

	socket->send(data.get_buffer(), data.get_buffer_size(), remote_point);
}

void rudp_server::stop()
{
	current_state = disconnected;
	
	clear_connections();

	ice_logger::log("server-stop", "closed!");
}
