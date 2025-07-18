#include "rudp_peer.h"

rudp_peer::pending_packet::~pending_packet()
{
	delete data;
}

#pragma region calculations

void rudp_peer::set_rtt(unsigned long long rtt)
{
	this->rtt = static_cast<unsigned short>(rtt > 0xFFFF ? 0xFFFF : rtt);
}

unsigned short rudp_peer::get_resend_time()
{
	smooth_rtt = static_cast<unsigned short>((0.2 * rtt) + (0.8 * smooth_rtt));

	return
		smooth_rtt * 2 < rudp::max_resend_time ?
		(rudp::min_resend_time > smooth_rtt * 2 ? rudp::min_resend_time : smooth_rtt * 2) :
		rudp::max_resend_time;
}

unsigned short rudp_peer::get_next_packet_id()
{
	return last_packet_id =
		(last_packet_id == 65535) ? 1 :
		(++last_packet_id);
}

#pragma endregion

void rudp_peer::rudp_init()
{
	rudp_reset();

	start_heartbeat_timer();
}

void rudp_peer::rudp_stop()
{
	if (current_state == disconnected) return;

	current_state = disconnected;

	rudp_reset();
}

void rudp_peer::rudp_reset()
{	
	planner.clear();

	if (!pending_packets.empty()) for (auto& it : pending_packets) reliable_release(it.second.packet_id);
		
	pending_packets.clear();	
}

void rudp_peer::handle_heartbeat_request()
{
	send_heartbeat_response();
}

void rudp_peer::handle_heartbeat_response()
{
	rtt_watch.stop();

	set_rtt(rtt_watch.get_elapsed_milliseconds());

	stop_disconnect_timer();
}

void rudp_peer::handle_unreliable(ice_data::read& data)
{
	ch_handle(data);
}

void rudp_peer::handle_reliable(ice_data::read& data)
{
	unsigned short packet_id = data.get_int16();

	if (packet_id < 1) return;

	send_ack(packet_id);
	ch_handle(data);
}

void rudp_peer::handle_ack(ice_data::read& data)
{
	unsigned short packet_id = data.get_int16();

	if (packet_id < 1) return;

	auto pair = pending_packets.find(packet_id);

	if (pair == pending_packets.end()) return;

	auto& packet = pair->second;

	planner.remove(packet.element);

	pending_packets.erase(pair);
}

void rudp_peer::send_heartbeat_request()
{
	ice_data::write data(1);
	data.set_flag(_flag_heartbeat_request());

	ch_send(data);

	rtt_watch.start();
}

void rudp_peer::send_heartbeat_response()
{
	ice_data::write data(1);
	data.set_flag(_flag_heartbeat_response());
	ch_send(data);
}

void rudp_peer::send_unreliable(ice_data::write& data)
{
	if (current_state != connected) return;

	data.set_flag(_flag_unreliable());
	ch_send(data);
}

void rudp_peer::send_reliable(ice_data::write& data)
{
	if (current_state != connected) return;

	char* reliable_arr = new char[data.get_buffer_size() - 1];
	memcpy(reliable_arr, data.get_buffer() + 1, data.get_buffer_size() - 1);

	unsigned short packet_id = get_next_packet_id();

	ice_data::write* reliable_data = new ice_data::write(data.get_buffer_size() - 1 + 3);

	reliable_data->set_flag(_flag_reliable());

	reliable_data->add_int16(packet_id);

	reliable_data->add_buffer(reliable_arr, data.get_buffer_size() - 1);

	pending_packets.insert(std::make_pair(packet_id, pending_packet{}));

	auto pair = pending_packets.find(packet_id);

	if (pair == pending_packets.end()) return;

	auto& packet = pair->second;

	packet.data = reliable_data;

	packet.packet_id = packet_id;

	send_reliable_attempt(packet_id);
}

void rudp_peer::send_reliable_attempt(unsigned short packet_id)
{
	if (current_state != connected) return;

	auto pair = pending_packets.find(packet_id);

	if (pair == pending_packets.end()) return;

	auto& packet = pair->second;

	if (packet.attempts >= rudp::max_resend_count)
	{
		reliable_release(packet_id);

		pending_packets.erase(pair);

		return;
	}

	++packet.attempts;

	packet.element = planner.add([this, packet_id]() { send_reliable_attempt(packet_id); }, get_resend_time());

	ch_send(*packet.data);
}

void rudp_peer::send_ack(unsigned short packet_id)
{
	ice_data::write data(3);
	data.set_flag(_flag_ack());
	data.add_int16(packet_id);

	ch_send(data);
}

void rudp_peer::reliable_release(unsigned short packet_id)
{
	auto pair = pending_packets.find(packet_id);

	if (pair == pending_packets.end()) return;

	auto& packet = pair->second;

	ice_logger::log("reability", ("reliable packet[" + std::to_string(packet.packet_id) + "] was not handled!"));

	ch_reliable_packet_lost(packet.data->get_buffer() + 3, packet.data->get_buffer_size() - 3, packet.packet_id);
}

void rudp_peer::start_heartbeat_timer()
{
	send_heartbeat_request();

	start_disconnect_timer();

	heartbeat_element = planner.add([this]() { start_heartbeat_timer(); }, rudp::heartbeat_interval);
}

void rudp_peer::start_disconnect_timer()
{
	if (disconnect_element != nullptr) return;

	disconnect_element = planner.add([this]() { disconnect(); }, rudp::disconnect_timeout);
}

void rudp_peer::stop_disconnect_timer()
{
	planner.remove(disconnect_element);
}
