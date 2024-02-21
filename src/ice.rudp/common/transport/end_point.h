#pragma once

#ifndef ICE_END_P
#define ICE_END_P

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <cstdint>

struct end_point
{

public:

	end_point(
		std::string address_str,
		unsigned short port);

	end_point(
		unsigned long address,
		unsigned short port);

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

	bool operator ==(const end_point& other);

	bool operator <(const end_point& other) const;
};

#endif