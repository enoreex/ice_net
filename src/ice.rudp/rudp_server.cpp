#include "rudp_server.h"

end_point rudp_server::get_local_point()
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	return socket->get_local_point();
}

void rudp_server::update()
{
	if (current_state == disconnected) return;

	if (connections_arr.size() != 0) for (auto& c : connections_arr) c->update();

	receive();
}

bool rudp_server::try_start(end_point local_point)
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	if (socket == nullptr)
	{
		ice_logger::log_error("server-start", "you cannot start when the socket is null!");

		return false;
	}

	if (current_state == connected) return false;

	r_lock.unlock();

	std::unique_lock<std::shared_mutex> w_lock(mutex);

	auto result = socket->start(local_point);

	if (result == true) current_state = connected;

	ice_logger::log("server-start", ("socket created! local ep: [" +
		socket->get_local_point().get_address_str() + ":" +
		socket->get_local_point().get_port_str() + "]"));

	return result;
}

void rudp_server::receive()
{
	std::shared_lock<std::shared_mutex> r_sock_lock(mutex);

	if (socket->receive_available() == false) return;

	auto result = socket->receive();

	r_sock_lock.unlock();

	if (result.recv_arr == nullptr) return;

	char raw_packet_id = result.recv_arr[0];

	rudp_connection* connection = nullptr;

	std::shared_lock<std::shared_mutex> r_get_lock(mutex);

	try_get_connection(connection, result.recv_point);

	if (raw_packet_id == rudp::connect_request && connection == nullptr)
	{
		r_get_lock.unlock();

		std::unique_lock<std::shared_mutex> w_add_lock(mutex);

		if (try_add_connection(result.recv_point) == true)
		{
			try_get_connection(connection, result.recv_point);

			if (connection == nullptr) return;
			
			ice_data::read data(result.recv_arr, result.recv_size);

			connection->handle(data);

			w_add_lock.unlock();

			std::shared_lock<std::shared_mutex> r_ext_lock(mutex);

			try_get_connection(connection, result.recv_point);
			
			if (connection) ext_connection_added(*connection);

			r_ext_lock.unlock();
		}

		return;
	}

	if (connection == nullptr) return;

	ice_data::read data(result.recv_arr, result.recv_size);

	connection->handle(data);
}

bool rudp_server::try_get_connection(rudp_connection*& connection, end_point& remote_point)
{
	auto it = connections.find(remote_point);

	if (it == connections.end()) return false;

	connection = it->second;

	return connection != nullptr;
}

bool rudp_server::try_add_connection(end_point& remote_point)
{
	bool predicate_result;

	try
	{
		if (predicate_add_connection == nullptr) predicate_result = true;
		else predicate_result = predicate_add_connection(remote_point);
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection added error", "user predicate error: "
			+ std::string(exc.what()));

		predicate_result = false;
	}

	if (predicate_result == false) return false;

	const auto cch = [this](rudp_connection& c, ice_data::read& d) { connection_callback_handle(c, d); };
	const auto ccs = [this](end_point& e, ice_data::write& d) { connection_callback_send(e, d); };
	const auto ccd = [this](end_point& c) { connection_callback_disconnect(c); };

	rudp_connection* connection = nullptr;

	try
	{
		connection = new rudp_connection(cch, ccs, ccd);
		connection->connect(remote_point);

		connections.insert({ remote_point, connection });
		connections_arr.push_back(connection);

		ice_logger::log("connection added", ("connection added! remote ep: [" +
			remote_point.get_address_str() + ":" +
			remote_point.get_port_str() + "]"));

		return true;
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection add error", "add connection error: "
			+ std::string(exc.what()));

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
		std::unique_lock<std::shared_mutex> w_lock(mutex);

		if (try_get_connection(connection, remote_point) == false) return false;

		connections.erase(remote_point);

		auto it = std::remove(connections_arr.begin(), connections_arr.end(), connection);
		connections_arr.erase(it, connections_arr.end());

		w_lock.unlock();

		ice_logger::log("connection removed", ("connection removed! remote ep: [" +
			remote_point.get_address_str() + ":" +
			remote_point.get_port_str() + "]"));

		ext_connection_removed(*connection);

		delete connection;

		return true;
	}

	catch (const std::exception& exc)
	{
		ice_logger::log_error("connection remove error", "add connection error: "
			+ std::string(exc.what()));

		delete connection;

		return false;
	}

}

bool rudp_server::try_remove_connection(rudp_connection*& connection)
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	auto it = std::find(connections_arr.begin(), connections_arr.end(), connection);

	if (it == connections_arr.end()) return false;

	auto remote_point = connection->get_remote_point();

	r_lock.unlock();

	return try_remove_connection(remote_point);
}

void rudp_server::connection_callback_handle(rudp_connection& connection, ice_data::read& data)
{
	external_data_callback(connection, data);
}

void rudp_server::connection_callback_send(end_point& remote_point, ice_data::write& data)
{
	send(remote_point, data);
}

void rudp_server::connection_callback_disconnect(end_point& remote_point)
{
	try_remove_connection(remote_point);
}

void rudp_server::send_unreliable(end_point& ep, ice_data::write& data)
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	auto it = connections.find(ep);

	if (it == connections.end()) return;

	it->second->send_unreliable(data);
}

void rudp_server::send_reliable(end_point& ep, ice_data::write& data)
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	auto it = connections.find(ep);

	if (it == connections.end()) return;

	it->second->send_reliable(data);
}

inline void rudp_server::ext_connection_added(rudp_connection& c)
{
	if (connection_added_callback) connection_added_callback(c);
}

inline void rudp_server::ext_connection_removed(rudp_connection& c)
{
	if (connection_removed_callback) connection_removed_callback(c);
}

inline void rudp_server::ext_data_handled(rudp_connection& c, ice_data::read& d)
{
	if (external_data_callback) external_data_callback(c, d);
}

void rudp_server::send(end_point& remote_point, ice_data::write& data)
{
	std::shared_lock<std::shared_mutex> r_lock(mutex);

	if (current_state == disconnected) return;

	socket->send(data.get_buffer(), data.get_buffer_size(), remote_point);
}

void rudp_server::stop()
{
	std::unique_lock<std::shared_mutex> w_lock(mutex);

	if (current_state == disconnected || !socket) return;

	current_state = disconnected;

	connections.clear();
	connections_arr.clear();

	ice_logger::log("server-stop", "closed!");
}
