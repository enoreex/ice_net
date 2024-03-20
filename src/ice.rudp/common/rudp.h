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
		c_connect_request = 1,

		c_connect_response = 2,

		c_connect_confirm = 3

		c_heartbeat_request = 4,

		c_heartbeat_response = 5,

		c_unreliable = 6,

		c_reliable = 7,

		c_ack = 8,
	};

	enum headers_server
	{
		s_connect_request = 21,

		s_connect_response = 22,

		s_connect_confirm = 23,

		s_heartbeat_request = 24,

		s_heartbeat_response = 25,

		s_unreliable = 26,

		s_reliable = 27,

		s_ack = 28,
	};

	static const int disconnect_timeout = 5000;

	static const int heartbeat_interval = 750;

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