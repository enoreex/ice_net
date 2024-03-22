#pragma once

#ifndef ICE_RUDP
#define ICE_RUDP

#include <chrono>
#include <functional>

class rudp
{

public:

	enum headers_client
	{
		c_connect_request = 31,

		c_connect_response = 32,

		c_connect_confirm = 33,

		c_heartbeat_request = 34,

		c_heartbeat_response = 35,

		c_unreliable = 36,

		c_reliable = 37,

		c_ack = 38,
	};

	enum headers_server
	{
		s_connect_request = 61,

		s_connect_response = 62,

		s_connect_confirm = 63,

		s_heartbeat_request = 64,

		s_heartbeat_response = 65,

		s_unreliable = 66,

		s_reliable = 67,

		s_ack = 68,
	};

	static const int disconnect_timeout = 5000;

	static const int heartbeat_interval = 200;

	static const int min_resend_time = 200;

	static const int max_resend_time = 600;

	static const int max_resend_count = 8;
};

class scheduler final
{

public:

	struct element
	{
		element* next = nullptr;
		element* previous = nullptr;

		std::function<void()> event = nullptr;

		std::chrono::time_point<std::chrono::system_clock> event_time;
	};

private:

	element* head = nullptr;

public:

	bool empty();

public:

	element* add(std::function<void()> new_event, int time_in_milliseconds);

	void remove(element*& obj);

	void execute();

	void clear();
};

class stopwatch final
{

private:

	std::chrono::time_point<std::chrono::system_clock> start_time;

private:

	unsigned long long elapsed_milliseconds = 0;

public:

	void start();

	void stop();

public:

	unsigned long long get_elapsed_milliseconds();
};

#endif