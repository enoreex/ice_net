#pragma once

#ifndef ICE_END_P
#define ICE_END_P

#include <string>

struct end_point
{

public:

	end_point(
		std::string address_str,
		unsigned short port);

	end_point(
		unsigned long address,
		unsigned short port);

	end_point(const end_point& other);

private:

	unsigned long address;

	unsigned short port;

public:

	void set_address(unsigned long address);

	void set_port(unsigned short port);

public:

	bool set_address(std::string address_str);

	bool set_port(std::string port_str);

public:

	unsigned long get_address() const;

	unsigned short get_port() const;

public:

	std::string get_address_str() const;

	std::string get_port_str() const;

public:

	void reset();

public:

	bool operator ==(const end_point& other);

public:

	end_point& operator=(const end_point& other);

	std::size_t operator()(const end_point& ep) const;
};

#endif